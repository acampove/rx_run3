'''
Module containing Plotter2D class
'''

from typing import Union

import numpy
import matplotlib.pyplot as plt

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

        self._rdf   = rdf
        self._d_cfg = cfg

        self._wgt : numpy.ndarray
    # --------------------------------------------
    def _plot_vars(self, varx : str, vary : str) -> None:
        log.info(f'Plotting {varx} vs {vary}')
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
