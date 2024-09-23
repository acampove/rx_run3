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

        d_cfg = self._d_cfg['plots'][var]

        minx, maxx, bins = d_cfg['binning']
        yscale           = d_cfg['yscale' ]
        [xname, yname]   = d_cfg['labels' ]

        normalized=False
        if 'normalized' in d_cfg:
            normalized = d_cfg['normalized']

        l_bc_all = []
        for name, rdf in self._d_rdf.items():
            arr_mass = rdf.AsNumpy([var])[var]
            l_bc, _, _ = plt.hist(arr_mass, bins=bins, range=(minx, maxx), density=normalized, histtype='step', label=name)
            l_bc_all  += l_bc.tolist()

            plt.yscale(yscale)
            plt.xlabel(xname)
            plt.ylabel(yname)

        if var in ['B_const_mass_M', 'B_M']:
            plt.axvline(x=5280, color='r', label=r'$B^+$'   , linestyle=':')
        elif var == 'Jpsi_M':
            plt.axvline(x=3096, color='r', label=r'$J/\psi$', linestyle=':')

        if yscale == 'linear':
            plt.ylim(bottom=0)

        max_y = max(l_bc_all)
        plt.ylim(top=1.2 * max_y)

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
            log.debug(f'Plotting: {var}')
            self._plot_var(var)
# --------------------------------------------
