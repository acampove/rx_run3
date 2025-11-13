'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import typer

from dmu.logging.log_store import LogStore
from rx_common.types       import Project 

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_fitter:cli')
# ----------------------
@app.command()
def fit_summary(
    log_lvl : int     = typer.Option(20 , '--log_lvl', '-l', help='Logging level'),
    project : str     = typer.Option(..., '--project', '-p', help='E.g. rk, rkst')) -> None:
    '''
    This will compare refitted and non-refitted data
    '''
    if project not in [Project.rk, Project.rkst]:
        raise ValueError(f'Invalid project: {project}')

    LogStore.set_level('rx_fitter:refitting', log_lvl)
# ----------------------
@app.command()
def dummy():
    pass
# ----------------------
if __name__ == '__main__':
    app()

