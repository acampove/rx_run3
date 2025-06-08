'''
Script in charge of comparing different classifiers' performances
'''
import glob
import argparse
from importlib.resources   import files

import joblib
import matplotlib.pyplot as plt

from ROOT                  import RDataFrame
from dmu.ml.cv_performance import CVPerformance
from dmu.ml.cv_classifier  import CVClassifier

from dmu.logging.log_store import LogStore
from dmu.generic           import utilities as gut
from rx_data.rdf_getter    import RDFGetter
from rx_selection          import selection as sel

log=LogStore.add_logger('rx_classifier:compare_classifier')
# -------------------------------
class Data:
    '''
    Data class
    '''
    cfg : dict
    nev : int
    cvp = CVPerformance()
# -------------------------------
def _initialize():
    LogStore.set_level('rx_data:rdf_getter'    , 30)
    LogStore.set_level('rx_selection:selection', 30)
# -------------------------------
def _load_config(name : str) -> dict:
    fpath = files('rx_classifier_data').joinpath(f'performance/{name}.yaml')
    fpath = str(fpath)
    data  = gut.load_json(fpath)

    return data
# -------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to compare performance of classifiers')
    parser.add_argument('-c', '--config', type=str, help='Name of config file', required=True)
    parser.add_argument('-n', '--nevent', type=int, help='Number of entries to limit run')
    args = parser.parse_args()

    Data.cfg = _load_config(name=args.config)
    Data.nev = args.nevent
# -------------------------------
def _get_models(path : str) -> list[CVClassifier]:
    path_wc = f'{path}/model_*.pkl'
    l_path  = glob.glob(path_wc)
    l_model = [ joblib.load(path) for path in l_path ]

    if len(l_model) == 0:
        raise ValueError(f'No models found in: {path_wc}')

    return l_model
# -------------------------------
def _override_selection(cfg : dict, d_sel : dict[str,str]) -> dict[str,str]:
    if 'selection' not in cfg:
        return d_sel

    d_cut=cfg['selection']
    d_sel.update(d_cut)

    return d_sel
# -------------------------------
def _apply_selection(rdf : RDataFrame, cfg : dict) -> RDataFrame:
    name  = cfg['name']
    q2bin = cfg['q2bin']
    trig  = cfg['trigger']

    d_sel = sel.selection(
            q2bin  =q2bin,
            trigger=trig,
            process=name,
            smeared=True)

    d_sel = _override_selection(cfg=cfg, d_sel=d_sel)
    for name, expr in d_sel.items():
        rdf = rdf.Filter(expr, name)

    return rdf
# -------------------------------
def _get_rdf(cfg : dict) -> RDataFrame:
    name = cfg['name']
    trig = cfg['trigger']

    gtr  = RDFGetter(sample=name, trigger=trig)
    rdf  = gtr.get_rdf()
    rdf  = _apply_selection(rdf, cfg)

    if Data.nev is None:
        return rdf

    log.warning(f'Limiting run to {Data.nev} entries')
    rdf = rdf.Range(Data.nev)

    return rdf
# -------------------------------
def _plot_roc(kind : str, cfg : dict) -> None:
    rdf_sig = _get_rdf(cfg['samples']['signal'    ])
    rdf_bkg = _get_rdf(cfg['samples']['background'])
    color   = cfg['color']
    models  = _get_models(cfg['model'])

    Data.cvp.plot_roc(
        sig  =rdf_sig, bkg=rdf_bkg,
        model=models , name=kind, color=color)
# -------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _initialize()

    log.info('Plotting:')
    for kind, cfg in Data.cfg.items():
        log.info(f'{"":<4}{kind:<20}')
        _plot_roc(kind=kind, cfg=cfg)

    plt.show()
# -------------------------------
if __name__ == '__main__':
    main()
