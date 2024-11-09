'''
Module containing plotter class
'''

import numpy
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore
from dmu.plotting.plotter  import Plotter

log = LogStore.add_logger('dmu:plotting:Plotter1D')
# --------------------------------------------
class Plotter1D(Plotter):
    '''
    Class used to plot columns in ROOT dataframes
    '''
    # --------------------------------------------
    def __init__(self, d_rdf=None, cfg=None):
        '''
        Parameters:

        d_rdf (dict): Dictionary mapping the kind of sample with the ROOT dataframe
        cfg   (dict): Dictionary with configuration, e.g. binning, ranges, etc
        '''

        super().__init__(d_rdf=d_rdf, cfg=cfg)
    #-------------------------------------
    def _get_labels(self, var : str) -> tuple[str,str]:
        if 'labels' not in self._d_cfg['plots'][var]:
            return var, 'Entries'

        xname, yname = self._d_cfg['plots'][var]['labels' ]

        return xname, yname
    #-------------------------------------
    def _plot_var(self, var):
        '''
        Will plot a variable from a dictionary of dataframes
        Parameters
        --------------------
        var   (str)  : name of column
        '''
        # pylint: disable=too-many-locals

        d_cfg = self._d_cfg['plots'][var]

        minx, maxx, bins = d_cfg['binning']
        yscale           = d_cfg['yscale' ] if 'yscale' in d_cfg else 'linear'
        xname, yname     = self._get_labels(var)

        normalized=False
        if 'normalized' in d_cfg:
            normalized = d_cfg['normalized']

        title = ''
        if 'title'      in d_cfg:
            title = d_cfg['title']

        d_data = {}
        for name, rdf in self._d_rdf.items():
            d_data[name] = rdf.AsNumpy([var])[var]

        if maxx <= minx + 1e-5:
            log.info(f'Bounds not set for {var}, will calculated them')
            minx, maxx = self._find_bounds(d_data = d_data, qnt=minx)
            log.info(f'Using bounds [{minx:.3e}, {maxx:.3e}]')
        else:
            log.debug(f'Using bounds [{minx:.3e}, {maxx:.3e}]')

        l_bc_all = []
        d_wgt    = self._get_weights(var)
        for name, arr_val in d_data.items():
            arr_wgt    = d_wgt[name] if d_wgt is not None else None

            self._print_weights(arr_wgt, var, name)
            l_bc, _, _ = plt.hist(arr_val, weights=arr_wgt, bins=bins, range=(minx, maxx), density=normalized, histtype='step', label=name)
            l_bc_all  += numpy.array(l_bc).tolist()

            plt.yscale(yscale)
            plt.xlabel(xname)
            plt.ylabel(yname)

        if yscale == 'linear':
            plt.ylim(bottom=0)

        max_y = max(l_bc_all)
        plt.ylim(top=1.2 * max_y)
        plt.title(title)
    # --------------------------------------------
    def _plot_lines(self, var : str):
        '''
        Will plot vertical lines for some variables

        var (str) : name of variable
        '''
        if var in ['B_const_mass_M', 'B_M']:
            plt.axvline(x=5280, color='r', label=r'$B^+$'   , linestyle=':')
        elif var == 'Jpsi_M':
            plt.axvline(x=3096, color='r', label=r'$J/\psi$', linestyle=':')
    # --------------------------------------------
    def run(self):
        '''
        Will run plotting
        '''

        fig_size = self._get_fig_size()
        for var in self._d_cfg['plots']:
            log.debug(f'Plotting: {var}')
            plt.figure(var, figsize=fig_size)
            self._plot_var(var)
            self._plot_lines(var)
            self._save_plot(var)
# --------------------------------------------
