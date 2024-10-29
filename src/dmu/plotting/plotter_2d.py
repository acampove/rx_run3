'''
Module containing Plotter2D class
'''

import hist
import numpy
import mplhep
import matplotlib.pyplot as plt

from hist                  import Hist
from ROOT                  import RDataFrame
from dmu.logging.log_store import LogStore
from dmu.plotting.plotter  import Plotter

log = LogStore.add_logger('dmu:plotting:Plotter2D')
# --------------------------------------------
class Plotter2D(Plotter):
    '''
    Class used to plot columns in ROOT dataframes
    '''
    # --------------------------------------------
    def __init__(self, rdf=None, cfg=None):
        '''
        Parameters:

        d_rdf (dict): Dictionary mapping the kind of sample with the ROOT dataframe
        cfg   (dict): Dictionary with configuration, e.g. binning, ranges, etc
        '''

        if not isinstance(cfg, dict):
            raise ValueError('Config dictionary not passed')

        if not isinstance(rdf, RDataFrame):
            raise ValueError('Dataframe dictionary not passed')

        self._rdf   : RDataFrame = rdf
        self._d_cfg : dict       = cfg

        self._wgt : numpy.ndarray
    # --------------------------------------------
    def _get_axis(self, var : str):
        [minx, maxx, nbins] = self._d_cfg['axes'][var]['binning']
        label               = self._d_cfg['axes'][var][  'label']

        axis = hist.axis.Regular(nbins, minx, maxx, name=label, label=label)

        return axis
    # --------------------------------------------
    def _get_data(self, varx : str, vary : str) -> tuple[numpy.ndarray, numpy.ndarray]:
        d_data = self._rdf.AsNumpy([varx, vary])
        arr_x  = d_data[varx]
        arr_y  = d_data[vary]

        return arr_x, arr_y
    # --------------------------------------------
    def _plot_vars(self, varx : str, vary : str) -> None:
        log.info(f'Plotting {varx} vs {vary}')

        ax_x         = self._get_axis(varx)
        ax_y         = self._get_axis(vary)
        arr_x, arr_y = self._get_data(varx, vary)

        hst = Hist(ax_x, ax_y)
        hst.fill(arr_x, arr_y)

        mplhep.hist2dplot(hst)
    # --------------------------------------------
    def run(self):
        '''
        Will run plotting
        '''

        fig_size = self._get_fig_size()
        for [varx, vary] in self._d_cfg['plots_2d']:
            plot_name = f'{varx}_{vary}'
            plt.figure(plot_name, figsize=fig_size)
            self._plot_vars(varx, vary)
            self._save_plot(plot_name)
# --------------------------------------------
