'''
Module containing plotter class
'''

import os
import math

import numpy
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

        if not isinstance(  cfg, dict):
            raise ValueError('Config dictionary not passed')

        if not isinstance(d_rdf, dict):
            raise ValueError('Dataframe dictionary not passed')

        self._d_cfg = cfg
        self._d_rdf = { name : self._preprocess_rdf(rdf) for name, rdf in d_rdf.items()}
    #-------------------------------------
    def _preprocess_rdf(self, rdf):
        '''
        rdf (RDataFrame): ROOT dataframe

        returns preprocessed dataframe
        '''
        if 'selection' in self._d_cfg:
            rdf = self._apply_selection(rdf)
            rdf = self._max_ran_entries(rdf)

        rdf = self._define_vars(rdf)

        return rdf
    #-------------------------------------
    def _define_vars(self, rdf):
        '''
        Will define extra columns in dataframe and return updated dataframe
        '''

        if 'definitions' not in self._d_cfg:
            log.debug('No definitions section found, returning same RDF')
            return rdf

        d_def = self._d_cfg['definitions']

        log.info('Defining extera variables')
        for name, expr in d_def.items():
            log.debug(f'{name:<30}{expr:<150}')
            rdf = rdf.Define(name, expr)

        return rdf
    #-------------------------------------
    def _apply_selection(self, rdf):
        '''
        Will take dataframe, apply selection and return dataframe
        '''
        if 'cuts' not in self._d_cfg['selection']:
            log.debug('Cuts not found in selection section, not applying any cuts')
            return rdf

        d_cut = self._d_cfg['selection']['cuts']

        log.info('Applying cuts')
        for name, cut in d_cut.items():
            log.debug(f'{name:<50}{cut:<150}')
            rdf = rdf.Filter(cut, name)

        return rdf
    #-------------------------------------
    def _max_ran_entries(self, rdf):
        '''
        Will take dataframe and randomly drop events
        '''

        if 'max_ran_entries' not in self._d_cfg['selection']:
            log.debug('Cuts not found in selection section, not applying any cuts')
            return rdf

        tot_entries = rdf.Count().GetValue()
        max_entries = self._d_cfg['selection']['max_ran_entries']

        if tot_entries < max_entries:
            log.debug(f'Not dropping dandom entries: {tot_entries} < {max_entries}')
            return rdf

        prescale = math.floor(tot_entries / max_entries)
        if prescale < 2:
            log.debug(f'Not dropping random entries, prescale is below 2: {tot_entries}/{max_entries}')
            return rdf

        rdf = rdf.Filter(f'rdfentry_ % {prescale} == 0', 'max_ran_entries')

        fnl_entries = rdf.Count().GetValue()

        log.info(f'Dropped entries randomly: {tot_entries} -> {fnl_entries}')

        return rdf
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
        yscale           = d_cfg['yscale' ]
        [xname, yname]   = d_cfg['labels' ]

        normalized=False
        if 'normalized' in d_cfg:
            normalized = d_cfg['normalized']

        l_bc_all = []
        for name, rdf in self._d_rdf.items():
            arr_val    = rdf.AsNumpy([var])[var]
            l_bc, _, _ = plt.hist(arr_val, bins=bins, range=(minx, maxx), density=normalized, histtype='step', label=name)
            l_bc_all  += numpy.array(l_bc).tolist()

            plt.yscale(yscale)
            plt.xlabel(xname)
            plt.ylabel(yname)

        if yscale == 'linear':
            plt.ylim(bottom=0)

        max_y = max(l_bc_all)
        plt.ylim(top=1.2 * max_y)
    # --------------------------------------------
    def _save_plot(self, var):
        '''
        Will save to PNG:

        var (str) : Name of variable, needed for plot name
        '''
        plt.legend()

        plt_dir = self._d_cfg['saving']['plt_dir']
        os.makedirs(plt_dir, exist_ok=True)

        plot_path = f'{plt_dir}/{var}.png'
        log.info(f'Saving to: {plot_path}')
        plt.tight_layout()
        plt.savefig(plot_path)
        plt.close(var)
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

        for var in self._d_cfg['plots']:
            log.debug(f'Plotting: {var}')
            plt.figure(var)
            self._plot_var(var)
            self._plot_lines(var)
            self._save_plot(var)
# --------------------------------------------
