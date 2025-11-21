'''
Module holding FitParameters class
'''

from dmu.generic                    import utilities as gut
from rx_plotter.fit_parameters_conf import FitParametersConf, GraphConf, Info, PlotConf
from fitter                         import ParameterReader
from dmu                            import LogStore

log=LogStore.add_logger('rx_plots:fit_parameters')
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
    def _plot_data(
        self, 
        pcfg : PlotConf,
        gcfg : GraphConf) -> None:
        '''
        This method plots a given graph

        Parameters
        -------------
        pcfg : Plotting configuration for a group of graphs
        gcfg : Plotting configuration for a signle graph
        '''
        info : Info = gcfg.info
        pcfg.xaxis

        ms   = self._rdr(
            block    = 3, 
            brem     = info.brem, 
            trigger  = info.trigger, 
            project  = info.project,
            q2bin    = info.q2bin)

        print(ms)
    # ----------------------
    def run(self) -> None:
        '''
        Starts plotting
        '''
        for plot_name, plot_cfg in self._cfg.root.items():
            log.info(f'Plotting {plot_name}')
            for expr, graph_cfg in plot_cfg.graphs.items():
                log.debug(f'    {expr}')

                self._plot_data(pcfg = plot_cfg, gcfg = graph_cfg)
# ----------------------
