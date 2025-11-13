'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import typer

from dmu.logging.log_store import LogStore

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_fitter:cli')
# ----------------------
@app.command()
def fit_summary(
    log_lvl : int     = typer.Option(20 , '--log_lvl', '-l', help='Logging level'))-> None:
    '''
    This will compare refitted and non-refitted data
    '''
    smr = FitSummary()
    smr.save()

    LogStore.set_level('rx_fitter:refitting', log_lvl)
# ----------------------
@app.command()
def dummy():
    pass
# ----------------------
if __name__ == '__main__':
    app()

