'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import typer

from dmu.logging.log_store import LogStore
from fitter                import FitSummary

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_fitter:cli')
# ----------------------
@app.command()
def fit_summary(
    name    : str = typer.Option(..., '--name'   , '-n', help='Name of fits, e.g. mid_window'),
    log_lvl : int = typer.Option(20 , '--log_lvl', '-l', help='Logging level'))-> None:
    '''
    This will compare refitted and non-refitted data
    '''
    smr = FitSummary(name = name)
    smr.get_df()

    LogStore.set_level('rx_fitter:fit_summary', log_lvl)
# ----------------------
@app.command()
def dummy():
    pass
# ----------------------
if __name__ == '__main__':
    app()

