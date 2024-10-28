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
    def __init__(self, d_rdf : dict, cfg : dict):
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

        rdf = self._define_vars(rdf)
        if 'selection' in self._d_cfg:
            rdf = self._apply_selection(rdf)
            rdf = self._max_ran_entries(rdf)

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

        log.info('Defining extra variables')
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
    def _check_quantile(self, qnt : float):
        '''
        Will check validity of quantile
        '''

        if 0.5 < qnt <= 1.0:
            return

        raise ValueError(f'Invalid quantile: {qnt:.3e}, value needs to be in (0.5, 1.0] interval')
    #-------------------------------------
    def _find_bounds(self, d_data : dict, qnt : float = 0.98):
        '''
        Will take dictionary between kinds of data and numpy array
        Will return tuple with bounds, where 95% of the data is found
        '''
        self._check_quantile(qnt)

        l_max = []
        l_min = []

        for arr_val in d_data.values():
            minv = numpy.quantile(arr_val, 1 - qnt)
            maxv = numpy.quantile(arr_val,     qnt)

            l_max.append(maxv)
            l_min.append(minv)

        minx = min(l_min)
        maxx = max(l_max)

        if minx >= maxx:
            raise ValueError(f'Could not calculate bounds correctly: [{minx:.3e}, {maxx:.3e}]')

        return minx, maxx
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
        for name, arr_val in d_data.items():
            l_bc, _, _ = plt.hist(arr_val, bins=bins, range=(minx, maxx), density=normalized, histtype='step', label=name)
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
    def _print_weights(self, arr_wgt : Union[numpy.ndarray, None], var : str, sample : str) -> None:
        if arr_wgt is None:
            log.debug(f'Not using weights for {sample}:{var}')
            return

        num_wgt = len(arr_wgt)
        sum_wgt = numpy.sum(arr_wgt)

        log.debug(f'Using weights [{num_wgt},{sum_wgt}] for {var}')
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
    def _get_fig_size(self):
        '''
        Will read size list from config dictionary if found
        other wise will return None
        '''
        if 'general' not in self._d_cfg:
            return None

        if 'size' not in self._d_cfg['general']:
            return None

        fig_size = self._d_cfg['general']['size']

        return fig_size
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
