'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

from pathlib import Path
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
    mass = 'B_Mass_hdpipi'
    #mass = 'B_Mass_hdkk'

    #sample = 'Bu_piplpimnKpl_eq_sqDalitz_DPC'
    sample = 'DATA_24*'

    with RDFGetter.multithreading(nthreads = 10):
        gtr = RDFGetter(sample = sample, trigger = Trigger(trig))
        rdf = gtr.get_rdf(per_file = False)

        tg_cut = 'L1_PROBNN_K < 0.1 && L1_PROBNN_K < 0.1'
        #tg_cut = 'L1_PROBNN_K > 0.1 && L1_PROBNN_K > 0.1'

        l1_cut = 'L1_PROBNN_E < 0.2 || L1_PID_E < 3.0'
        l2_cut = 'L2_PROBNN_E < 0.2 || L2_PID_E < 3.0'
        mv_cut = '(mva_cmb > 0.50) && (mva_prc > 0.30)'

        with sel.custom_selection(d_sel = {'pid_l' : f'({l1_cut}) && ({l2_cut}) && ({tg_cut})', 'bdt' : mv_cut}):
            rdf = sel.apply_full_selection(
                rdf     = rdf, 
                q2bin   = qsq, 
                process = sample, 
                out_path= Path('./cutflow'),
                trigger = trig)

        arr_mass = rdf.AsNumpy([mass])[mass]

    plt.hist(arr_mass, bins = 100, range=(5000, 5500), label = 'Control region', color='black', alpha=0.5)
    plt.axvline(x=5280, color='red', linestyle=':', label = '$M(B^0)$')
    plt.legend()
    plt.show()
# ----------------------
if __name__ == '__main__':
    app()
