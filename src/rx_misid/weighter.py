'''
Module with SampleWeighter class
'''
import os
import re
import glob
import pickle
import pandas          as pnd
import boost_histogram as bh
from dmu.logging.log_store     import LogStore

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
    def __init__(self, df : pnd.DataFrame, cfg : dict):
        '''
        df  : Pandas dataframe with columns 'hadron', 'bmeson' and 'kind'. Used to assign weights
        cfg : Dictionary storing configuration
        '''
        self._df  = df
        self._cfg = cfg

        self._varx : str
        self._vary : str
        self._set_variables()

        self._regex = r'.*_(block\d)(?:_v\d)?-(?:up|down)-(K|Pi)-.*'

        self._d_map : dict[str, bh] = self._load_maps()
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
                hist = pickle.load(ifile)

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
    def _get_lepton_weight(self, lep : str, row : pnd.Series) -> float:
        key_sig = f'{row.block}_{row.hadron}_signal'
        key_ctr = f'{row.block}_{row.hadron}_control'

        hist_signal  = self._d_map[key_sig]
        hist_control = self._d_map[key_ctr]

        x_value = getattr(row, f'{lep}_{self._varx}')
        y_value = getattr(row, f'{lep}_{self._vary}')

        ix = hist_signal.axes[0].index(x_value)
        iy = hist_signal.axes[1].index(y_value)

        eff_signal  = hist_signal [ix, iy]
        eff_control = hist_control[ix, iy]

        if eff_control.value == 0:
            log.warning(f'Found zero control efficiency for {row.block}/{row.hadron} at: ({x_value:.2f},{y_value:.2f})')
            return 1

        weight = eff_signal.value / eff_control.value
        if weight < 0:
            log.warning(f'Found negative weight for {row.block}/{row.hadron} at: ({x_value:.2f},{y_value:.2f})')
            return 1

        return weight
    # ------------------------------
    def _get_candidate_weight(self, row : pnd.Series) -> float:
        if   row.kind == 'PassFail':
            w = self._get_lepton_weight(lep='L2', row=row)
        elif row.kind == 'FailPass':
            w = self._get_lepton_weight(lep='L1', row=row)
        elif row.kind == 'FailFail':
            w1 = self._get_lepton_weight(lep='L1', row=row)
            w2 = self._get_lepton_weight(lep='L2', row=row)
            w  = w1 * w2
        else:
            raise ValueError(f'Invalid kind: {row.kind}')

        return w
    # ------------------------------
    def get_weighted_data(self) -> pnd.DataFrame:
        '''
        Returns instance of weighted data
        '''
        self._df['weights'] = self._df.apply(self._get_candidate_weight, axis=1)

        return self._df
# ------------------------------
