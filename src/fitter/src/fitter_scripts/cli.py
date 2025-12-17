'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import typer
import matplotlib.pyplot as plt

from dmu      import LogStore
from pathlib  import Path
from fitter   import FitSummary

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_fitter:cli')
# ----------------------
@app.command()
def fit_summary(
    name    : str = typer.Option(..., '--name'   , '-n', help='Name of fits, e.g. mid_window'),
    log_lvl : int = typer.Option(20 , '--log_lvl', '-l', help='Logging level'))-> None:
    '''
    This will create a fit summary for a given fit
    '''
    smr = FitSummary(name = name)
    smr.get_df()

    LogStore.set_level('rx_fitter:fit_summary', log_lvl)
# ----------------------
@app.command()
def make_dummy_plot(
    text : str  = typer.Option('Placeholder', '--text', '-t', help='Text that will go in plot'),
    path : Path = typer.Option(...          , '--path', '-p', help='Path to PNG file')) -> None:
    '''
    This will check if a plot exists and if not, will create a placeholder
    '''

    if path.exists():
        log.info(f'Path found: {path}')
        return

    log.info(f'Path not found, making it: {path}')
    plt.figure(figsize=(15, 10))
    plt.text(0.5, 0.5, text, 
             fontsize=72, 
             color='red', 
             ha='center', 
             va='center',
             weight='bold')
    plt.axis('off')
    plt.savefig(path)
    plt.close('all')
# ----------------------
if __name__ == '__main__':
    app()

