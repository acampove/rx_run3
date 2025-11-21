'''
Module holding FitParameters class
'''

from dmu.generic                    import utilities as gut
from rx_plotter.fit_parameters_conf import FitParametersConf

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
        self._cfg  = self._load_config(name = cfg) 
    # ----------------------
    def _load_config(self, name : str) -> FitParametersConf:
        '''
        Parameters
        -------------
        name: Name of config, e.g. fpars

        Returns
        -------------
        Class holding configuration
        '''
        data = gut.load_data(package='rx_plotter_data', fpath = f'fits/{name}.yaml')

        return FitParametersConf(**data)
    # ----------------------
    def run(self) -> None:
        '''
        Starts plotting
        '''
        pass 
# ----------------------
