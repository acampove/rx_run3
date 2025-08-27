'''
Module with SampleWeighter class
'''
import os
import re
import glob
import math
import pickle
import numexpr
from pathlib import Path

import numpy
import matplotlib.pyplot as plt
import pandas            as pnd
from numpy import typing as numpy_typing
from omegaconf              import DictConfig
from boost_histogram        import Histogram    as bh
from boost_histogram        import accumulators as acc
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_misid:sample_weighter')

BREM       ='brem'
NOBREM     ='nobrem'
FloatArray = numpy_typing.NDArray[numpy.float64]
# ------------------------------
class SampleWeighter:
    '''
    Class intended to:

    - Read calibration maps
    - Pick datasets
    - Apply weights to datasets and return them
    '''
    # ------------------------------
    def __init__(
        self,
        df     : pnd.DataFrame,
        is_sig : bool,
        sample : str,
        cfg    : DictConfig):
        '''
        df    : Pandas dataframe with columns including 'hadron' and 'kind'. Used to assign weights
        is_sig: If true, the weights will provide signal region sample, otherwise control region
        sample: E.g. DATA_24_... Needed to pick maps based on actual particle identity
        cfg   : omegaconf dictionary storing configuration
        '''
        self._cfg    = cfg
        self._is_sig = is_sig
        self._sample = sample
        self._varx   : str
        self._vary   : str

        self._l_electron_sample = ['Bu_JpsiK_ee_eq_DPC', 'Bu_Kee_eq_btosllball05_DPC']
        self._l_hadron_sample   = ['Bu_piplpimnKpl_eq_sqDalitz_DPC', 'Bu_KplKplKmn_eq_sqDalitz_DPC']
        self._l_kind            = [BREM, NOBREM]
        self._regex             = r'.*_(block\d)(?:_v\d)?-(?:up|down)-(K|Pi)-.*'

        #PT:
        #    True : 10
        #    False: 20
        self._d_out_of_map : dict[str,dict[bool,int]] = {}
        self._d_quality    : dict[str,int] = {
            'NaN'      : 0, 
            'Inf'      : 0, 
            'Negative' : 0, 
            'Zeroes'   : 0, 
            'Good'     : 0,
            'Ones'     : 0,
            'Above 1'  : 0} 

        self._set_variables()
        self._df                              = self._get_df(df)
        self._d_map : dict[str, dict[str,bh]] = { kind : self._get_maps(kind=kind) for kind in self._l_kind }
        self._true_electron                   = self._is_true_electron()
    # ------------------------------
    def _is_true_electron(self) -> bool:
        '''
        Returns
        -------------
        True : If sample has Lepton candidates meant to be treated as electrons, e.g signal sample
        False: E.g. misID MC
        '''
        # Data is always evaluated in control region
        # Assume electrons are hadrons here
        if self._sample.startswith('DATA_24_'):
            return False

        if self._sample in self._l_electron_sample:
            log.info(f'Reading true electron efficiencies for: {self._sample}')
            return True

        if self._sample in self._l_hadron_sample:
            log.info(f'Reading fake electron efficiencies for: {self._sample}')
            return False

        raise NotImplementedError(f'Cannot obtain efficiency for {self._sample} sample')
    # ------------------------------
    def _get_df(self, df : pnd.DataFrame) -> pnd.DataFrame:
        '''
        Parameters
        ---------------
        df : Pandas dataframe with input

        Returns
        ---------------
        Input dataframe with columns added.
        '''

        df = self._add_columns(df=df, particle='L1')
        df = self._add_columns(df=df, particle='L2')

        return df
    # ------------------------------
    def _add_columns(self, df : pnd.DataFrame, particle : str) -> pnd.DataFrame:
        '''
        Parameters
        ---------------
        df      : Pandas dataframe with input
        particle: Name of particle for which columns are needed, e.g. L1

        Returns
        ---------------
        Dataframe with columns for X and Y axes added. These axes are the ones
        in function of which PIDCalib maps are parametrized
        '''
        for var in [self._varx, self._vary]:
            var = var.replace('PARTICLE', particle)
            if var in df.columns:
                log.debug(f'Variable {var} already found, not adding it')
                continue

            log.info(f'Adding column: {var}')
            df[var] = df.eval(var)

        return df
    # ------------------------------
    def _key_from_path(self, path : str) -> str:
        '''
        Parameters
        ---------------
        path: Path to pickle file holding the calibration map

        Returns
        ---------------
        Identifier, needed as key of dictionary holding maps
        '''
        file_name = os.path.basename(path)
        mtch = re.match(self._regex, file_name)
        if not mtch:
            raise ValueError(f'Cannot find block and particle in: {file_name}')

        [block, part] = mtch.groups()

        part = {'K': 'kaon', 'Pi' : 'pion'}[part]

        if   self._cfg['regions']['signal']  in file_name:
            region = 'signal'
        elif self._cfg['regions']['control'] in file_name:
            region = 'control'
        else:
            raise ValueError(f'Cannot determine region from: {file_name}')

        return f'{block}_{part}_{region}'
    # ------------------------------
    def _get_maps(self, kind : str) -> dict[str, bh]:
        '''
        Loads pickle files with PIDCalib2 efficiencies for
        kaons or pions and returns them

        Parameters
        ----------------
        kind: Describes things like brem or no brem or different binnings 

        Returns
        ----------------
        Dictionary mapping string identfying maps and
        boosthistogram object
        '''
        ana_dir = os.environ['ANADIR']
        pkl_dir = f'{ana_dir}/{self._cfg.maps_path}/{kind}'
        path_wc = f'{pkl_dir}/*.pkl'

        d_map = {}
        for path in glob.glob(path_wc):
            key = self._key_from_path(path)

            with open(path, 'rb') as ifile:
                try:
                    hist = pickle.load(ifile)
                except EOFError as exc:
                    raise EOFError(f'Cannot open map: {path}') from exc

            d_map[key] = hist

        return d_map
    # ------------------------------
    def _set_variables(self) -> None:
        l_var = self._cfg['pars']
        if len(l_var) != 2:
            raise NotImplementedError(f'Only 2D reweighing suppored, requested: {l_var}')

        self._varx = l_var[0]
        self._vary = l_var[1]
    # ------------------------------
    def _get_bin_index(
        self,
        hist  : bh,
        iaxis : int,
        value : float,
        name  : str) -> int:
        '''
        Parameters
        ------------------
        hist : Boost histogram
        iaxis: Axis index, 0 or 1
        value: Value of variable whose intex is read for `iaxis`
        name : Name of the variable corresponding to `iaxis`

        Returns
        ------------------
        Index of bin in map for given `value` in given histogram
        '''
        axis = hist.axes[iaxis]
        minv = axis.edges[ 0] * 1.001
        maxv = axis.edges[-1] * 0.999

        old_value = value
        new_value = max(old_value, minv)
        new_value = min(new_value, maxv)

        if old_value != new_value:
            is_max = old_value > maxv
            if name not in self._d_out_of_map:
                self._d_out_of_map[name] = {True : 0, False : 0}

            self._d_out_of_map[name][is_max] += 1

        index = axis.index(new_value)

        return index
    # ------------------------------
    def _get_lepton_eff(
        self,
        lep    : str,
        row    : pnd.Series,
        is_sig : bool) -> float:
        '''
        This method will return lepton PID efficiencies for a given lepton

        Parameters
        -----------------
        lep   : L1 or L2
        row   : Contains information on candidate
        is_sig: If True will provide signal region efficiencies
        '''
        # NOTE: This method will be called per candidate
        # Do not add heavy stuff
        if self._true_electron:
            return self._get_true_lepton_eff(lep=lep, row=row, is_sig=is_sig)

        return self._get_fake_lepton_eff(lep=lep, row=row, is_sig=is_sig)
    # ------------------------------
    def _get_true_lepton_eff(
        self,
        lep    : str,
        row    : pnd.Series,
        is_sig : bool) -> float:
        '''
        Parameters
        --------------------
        lep   : E.g. L1 or L2
        row   : Stores information on candidate, PID, PT, ETA, etc.
        is_sig: If true, need the efficiency for signal region cut, otherwise, control region.

        Returns
        --------------------
        Either 0 or 1, depending on wether the cut passes or fails
        '''
        # TODO: Replace this section with efficiency maps for electrons, when available
        region = {True : 'signal', False : 'control'}[is_sig]
        cut    = self._cfg['regions'][region]
        cut    = cut.replace('DLLe'    , f'{lep}_PID_E')
        cut    = cut.replace('PROBNN_E', f'{lep}_PROBNN_E')
        data   = row.to_dict()

        try:
            flag = numexpr.evaluate(cut, local_dict=data)
            flag = bool(flag)
        except Exception as exc:
            raise ValueError(f'Cannot evaluate {cut} on {data}') from exc

        val = {True : 1.0, False : 0.0}[flag]

        return val
    # ------------------------------
    def _get_fake_lepton_eff(
        self,
        lep    : str,
        row    : pnd.Series,
        is_sig : bool) -> float:
        '''
        Reads loaded PID efficiency maps and returns efficiency, for a given particle
        This method assumes that the particles is not a true electron, but a hadron

        Parameters
        ----------------
        lep   : L1 or L2
        row   : Contains candidate information
        is_sig: Used to pick correct efficiency map

        Returns
        ----------------
        Lepton PID efficiency
        '''
        block   = int(row.block)
        key_map = f'block{block}_{row.hadron}_signal' if is_sig else f'block{block}_{row.hadron}_control'
        brem_key= self._get_brem_key(lep=lep, row=row)
        try:
            hist    = self._d_map[brem_key][key_map]
        except KeyError as exc:
            for key in sorted(self._d_map):
                log.info(key)
            raise KeyError(f'Cannot pick up PID map: {key_map}') from exc

        varx = self._varx.replace('PARTICLE', lep)
        vary = self._vary.replace('PARTICLE', lep)

        x_value = getattr(row, varx)
        y_value = getattr(row, vary)

        ix = self._get_bin_index(hist, iaxis=0, value=x_value, name=varx)
        iy = self._get_bin_index(hist, iaxis=1, value=y_value, name=vary)
        eff= hist[ix, iy]

        if isinstance(eff, acc.WeightedSum):
            eff = eff.value

        if not isinstance(eff, float):
            etype = type(eff)
            raise NotImplementedError(f'Unrecognized efficiency of type: {etype}')

        eff = self._check_eff(eff=eff, x=x_value, y=y_value)

        return eff
    # ----------------------
    def _get_brem_key(self, lep : str, row : pnd.Series) -> str:
        '''
        Parameters
        -------------
        lep: Name of lepton in this iteration, e.g. L1
        row: Row containing information of candidate

        Returns
        -------------
        Identifier for brem category, e.g brem, nobrem
        '''
        name  = f'{lep}_HASBREM'
        value = row[name]

        if value == 1:
            return BREM 
        if value == 0:
            return NOBREM 

        raise ValueError(f'Invalid {name} value: {value}')
    # ------------------------------
    def _check_eff(self, eff : float, x : float, y : float) -> float:
        '''
        Parameters
        ---------------
        eff: Efficiency
        x/y: Coordinates in map associated to efficiency

        Returns
        ---------------
        efficiency after sanitation step
        '''
        if 0 <= eff <= 1:
            if   math.isclose(eff, 0, rel_tol=1e-5):
                self._d_quality['Zeroes'] += 1
            elif math.isclose(eff, 1, rel_tol=1e-5):
                self._d_quality['Ones'] += 1
            else:
                self._d_quality['Good'] += 1

            return eff

        if math.isinf(eff):
            log.debug('At:')
            log.debug(f'X={x:.2f}')
            log.debug(f'Y={y:.2f}')
            log.debug(f'Eff={eff:0.3} returning 0')
            self._d_quality['Inf'] += 1

            return 0.0

        if math.isnan(eff):
            log.debug('At:')
            log.debug(f'X={x:.2f}')
            log.debug(f'Y={y:.2f}')
            log.debug(f'Eff={eff:0.3} returning 0')
            self._d_quality['NaN'] += 1

            return 0.0

        log.debug('At:')
        log.debug(f'X={x:.2f}')
        log.debug(f'Y={y:.2f}')
        log.debug(f'Eff={eff:0.3}')

        if eff < 0:
            self._d_quality['Negative'] += 1
            return 0.0

        if eff > 1:
            self._d_quality['Above 1'] += 1
            return 1.0

        raise ValueError(f'Unexpected efficiency value: {eff}')
    # ------------------------------
    def _print_info_from_row(self, row : pnd.Series) -> None:
        log.info(40 * '-')
        log.info(f'Block/Hadron: {row.block}/{row.hadron}')
        log.info(40 * '-')
        for lepton in ['L1', 'L2']:
            varx = self._varx.replace('PARTICLE', lepton)
            vary = self._vary.replace('PARTICLE', lepton)

            valx = getattr(row, varx)
            valy = getattr(row, vary)

            log.info(f'{varx:<20}{valx:20.2f}')
            log.info(f'{vary:<20}{valy:20.2f}')
            log.info('')
    # ------------------------------
    def _get_transfer_weight(self, row : pnd.Series) -> float:
        '''
        transfer weight: What needs to be applied as weight to get sample in target region

        Parameter
        ---------------
        row: Dataframe row holding candidate and tracks information

        Returns
        ---------------
        Weight used to _transfer_ candidate in PID control region to signal region
        for FP, PF or FF regions.
        '''

        # NOTE: For MC
        # - We use noPID samples.
        # - We have either PID maps (misid MC) or we cut (signal)
        # The numerator is:
        # - The cut outcome, indicating that the electron is in the signal/control region, i.e. 0 or 1
        # - The efficiency from the maps for the signal/control region
        # The candidate's transfer function is the **probability** calculated from these track probabilities.

        if not self._sample.startswith('DATA_'):
            trf_eff = self._get_mc_candidate_efficiency(row=row, is_sig=self._is_sig)
            return trf_eff

        # NOTE: For data
        # - We cannot remove the PID.
        # - The transfer function needs to remove the PID cut through maps.
        # - The data will always be in the control region. We do not do this with data in signal region
        # I.e. transfer function = eff_target / eff_control for each lepton

        trf_eff = self._get_data_candidate_efficiency(row=row, is_sig=self._is_sig)
        ctr_eff = self._get_data_candidate_efficiency(row=row, is_sig=       False)
        if ctr_eff == 0:
            log.warning('Control efficiency is zero at:')
            self._print_info_from_row(row)
            return 1

        return trf_eff / ctr_eff
    # ------------------------------
    def _get_data_candidate_efficiency(self, row : pnd.Series, is_sig : bool) -> float:
        '''
        Parameters
        ------------------
        row   : Stores information on candidate
        is_sig: If true the probability is for the signal region

        Returns
        ------------------
        probability for the candidate to be in the signal or control region
        '''
        if   row.kind in ['PassFail', 'FailPass']:
            lep  = {'PassFail' : 'L2', 'FailPass' : 'L1'}[row.kind]
            eff  = self._get_lepton_eff(lep= lep, row=row, is_sig=is_sig)
        elif row.kind == 'FailFail':
            eff1 = self._get_lepton_eff(lep='L1', row=row, is_sig=is_sig)
            eff2 = self._get_lepton_eff(lep='L2', row=row, is_sig=is_sig)
            eff  = eff1 * eff2
        else:
            raise ValueError(f'Invalid kind: {row.kind}')

        return eff
    # ------------------------------
    def _get_mc_candidate_efficiency(self, row : pnd.Series, is_sig : bool) -> float:
        '''
        Parameter
        ---------------
        row: Dataframe row holding candidate and tracks information

        Returns
        ---------------
        Efficiency associated to either signal or control region
        '''
        eff_p1 = self._get_lepton_eff(lep='L1', row=row, is_sig= True)
        eff_p2 = self._get_lepton_eff(lep='L2', row=row, is_sig= True)

        if is_sig: # This is the signal region
            eff  = eff_p1 * eff_p2
            return eff

        eff_f1 = self._get_lepton_eff(lep='L1', row=row, is_sig=False)
        eff_f2 = self._get_lepton_eff(lep='L2', row=row, is_sig=False)
        eff    = eff_p1 * eff_f2 + eff_p2 * eff_f1 + eff_f1 * eff_f2

        return eff
    # ----------------------
    def _print_stats(self, wgt : numpy.ndarray) -> None:
        '''
        This method will print a summary of the number of entries that
        ended up outside the maps, too high or too low

        Parameters
        ----------------
        wgt: Array of weights
        '''
        log.info(40 * '-')
        log.info(f'{"Value":<20}{"Frequency":<20}')
        log.info(40 * '-')
        for kind, val in self._d_quality.items():
            log.info(f'{kind:<20}{val:<20}')
        log.info(40 * '-')
        log.info('')
        log.info(f'Processed {len(self._df)} entries')
        log.info(40 * '-')
        log.info(f'{"Variable":<20}{"Low":<10}{"High":<10}')
        log.info(40 * '-')
        for var, d_frq in self._d_out_of_map.items():
            val_low  = d_frq.get(False,0)
            val_high = d_frq.get(True ,0)
            log.info(f'{var:<20}{val_low:<10}{val_high:<10}')
        log.info(40 * '-')
        log.info('')
        nwgt = len(wgt)
        sumw = numpy.sum(wgt)

        log.info(f'{"Entries":<20}{nwgt:<20}'   )
        log.info(f'{"SumW   ":<20}{sumw:<20.2f}')
    # ------------------------------
    def get_weighted_data(self) -> pnd.DataFrame:
        '''
        Returns 
        --------------------
        Input dataframe with:

        - Variables to read X and Y axis defined
        - Updated `weight` column with PID weights
        - Attached `pid_weights` column
        '''
        if len(self._df) == 0:
            log.warning('Empty dataframe, not assigning any weight')
            return self._df

        log.info(f'Getting signal={self._is_sig} PID weights for sample {self._sample}')

        try:
            sr_wgt = self._df.apply(self._get_transfer_weight, axis=1)
            arr_wgt= sr_wgt.to_numpy()
        except AttributeError as exc:
            log.warning('Found columns:')
            for column in self._df.columns:
                log.info(f'    {column}')
            raise AttributeError('Cannot assign weight') from exc

        self._print_stats(wgt=arr_wgt)

        self._df['weight']           *= arr_wgt
        self._df.attrs['pid_weights'] = arr_wgt.tolist()

        return self._df
# ------------------------------
