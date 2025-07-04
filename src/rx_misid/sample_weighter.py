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
            df   : pnd.DataFrame,
            mode : str,
            cfg  : dict):
        '''
        df  : Pandas dataframe with columns 'hadron', 'bmeson' and 'kind'. Used to assign weights
        mode: Controls what kind of weight is assigned to candidate; transfer, control or signal
        cfg : Dictionary storing configuration
        '''
        self._cfg = cfg
        if mode not in ['transfer', 'control', 'signal']:
            raise ValueError(f'Invalid weight kind: {mode}')

        self._mode = mode

        self._varx : str
        self._vary : str
        self._set_variables()
        self._df   = self._get_df(df)

        self._regex = r'.*_(block\d)(?:_v\d)?-(?:up|down)-(K|Pi)-.*'

        self._d_map        : dict[str, bh] = self._load_maps()
        self._d_out_of_map : dict[str,dict[int,int]] = {}
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
    def _get_lepton_eff(self, lep : str, row : pnd.Series, is_sig : bool) -> float:
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

        if isinstance(eff, float):
            return eff

        if isinstance(eff, acc.WeightedSum):
            return eff.value

        etype = type(eff)
        raise NotImplementedError(f'Unrecognized efficiency of type: {etype}')
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
    def _get_candidate_weight(self, row : pnd.Series) -> float:
        if self._mode == 'transfer':
            return self._get_transfer_weight(row=row)

        return self._get_efficiency(row=row)
    # ------------------------------
    def _get_efficiency(self, row : pnd.Series) -> float:
        '''
        Parameter
        ---------------
        row: Dataframe row holding candidate and tracks information

        Returns
        ---------------
        Efficiency associated to either signal or control region
        '''
        is_sig = { 'signal' : True, 'control' : False }[self._mode]

        eff1 = self._get_lepton_eff(lep='L1', row=row, is_sig=is_sig)
        eff2 = self._get_lepton_eff(lep='L2', row=row, is_sig=is_sig)
        eff  = eff1 * eff2

        if 1 < eff:
            log.warning(f'Returning 1.0 instead of efficiency: {eff:.3f}')
            return 1.0

        if eff < 0:
            log.warning(f'Returning 0.0 instead of efficiency: {eff:.3f}')
            return 0.0

        return eff
    # ------------------------------
    def _get_transfer_weight(self, row : pnd.Series) -> float:
        '''
        Parameter
        ---------------
        row: Dataframe row holding candidate and tracks information

        Returns
        ---------------
        Weight used to _transfer_ candidate in PID control region to signal region
        for FP, PF or FF regions.
        '''
        if   row.kind == 'PassFail':
            num  = self._get_lepton_eff(lep='L2', row=row, is_sig= True)
            den  = self._get_lepton_eff(lep='L2', row=row, is_sig=False)
        elif row.kind == 'FailPass':
            num  = self._get_lepton_eff(lep='L1', row=row, is_sig= True)
            den  = self._get_lepton_eff(lep='L1', row=row, is_sig=False)
        elif row.kind == 'FailFail':
            eff1 = self._get_lepton_eff(lep='L1', row=row, is_sig= True)
            eff2 = self._get_lepton_eff(lep='L2', row=row, is_sig= True)
            eff3 = self._get_lepton_eff(lep='L1', row=row, is_sig=False)
            eff4 = self._get_lepton_eff(lep='L2', row=row, is_sig=False)

            num  = eff1 * eff2
            den  = eff3 * eff4
        else:
            raise ValueError(f'Invalid kind: {row.kind}')

        if den == 0:
            log.warning('Control efficiency is zero at:')
            self._print_info_from_row(row)
            return 1

        return num / den
    # ------------------------------
    def get_weighted_data(self) -> pnd.DataFrame:
        '''
        Returns instance of weighted data
        '''
        if len(self._df) == 0:
            log.warning('Empty dataframe, not assigning any weight')
            return self._df

        try:
            self._df['weight'] *= self._df.apply(self._get_candidate_weight, axis=1)
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
