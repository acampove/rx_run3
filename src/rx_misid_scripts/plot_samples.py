'''
This script is meant to:

- Plot pT and Eta distributions with and without PID weights in brem categories
- Plot map overlaid to countour plots of misID samples

where the misID samples are B-> KKK and B -> Kpipi
'''
import os
import argparse
from dataclasses import dataclass, field

import numpy
import pandas       as pnd
from omegaconf                import DictConfig, OmegaConf
from rx_data.rdf_getter       import RDFGetter
from rx_misid.sample_splitter import SampleSplitter
from rx_misid.sample_weighter import SampleWeighter
from rx_selection             import selection  as sel
from dmu.logging.log_store    import LogStore
from dmu.generic              import utilities  as gut
from dmu.workflow.cache       import Cache      as Wcache

log=LogStore.add_logger('rx_misid:plot_samples')
# ----------------------
@dataclass
class PlotConfig:
    sample   : str

    SAMPLES  = ['kkk', 'kpipi']
    BLOCKS   = [1, 2, 3, 4, 5, 6, 7, 8]
    BREMCATS = [0, 1]
    Q2BIN    = ['low', 'central', 'high']

    block    : str        = field(init=False) 
    q2bin    : str        = field(init=False) 
    bremcat  : int        = field(init=False) 
    is_sig   : bool       = field(init=False) 

    rdf_cfg  : dict       = field(init=False) 
    weighter : DictConfig = field(init=False)
    splitter : DictConfig = field(init=False)
    # ----------------------
    def __post_init__(self):
        cfg = gut.load_conf(package='rx_misid_data', fpath='sample_plotting.yaml')
        this_dir = os.getcwd()
        Wcache.set_cache_root(f'{this_dir}/.cache')

        self._build_rdf_getter(cfg=cfg)
        self.weighter = cfg.wgt_cfg
        self.splitter = cfg.spl_cfg
    # ----------------------
    def _resolve_sample(self) -> str:
        '''
        Transform nickname into actual sample name
        '''
        if self.sample == 'kkk':
            return 'Bu_KplKplKmn_eq_sqDalitz_DPC'

        if self.sample == 'kpipi':
            return 'Bu_piplpimnKpl_eq_sqDalitz_DPC'

        raise NotImplementedError(f'Invalid sample: {self.sample}')
    # ----------------------
    def _build_rdf_getter(self, cfg : DictConfig) -> None:
        '''
        Parameters
        -------------
        cfg : Configuration from yaml, needed to update current class
        '''
        d_cfg      = OmegaConf.to_container(cfg.rdf_getter, resolve=True)
        if not isinstance(d_cfg, dict):
            raise TypeError('Config is not a dictionary')

        d_cfg['sample'] = self._resolve_sample()

        self.rdf_cfg = d_cfg
# ----------------------
def _parse_args() -> PlotConfig:
    parser = argparse.ArgumentParser(description='Script used to make diagnostic plots')
    parser.add_argument('-r', '--region' , type=int, help='Signal region (1) or control region', choices=[0, 1]             , required=True)
    parser.add_argument('-q', '--q2bin'  , type=str, help='Q2 bin'                             , choices=PlotConfig.Q2BIN   , required=True)
    parser.add_argument('-s', '--sample' , type=str, help='Sample nickname'                    , choices=PlotConfig.SAMPLES , required=True)
    parser.add_argument('-b', '--block'  , type=int, help='Block'                              , choices=PlotConfig.BLOCKS  , required=True)
    parser.add_argument('-B', '--bremcat', type=int, help='Brem category'                      , choices=PlotConfig.BREMCATS, required=True)
    args = parser.parse_args()

    cfg        = PlotConfig(sample=args.sample)
    cfg.is_sig = bool(args.region)
    cfg.q2bin  = args.q2bin
    cfg.block  = args.block
    cfg.bremcat= args.bremcat

    return cfg
# ----------------------
def _get_df(cfg : PlotConfig) -> pnd.DataFrame:
    '''
    Parameters
    -------------
    cfg: Configuration needed to make dataframe

    Returns
    -------------
    Pandas dataframe with PID weighted sample
    '''
    gtr   = RDFGetter(**cfg.rdf_cfg)
    rdf   = gtr.get_rdf(per_file=False)
    uid   = gtr.get_uid()
    cuts  = {
        'pid_l' : '(1)', 
        'brem'  : 'nbrem != 0'} # Do not care about brem 0 candidates

    with sel.custom_selection(d_sel = cuts):
        rdf = sel.apply_full_selection(
            rdf    = rdf,
            uid    = uid,
            q2bin  = cfg.q2bin,
            process= cfg.rdf_cfg['sample'],
            trigger= cfg.rdf_cfg['trigger'])

    spl   = SampleSplitter(rdf = rdf, cfg = cfg.splitter)
    df    = spl.get_sample()

    wgt   = SampleWeighter(
        df    = df,
        cfg   = cfg.weighter,
        sample= cfg.sample,
        is_sig= cfg.is_sig)
    df  = wgt.get_weighted_data()

    return df
# ----------------------
def _plot_projections(df : pnd.DataFrame, cfg : PlotConfig) -> None:
    '''
    Parameters
    -------------
    df : DataFrame with data to be plotted
    cfg: Configuration to make plots
    '''
    arr_pt = _get_array(df=df, quantity='TRACK_PT' , cfg=cfg)
    arr_et = _get_array(df=df, quantity='TRACK_ETA', cfg=cfg)
    arr_wt = _get_array(df=df, quantity='weight'   , cfg=cfg)
# ----------------------
def _get_array(
    df       : pnd.DataFrame,
    quantity : str,
    cfg      : PlotConfig) -> numpy.ndarray:
    '''
    Parameters
    -------------
    df      : DataFrame with data to be plotted
    quantity: E.g. PT
    cfg     : Object with full configuration

    Returns
    -------------
    Array of `quantity` for L1 and L2
    '''
    df_l1 = df[ df['L1_HASBREM'] == cfg.bremcat ]
    name  = 'pid_eff_l1' if quantity == 'weight' else f'L1_{quantity}'
    arr_l1= df_l1[name].to_numpy()

    df_l2 = df[ df['L2_HASBREM'] == cfg.bremcat ]
    name  = 'pid_eff_l2' if quantity == 'weight' else f'L2_{quantity}'
    arr_l2= df_l2[name].to_numpy()

    return numpy.concatenate((arr_l1, arr_l2))
# ----------------------
def main():
    '''
    Entry point
    '''
    cfg = _parse_args()
    df  = _get_df(cfg=cfg)

    for block, df_block in df.groupby('block'):
        if block != cfg.block:
            continue

        log.info(f'Plotting block: {block}')
        _plot_projections(df=df_block, cfg=cfg)
# ----------------------
if __name__ == '__main__':
    main()
