'''
Module in charge of plotting refitted vs non-refitted data
'''

import os

from ROOT                    import RDF # type: ignore
from pathlib                 import Path
from dmu.generic             import utilities as gut
from dmu.plotting.plotter_1d import Plotter1D as Plotter
from rx_common.types         import Qsq, Trigger 
from rx_data.rdf_getter      import RDFGetter
from rx_selection            import selection as sel

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_plots:refitting')
# ----------------------
def _get_rdf(
    trigger : Trigger, 
    qsq     : Qsq, 
    project : str,
    out_path: Path) -> RDF.RNode:
    '''
    Parameters
    -------------
    channel: Electron or muon
    qsq    : q2 bin
    project: Refitted or not

    Returns
    -------------
    Dataframe after full selection
    '''
    sample = 'DATA_24*'

    with RDFGetter.project(name = project):
        gtr = RDFGetter(sample = sample, trigger = trigger)
        rdf = gtr.get_rdf(per_file = False)

    rdf = sel.apply_full_selection(
        rdf     = rdf, 
        process = sample, 
        q2bin   = qsq, 
        trigger = trigger,
        out_path= out_path / project)

    return rdf
# ----------------------
def plot(project : str, qsq : Qsq, trigger : Trigger):
    '''
    Parameters
    -----------------
    project: E.g. rk or rkst
    qsq    : q2 bin
    channel: Electron or muon channel 
    '''
    ana_dir = Path(os.environ['ANADIR'])
    plt_dir = ana_dir / 'plots/refitting'

    rdf_org = _get_rdf(trigger = trigger, qsq = qsq, out_path = plt_dir, project = f'{project}_no_refit')
    rdf_rft = _get_rdf(trigger = trigger, qsq = qsq, out_path = plt_dir, project =    project)

    d_rdf   = {'Original' : rdf_org, 'Refitted' : rdf_rft}
    cfg = gut.load_conf(package='rx_plotter_data', fpath = 'refitting/v1.yaml')
    cfg['saving'] = {'plt_dir' : plt_dir}

    ptr = Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ----------------------
