'''
Module in charge of plotting refitted vs non-refitted data
'''

import os

from ROOT                    import RDF # type: ignore
from dmu.generic             import utilities as gut
from dmu.plotting.plotter_1d import Plotter1D as Plotter
from rx_common.types         import Project, Qsq, Trigger 
from rx_data.rdf_getter      import RDFGetter

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_plots:refitting')
# ----------------------
def _get_rdf(trigger : Trigger, qsq : Qsq, project : str) -> RDF.RNode:
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
    with RDFGetter.project(name = project):
        gtr = RDFGetter(sample = 'DATA_24*', trigger = trigger)
        rdf = gtr.get_rdf(per_file = False)

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

    rdf_org = _get_rdf(trigger = trigger, qsq = qsq, project = f'{project}_no_refit')
    rdf_rft = _get_rdf(trigger = trigger, qsq = qsq, project =    project)

    d_rdf   = {'Original' : rdf_org, 'Refitted' : rdf_rft}

    ana_dir = os.environ['ANADIR']
    cfg = gut.load_conf(package='rx_plotter_data', fpath = 'refitting/v1.yaml')
    cfg.saving = f'{ana_dir}/plots/refitting'

    ptr = Plotter(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
