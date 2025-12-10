'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import typer
import mplhep
import matplotlib.pyplot   as plt

from dmu.logging.log_store import LogStore
from rx_data.rdf_getter    import RDFGetter
from rx_common.types       import Trigger, Qsq
from rx_common             import info
from rx_plotter.refitting  import plot      as refitting_plot
from rx_selection          import selection as sel

plt.style.use(mplhep.style.LHCb2)
app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_plots:cli')
# ----------------------
@app.command()
def refitting(
    max_evt : int     = typer.Option(-1 , '--max_evt', '-m', help='Maximum number of entries'),
    log_lvl : int     = typer.Option(20 , '--log_lvl', '-l', help='Logging level'),
    nthread : int     = typer.Option(4  , '--nthread', '-n', help='Number of threads'),
    project : str     = typer.Option(..., '--project', '-p', help='E.g. rk, rkst'),
    qsq     : Qsq     = typer.Option(..., '--qsq'    , '-q', help='q2 bin'),
    trigger : Trigger = typer.Option(..., '--trigger', '-t')) -> None:
    '''
    This will compare refitted and non-refitted data
    '''
    if project not in ['rk', 'rkst']:
        raise ValueError(f'Invalid project: {project}')

    LogStore.set_level('rx_plots:refitting', log_lvl)

    if max_evt > 0:
        log.warning(f'Running over {max_evt} entries')
        with RDFGetter.max_entries(value = max_evt):
            refitting_plot(project=project, qsq=qsq, trigger=trigger)
    else:
        log.info(f'Running over full dataset with {nthread} threads')
        with RDFGetter.multithreading(nthreads = nthread):
            refitting_plot(project=project, qsq=qsq, trigger=trigger)
# ----------------------
@app.command()
def control_region(
    chan : str = typer.Option(... , '--chan', '-c', help='E.g. EE'      ),
    kind : str = typer.Option(... , '--kind', '-k', help='E.g, OS, SS, EXT'),
    proj : str = typer.Option(... , '--proj', '-p', help='E.g. RK'      ),
    qsq  : str = typer.Option(... , '--qsq' , '-q', help='E.g. central' )) -> None:
    '''
    This can be used to plot control regions for data
    '''
    trig = info.get_trigger(project = proj, channel = chan, kind = kind)

    gtr = RDFGetter(sample = 'DATA_24*', trigger = Trigger(trig))
    rdf = gtr.get_rdf(per_file = False)
# ----------------------
if __name__ == '__main__':
    app()
