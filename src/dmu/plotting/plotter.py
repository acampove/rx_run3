'''
Module containing plotter class
'''

import os

import matplotlib.pyplot as plt

from log_store import log_store

log = log_store.add_logger('dmu:plotting:Plotter')
# --------------------------------------------
class Plotter:
    '''
    Class used to plot columns in ROOT dataframes
    '''
    # --------------------------------------------
    def __init__(self, d_rdf : dict | None = None, cfg : dict | None = None):
        '''
        Parameters:

        d_rdf (dict): Dictionary mapping the kind of sample with the ROOT dataframe
        cfg   (dict): Dictionary with configuration, e.g. binning, ranges, etc
        '''

        self._d_rdf = d_rdf if d_rdf is not None else {}
        self._d_cfg = cfg   if cfg   is not None else {}
    #-------------------------------------
    def _plot_var(self, var):
        '''
        Will plot a variable from a dictionary of dataframes
        Parameters
        --------------------
        var   (str)  : name of column
        '''
        plt.figure(var)

        plt_dir = self._d_cfg['saving']['plt_dir']
        os.makedirs(plt_dir, exist_ok=True)

        for name, rdf in self._d_rdf.items():
            minx, maxx, bins = self._d_cfg['plots'][var]['binning']
            yscale           = self._d_cfg['plots'][var]['yscale' ]
            [xname, yname]   = self._d_cfg['plots'][var]['labels' ]

            arr_mass = rdf.AsNumpy([var])[var]
            plt.hist(arr_mass, bins=bins, range=(minx, maxx), histtype='step', label=name)
            plt.yscale(yscale)
            plt.xlabel(xname)
            plt.ylabel(yname)

        if var in ['B_const_mass_M', 'B_M']:
            plt.axvline(x=5280, color='r', label=r'$B^+$'   , linestyle=':')
        elif var == 'Jpsi_M':
            plt.axvline(x=3096, color='r', label=r'$J/\psi$', linestyle=':')

        plt.legend()

        plot_path = f'{plt_dir}/{var}.png'
        log.info(f'Saving to: {plot_path}')
        plt.tight_layout()
        plt.savefig(plot_path)
        plt.close(var)
    # --------------------------------------------
    def run(self):
        '''
        Will run plotting
        '''
        for var in self._d_cfg['plots']:
            log.info(f'Plotting: {var}')
            self._plot_var(var)
# --------------------------------------------
