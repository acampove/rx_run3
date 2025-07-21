'''
Script meant to do comparisons of variables between Run1,2 and 3
'''
import argparse

from ROOT                    import RDF
from omegaconf               import DictConfig
from dmu.generic             import utilities as gut
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_1d import Plotter1D
from rx_data.rdf_getter      import RDFGetter
from rx_data.rdf_getter12    import RDFGetter12

log=LogStore.add_logger('rx_plotter:run123')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    cfg : DictConfig
# ----------------------
def _initialize() -> None:
    '''
    Nothing, this function initializes the state of the Data class
    '''
    args = _parse_args()

    Data.cfg = gut.load_conf(package='rx_plotter_data', fpath=f'run123/{args.conf}.yaml')
# ----------------------
def _get_rdf(dset : str) -> RDF.RNode:
    '''
    Parameters
    -------------
    dset: Label for dataset, e.g. 2024, run12

    Returns
    -------------
    ROOT dataframe with corresponding data
    '''
    sample = Data.cfg.input.sample
    trigger= Data.cfg.input.trigger

    if   dset == '2024':
        gtr = RDFGetter(sample=sample, trigger=trigger)
    elif dset in 'run12':
        gtr = RDFGetter12(
            sample =sample,
            trigger=trigger,
            dset   ='all')
    else:
        raise NotImplementedError(f'Invalid dataset {dset}')

    return gtr.get_rdf()
# ----------------------
def _get_dataframes() -> dict[str,RDF.RNode]:
    '''
    Returns
    -------------
    Dictionary with:
    Key  : Label for dataset, e.g. run12, 2014
    Value: Dataframe with that data corresponding to sample and trigger from config
    '''
    d_rdf = {}
    for dataset in Data.cfg.comparison:
        d_rdf[dataset] = _get_rdf(dset=dataset)

    return d_rdf
# ----------------------
def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Script used to make comparisons of variables between Run1,2 and 3')
    parser.add_argument('-c', '--conf' , type=str, help='Name of config file, e.g. mass', required=True)
    args = parser.parse_args()

    return args
# ----------------------
def main():
    '''
    Entry point
    '''
    _initialize()
    d_rdf = _get_dataframes()

    ptr   = Plotter1D(d_rdf=d_rdf, cfg=Data.cfg)
    ptr.run()
# ----------------------
if __name__ == '__main__':
    main()
