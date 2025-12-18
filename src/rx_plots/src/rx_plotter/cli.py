'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import os
import typer
import mplhep
import pandas              as pnd
import matplotlib.pyplot   as plt

from pathlib               import Path
from dmu.workflow          import Cache
from dmu                   import LogStore
from rx_efficiencies       import CXCalculator
from rx_data.rdf_getter    import RDFGetter
from rx_common             import Trigger, Qsq, Project
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
    mass : str = typer.Option(... , '--mass', '-m', help='E.g, B_Mass_hdkk, B_Mass_hdpipi'),
    proj : str = typer.Option(... , '--proj', '-p', help='E.g. RK'      ),
    qsq  : str = typer.Option(... , '--qsq' , '-q', help='E.g. central' )) -> None:
    '''
    This can be used to plot control regions for data
    '''
    trig   = info.get_trigger(project = proj, channel = chan, kind = kind)
    sample = 'DATA_24*'

    with RDFGetter.multithreading(nthreads = 10):
        gtr = RDFGetter(sample = sample, trigger = Trigger(trig))
        rdf = gtr.get_rdf(per_file = False)

        if   mass == 'B_Mass_hdpipi':
            tg_cut = 'L1_PROBNN_K < 0.1 && L1_PROBNN_K < 0.1'
        elif mass == 'B_Mass_hdkk':
            tg_cut = 'L1_PROBNN_K > 0.1 && L1_PROBNN_K > 0.1'
        else:
            raise ValueError(f'Invalid mass: {mass}')

        l1_cut = 'L1_PROBNN_E < 0.2 || L1_PID_E < 3.0'
        l2_cut = 'L2_PROBNN_E < 0.2 || L2_PID_E < 3.0'
        mv_cut = '(mva_cmb > 0.50) && (mva_prc > 0.50)'

        with sel.custom_selection(d_sel = {'pid_l' : f'({l1_cut}) && ({l2_cut}) && ({tg_cut})', 'bdt' : mv_cut}):
            rdf = sel.apply_full_selection(
                rdf     = rdf, 
                q2bin   = qsq, 
                process = sample, 
                out_path= Path('./cutflow'),
                trigger = trig)

        arr_mass = rdf.AsNumpy([mass])[mass]

    lmass = {'B_Mass_hdpipi' : r'$M_{e\to\pi}$', 'B_Mass_hdkk' : r'$M_{e\to K}$'}[mass]
    label = {'RK' : f'{lmass}$(B^+)$[MeV]' , 'RKst' : f'{lmass}$(B^0)$[MeV]'}[proj]

    title = f'{l1_cut}\n {l2_cut}\n {mv_cut}\n {tg_cut}' 
    plt.hist(arr_mass, bins = 100, range=(5000, 5500), label = 'Control region', color='blue', alpha=0.5)
    plt.axvline(x=5280, color='red', linestyle=':', label = 'PDG')
    plt.title(title)
    plt.xlabel(label)
    plt.legend()

    name     = f'{chan}_{mass}_{proj}_{qsq}'
    ana_dir  = Path(os.environ['ANADIR'])
    out_path = ana_dir / f'plots/contro_region/PID/{name}.png'
    out_path.parent.mkdir(parents = True, exist_ok = True)

    plt.savefig(out_path)
# ----------------------
@app.command()
def cx():
    '''
    This is used to plot CK and CKstar
    '''
    ana_dir  = Path(os.environ['ANADIR'])
    out_path = ana_dir / 'plots/efficiencies/cx.png'
    cache_dir= Path('/tmp/rx/cache/plots/cx')

    with Cache.cache_root(path=cache_dir),\
        LogStore.level(name = 'rx_selection:selection'               , lvl = 30),\
        LogStore.level(name = 'rx_efficiencies:efficiency_calculator', lvl = 30):
        data = {'Value' : [], 'Error' : [], 'Quantity' : [], 'qsq' : []}
        for project in {Project.rk, Project.rkst}:
            for qsq in {Qsq.low, Qsq.central, Qsq.high}:
                quantity = {Project.rk : '$C_K$', Project.rkst : '$C_{K^*}$'}[project]
                obj      = CXCalculator(project = project, qsq = qsq)
                val, err = obj.calculate()

                data['qsq'].append(qsq)
                data['Value'].append(val)
                data['Error'].append(err)
                data['Quantity'].append(quantity)

    df = pnd.DataFrame(data)

    print(df)
# ----------------------
if __name__ == '__main__':
    app()
