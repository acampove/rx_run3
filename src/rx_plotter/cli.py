'''
This script is meant to be the entry point for all the functionalities
provided by this project
'''

import typer
from dmu.logging.log_store import LogStore
from rx_common.types       import Channel, Qsq
from rx_plotter.refitting  import plot as refitting_plot

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_plots:cli')
# ----------------------
@app.command()
def refitting(
    project : str     = typer.Option(..., '--project', '-p', help='E.g. rk, rkst'),
    qsq     : Qsq     = typer.Option(..., '--qsq'    , '-q', help='q2 bin'),
    channel : Channel = typer.Option(..., '--channel', '-c')
) -> None:
    '''
    This will compare refitted and non-refitted data
    '''
    if project not in ['rk', 'rkst']:
        raise ValueError(f'Invalid project: {project}')

    refitting_plot(project=project, qsq=qsq, channel=channel)
# ----------------------
if __name__ == '__main__':
    app()
