'''
Module holding FitParameters class
'''

# ----------------------
class FitParameters:
    '''
    Class meant to plot fit parameters
    '''
    # ----------------------
    def __init__(self, name : str, cfg : str):
        '''
        Parameters
        -------------
        name : Fit name, e.g. mid_window
        cfg  : Name of config meant for plots, e.g. fpars
        '''
        self._name = name
        self._cfg  = cfg
    # ----------------------
    def run(self) -> None:
        '''
        Starts plotting
        '''
        pass 
# ----------------------
