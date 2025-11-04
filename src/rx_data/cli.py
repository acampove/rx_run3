'''
This is the entry point for all the scripts in this project
Eventually all the non-library functionality should be accessed
through this place
'''

import typer

from dmu.logging.log_store import LogStore
from rx_common.types       import Project 
from rx_data.samples       import SamplesPrinter

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_data:cli')
# ----------------------
@app.command()
def show_samples_by_block(project : Project = typer.Option(..., '--project', '-p')) -> None:
    '''
    This will print a table with the samples available for each block
    '''
    printer = SamplesPrinter(project=project)
    printer.print_by_block()
# ----------------------
@app.command()
def dummy():
    pass
# ----------------------
if __name__ == '__main__':
    app()
