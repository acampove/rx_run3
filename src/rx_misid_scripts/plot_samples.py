'''
This script is meant to:

- Plot pT and Eta distributions with and without PID weights in brem categories
- Plot map overlaid to countour plots of misID samples

where the misID samples are B-> KKK and B -> Kpipi
'''
import argparse
from dataclasses import dataclass, field

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
        self._resolve_sample()
        self._build_rdf_getter(cfg=cfg)

        self._weighter = cfg.wgt_cfg
        self._splitter = cfg.spl_cfg
    # ----------------------
    def _resolve_sample(self) -> None:
        '''
        Transform nickname into actual sample name
        '''
        if self.sample == 'kkk':
            self.sample = 'Bu_KplKplKmn_eq_sqDalitz_DPC'

        if self.sample == 'kpipi':
            self.sample = 'Bu_piplpimnKpl_eq_sqDalitz_DPC'

        raise NotImplementedError(f'Invalid sample: {self.sample}')
    # ----------------------
    def _build_rdf_getter(self, cfg : DictConfig) -> None:
        '''
        Parameters
        -------------
        cfg : Configuration from yaml, needed to update current class
        '''
        cfg.sample = self.sample 

        self.cfg = OmegaConf.to_container(cfg, resolve=True)
# ----------------------
def _parse_args() -> PlotConfig:
    cfg    = PlotConfig()
    parser = argparse.ArgumentParser(description='Script used to make diagnostic plots')
    parser.add_argument('-r', '--region' , type=int, help='Signal region (1) or control region', choices=[0, 1]      , required=True)
    parser.add_argument('-s', '--sample' , type=str, help='Sample nickname'                    , choices=cfg.samples , required=True)
    parser.add_argument('-b', '--block'  , type=str, help='Block'                              , choices=cfg.blocks  , required=True)
    parser.add_argument('-B', '--bremcat', type=int, help='Brem category'                      , choices=cfg.bremcats, required=True)
    args = parser.parse_args()

    cfg.is_sig = bool(args.region)
    cfg.sample = args.sample
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
    gtr   = RDFGetter(**cfg.rdf_getter)
    rdf   = gtr.get_rdf(per_file=False)

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
    pass
# ----------------------
def main():
    '''
    Entry point
    '''
    cfg = _parse_args()
    df  = _get_df(cfg=cfg)

    _plot_projections(df=df, cfg=cfg)
# ----------------------
if __name__ == '__main__':
    main()
