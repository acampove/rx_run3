'''
Module holding FitParameters class
'''

from dmu.generic                    import utilities as gut
from rx_plotter.fit_parameters_conf import FitParametersConf
from fitter                         import ParameterReader

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
        self._rdr  = ParameterReader(name = name)
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
        data : dict = gut.load_data(package='rx_plotter_data', fpath = f'fits/{name}.yaml')
        data.pop('generic_info')

        return FitParametersConf(**data)
    # ----------------------
    def run(self) -> None:
        '''
        Starts plotting
        '''
        for plot_name, plot_cfg in self._cfg.root.items():
            for expr, graph_cfg in plot_cfg.graphs.items():
                info = graph_cfg.info

                ms = self._rdr(
                    block    = 3, 
                    brem     = info.brem, 
                    trigger  = info.trigger, 
                    project  = info.project,
                    q2bin    = info.q2bin)

                print(plot_name)
                print(expr)
                print(ms)
# ----------------------
