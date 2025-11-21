'''
Module holding FitParameters class
'''
import matplotlib.pyplot as plt

from pathlib                        import Path
from dmu.generic                    import utilities as gut
from rx_plotter.fit_parameters_conf import FitParametersConf, GraphConf, Info, PlotConf
from fitter                         import ParameterReader
from dmu                            import LogStore, Measurement

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
    def _get_values(
        self,
        ms    : Measurement, 
        expr  : str, 
        error : str) -> tuple[float,float]:
        '''
        Parameters
        -------------
        ms: Measurement instance
        expr: String describing y value
        error: String describing error in yvalue

        Returns
        -------------
        Tuple with y value and error 
        '''
        return 1, 1 
    # ----------------------
    def _plot_graph(
        self, 
        expr : str,
        pcfg : PlotConf,
        gcfg : GraphConf) -> None:
        '''
        This method plots a given graph

        Parameters
        -------------
        expr : Expression to plot, i.e. y axis
        pcfg : Plotting configuration for a group of graphs
        gcfg : Plotting configuration for a single graph
        '''
        info : Info = gcfg.info

        data  = info.model_dump()
        name  = pcfg.xaxis.name
        xvals = pcfg.xaxis.values

        yvals = []
        yerrs = []
        for value in xvals:
            data[name] = value
            ms = self._rdr(**data)
            yval, yerr = self._get_values(ms=ms, expr=expr, error=gcfg.error)

            yvals.append(yval)
            yerrs.append(yerr)

        plt.errorbar(x=xvals, y=yvals, yerr=yerrs, label=gcfg.label)
    # ----------------------
    def run(self, out_path : Path) -> None:
        '''
        Runs plotting

        Parameters
        -----------------
        out_path: Directory path where plots will be saved
        '''
        for plot_name, plot_cfg in self._cfg.root.items():
            log.info(f'Plotting {plot_name}')
            plt.figure(num = plot_name, figsize=plot_cfg.size)

            for expr, graph_cfg in plot_cfg.graphs.items():
                log.debug(f'    {expr}')

                self._plot_graph(expr = expr, pcfg = plot_cfg, gcfg = graph_cfg)
                plt.ylim(plot_cfg.yrange)

            plt.savefig(out_path / f'{plot_name}.png')
            plt.close(fig = plot_name)
# ----------------------
