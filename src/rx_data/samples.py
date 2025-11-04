'''
This module is meant to hold SamplesPrinter
'''

#    ana_dir = Path(os.environ['ANADIR'])
#    ana_dir / f'Data/{project}/main/v11'

from rx_common.types import Project

# ----------------------
class SamplesPrinter:
    '''
    This class is meant to print data and MC samples by block
    '''
    # ----------------------
    def __init__(self, project : Project) -> None:
        '''
        Parameters
        -------------
        project: Project, e.g. rk, rkst, rk_no_refit
        '''
        self._project = project
    # ----------------------
    def print_by_block(self) -> None:
        '''
        Prints a table with block column and samples as the rows
        '''
# ----------------------

