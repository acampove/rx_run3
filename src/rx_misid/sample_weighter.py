'''
Module with SampleWeighter class
'''
import os
import re
import glob
import pickle

import pandas  as pnd
from boost_histogram        import Histogram    as bh
from boost_histogram        import accumulators as acc
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_misid:weighter')
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
            cfg    : dict):
        '''
        df    : Pandas dataframe with columns 'hadron', 'bmeson' and 'kind'. Used to assign weights
        is_sig: If true, the weights will provide signal region sample, otherwise control region
        sample: E.g. DATA_24_... Needed to pick maps based on actual particle identity
        cfg   : Dictionary storing configuration
        '''
        self._cfg    = cfg
        self._is_sig = is_sig
        self._sample = sample
        self._varx   : str
        self._vary   : str

        self._l_electron_sample = ['Bu_JpsiK_ee_eq_DPC', 'Bu_Kee_eq_btosllball05_DPC']
        self._l_hadron_sample   = ['Bu_piplpimnKpl_eq_sqDalitz_DPC']
        self._regex             = r'.*_(block\d)(?:_v\d)?-(?:up|down)-(K|Pi)-.*'

        self._d_out_of_map : dict[str,dict[int,int]] = {}

        self._set_variables()
        self._df                           = self._get_df(df)
        self._d_map        : dict[str, bh] = self._load_maps()
        self._true_electron                = self._is_true_electron()
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
        df = self._add_columns(df=df, particle='L1')
        df = self._add_columns(df=df, particle='L2')

        return df
    # ------------------------------
    def _add_columns(self, df : pnd.DataFrame, particle : str) -> pnd.DataFrame:
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
    def _load_maps(self) -> dict[str, bh]:
        pkl_dir = self._cfg['path']
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
    def _get_bin_index(self,
                       hist  : bh,
                       iaxis : int,
                       value : float,
                       name  : str) -> int:
        axis = hist.axes[iaxis]
        minv = axis.edges[ 0] * 1.001
        maxv = axis.edges[-1] * 0.999

        old_value = value
        new_value = max(old_value, minv)
        new_value = min(new_value, maxv)

        if old_value != new_value:
            #log.debug(f'Moving {name} value inside map {old_value:.5f} -> {new_value:.5f}')

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
        cut    = cut.replace('|',  ' or ')
        cut    = cut.replace('&', ' and ')

        data   = row.to_dict()
        try:
            flag = eval(cut, {}, data)
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
        hist    = self._d_map[key_map]

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
            return eff

        log.debug('At:')
        log.debug(f'X={x:.2f}')
        log.debug(f'Y={y:.2f}')
        log.info(f'Eff={eff:0.3}')

        if eff < 0:
            return 0.0

        if eff > 1:
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
        eff_f1 = self._get_lepton_eff(lep='L1', row=row, is_sig=False)
        eff_f2 = self._get_lepton_eff(lep='L2', row=row, is_sig=False)

        if is_sig: # This is the signal region
            eff  = eff_p1 * eff_p2
        else:      # This is the control region
            eff  = eff_p1 * eff_f2 + eff_p2 * eff_f1 + eff_f1 * eff_f2

        return eff
    # ------------------------------
    def get_weighted_data(self) -> pnd.DataFrame:
        '''
        Returns instance of weighted data
        '''
        if len(self._df) == 0:
            log.warning('Empty dataframe, not assigning any weight')
            return self._df

        try:
            self._df['weight'] *= self._df.apply(self._get_transfer_weight, axis=1)
        except AttributeError as exc:
            log.info(self._df.dtypes)
            log.info(self._df.columns)
            log.info(self._df)
            raise AttributeError('Cannot assign weight') from exc

        log.info(f'Processed {len(self._df)} entries')
        log.info(40 * '-')
        log.info(f'{"Variable":<20}{"Low":<10}{"High":<20}')
        log.info(40 * '-')
        for var, d_frq in self._d_out_of_map.items():
            val_low  = d_frq.get(False,0)
            val_high = d_frq.get(True ,0)
            log.info(f'{var:<20}{val_low:<10}{val_high:<20}')
        log.info(40 * '-')

        return self._df
# ------------------------------
