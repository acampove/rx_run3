'''
Module containing plotter class
'''
# pylint: disable=too-many-positional-arguments, too-many-arguments

import cppyy
from hist import Hist

import numpy
import matplotlib.pyplot as plt

from dmu.logging.log_store import LogStore
from dmu.plotting.plotter  import Plotter
from dmu.plotting.fwhm     import FWHM

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
    def _is_normalized(self, var : str) -> bool:
        d_cfg     = self._d_cfg['plots'][var]
        normalized=False
        if 'normalized' in d_cfg:
            normalized = d_cfg['normalized']

        return normalized
    #-------------------------------------
    def _get_binning(self, var : str, d_data : dict[str, numpy.ndarray]) -> tuple[float, float, int]:
        d_cfg  = self._d_cfg['plots'][var]
        minx, maxx, bins = d_cfg['binning']
        if maxx <= minx + 1e-5:
            log.info(f'Bounds not set for {var}, will calculated them')
            minx, maxx = self._find_bounds(d_data = d_data, qnt=minx)
            log.info(f'Using bounds [{minx:.3e}, {maxx:.3e}]')
        else:
            log.debug(f'Using bounds [{minx:.3e}, {maxx:.3e}]')

        return minx, maxx, bins
    #-------------------------------------
    def _run_plugins(
            self,
            arr_val : numpy.ndarray,
            arr_wgt : numpy.ndarray,
            hst     : Hist,
            name    : str,
            varname : str) -> None:

        if 'plugin' not in self._d_cfg:
            log.debug('No plugins found')
            return

        if 'fwhm' in self._d_cfg['plugin']:
            if varname not in self._d_cfg['plugin']['fwhm']:
                log.debug(f'No FWHM plugin found for variable {varname}')
                return

            log.debug(f'FWHM plugin found for variable {varname}')
            cfg = self._d_cfg['plugin']['fwhm'][varname]
            self._run_fwhm(
                    arr_val = arr_val,
                    arr_wgt = arr_wgt,
                    hst     = hst,
                    name    = name,
                    varname = varname,
                    cfg     = cfg)

        if 'stats' in self._d_cfg['plugin']:
            if varname not in self._d_cfg['plugin']['stats']:
                log.debug(f'No stats plugin found for variable {varname}')
                return

            log.debug(f'stats plugin found for variable {varname}')
            cfg = self._d_cfg['plugin']['stats'][varname]
            self._run_stats(
                    arr_val = arr_val,
                    arr_wgt = arr_wgt,
                    name    = name,
                    varname = varname,
                    cfg     = cfg)
    #-------------------------------------
    def _run_stats(
            self,
            arr_val : numpy.ndarray,
            arr_wgt : numpy.ndarray,
            varname : str,
            name    : str,
            cfg     : dict[str:str]) -> None:

        this_title = ''
        data       = {}
        if 'sum' in cfg:
            form = cfg['sum']
            sumv = numpy.sum(arr_wgt)
            this_title += form.format(sumv) + '; '
            data['sum'] = sumv

        if 'mean' in cfg:
            form = cfg['mean']
            mean = numpy.average(arr_val, weights=arr_wgt)
            this_title += form.format(mean) + '; '
            data['mean'] = mean

        if 'rms'  in cfg:
            form = cfg['rms']
            mean = numpy.average(arr_val, weights=arr_wgt)
            rms  = numpy.sqrt(numpy.average((arr_val - mean) ** 2, weights=arr_wgt))
            this_title += form.format(rms ) + '; '
            data['rms'] = rms

        self._data_to_json(data = data, name = f'stats_{varname}_{name}')

        self._title+= f'\n{name}: {this_title}'
    #-------------------------------------
    def _run_fwhm(
            self,
            arr_val : numpy.ndarray,
            arr_wgt : numpy.ndarray,
            hst     : Hist,
            varname : str,
            name    : str,
            cfg     : dict) -> None:

        arr_bin_cnt = hst.values()
        maxy = numpy.max(arr_bin_cnt)
        obj  = FWHM(cfg=cfg, val=arr_val, wgt=arr_wgt, maxy=maxy)
        fwhm = obj.run()

        form        = cfg['format']
        this_title  = form.format(fwhm)
        data        = {}

        if 'add_std' in cfg and cfg['add_std']:
            mu         = numpy.average(arr_val            , weights=arr_wgt)
            var        = numpy.average((arr_val - mu) ** 2, weights=arr_wgt)
            std        = numpy.sqrt(var)
            form       = form.replace('FWHM', 'STD')
            this_title+= '; ' + form.format(std)
            data       = {'mu' : mu, 'std' : std, 'fwhm' : fwhm}

        self._data_to_json(data = data, name = f'fwhm_{varname}_{name}')

        self._title+= f'\n{name}: {this_title}'
    #-------------------------------------
    def _plot_var(self, var : str) -> float:
        '''
        Will plot a variable from a dictionary of dataframes
        Parameters
        --------------------
        var   (str)  : name of column

        Return
        --------------------
        Largest bin content among all bins and among all histograms plotted
        '''
        # pylint: disable=too-many-locals

        d_data = {}
        for name, rdf in self._d_rdf.items():
            try:
                log.debug(f'Plotting: {var}/{name}')
                d_data[name] = rdf.AsNumpy([var])[var]
            except cppyy.gbl.std.runtime_error as exc:
                raise ValueError(f'Cannot find variable {var} in category {name}') from exc

        minx, maxx, bins = self._get_binning(var, d_data)
        d_wgt            = self._get_weights(var)

        l_bc_all = []
        for name, arr_val in d_data.items():
            label        = self._label_from_name(name)
            arr_wgt      = d_wgt[name] if d_wgt is not None else numpy.ones_like(arr_val)
            arr_wgt      = self._normalize_weights(arr_wgt, var)
            hst          = Hist.new.Reg(bins=bins, start=minx, stop=maxx, name='x').Weight()
            hst.fill(x=arr_val, weight=arr_wgt)
            self._run_plugins(arr_val, arr_wgt, hst, name, var)
            style = self._get_style_config(var=var, label=label)

            log.debug(f'Style: {style}')
            hst.plot(**style)

            l_bc_all    += hst.values().tolist()

        max_y = max(l_bc_all)

        return max_y
    # --------------------------------------------
    def _get_style_config(self, var : str, label : str) -> dict[str,str]:
        style = {
            'label'     : label, 
            'histtype'  : 'errorbar', 
            'linestyle' : 'none'}

        if 'styling' not in self._d_cfg['plots'][var]:
            log.debug(f'Styling not specified for {var}')
            return style

        if label     not in self._d_cfg['plots'][var]['styling']:
            log.debug(f'Styling not specified for {var}/{label}')
            return style

        custom_style = self._d_cfg['plots'][var]['styling'][label]
        style.update(custom_style)
        log.debug(f'Using custom styling for {var}/{label}')

        return style
    # --------------------------------------------
    def _label_from_name(self, name : str) -> str:
        if 'stats' not in self._d_cfg:
            return name

        d_stat = self._d_cfg['stats']
        if 'sumw' not in d_stat:
            return name

        form = d_stat['sumw']

        arr_wgt  = self._d_wgt[name]
        arr_wgt  = numpy.nan_to_num(arr_wgt, nan=0.0)
        sumw     = numpy.sum(arr_wgt)
        nentries = form.format(sumw)

        return f'{name:<15}{nentries:<10}'
    # --------------------------------------------
    def _normalize_weights(self, arr_wgt : numpy.ndarray, var : str) -> numpy.ndarray:
        cfg_var = self._d_cfg['plots'][var]
        if 'normalized' not in cfg_var:
            log.debug(f'Not normalizing for variable: {var}')
            return arr_wgt

        if not cfg_var['normalized']:
            log.debug(f'Not normalizing for variable: {var}')
            return arr_wgt

        log.debug(f'Normalizing for variable: {var}')
        total   = numpy.sum(arr_wgt)
        arr_wgt = arr_wgt / total

        return arr_wgt
    # --------------------------------------------
    def _style_plot(self, var : str, max_y : float) -> None:
        d_cfg  = self._d_cfg['plots'][var]
        yscale = d_cfg['yscale' ] if 'yscale' in d_cfg else 'linear'

        xname, yname = self._get_labels(var)
        plt.xlabel(xname)
        plt.ylabel(yname)
        plt.yscale(yscale)
        if yscale == 'linear':
            plt.ylim(bottom=0)

        title = self._title
        if 'title'      in d_cfg:
            this_title = d_cfg['title']
            title += f'\n {this_title}'

        title = title.lstrip('\n')

        plt.ylim(top=1.2 * max_y)
        plt.legend()
        plt.title(title)
    # --------------------------------------------
    def _plot_lines(self, var : str) -> None:
        '''
        Will plot vertical lines for some variables

        var (str) : name of variable
        '''
        var_cfg = self._d_cfg['plots'][var]
        if 'vline' in var_cfg:
            line_cfg = var_cfg['vline']
            plt.axvline(**line_cfg)
    # --------------------------------------------
    def run(self):
        '''
        Will run plotting
        '''

        fig_size = self._get_fig_size()
        for var in self._d_cfg['plots']:
            self._title = ''
            plt.figure(var, figsize=fig_size)
            max_y = self._plot_var(var)
            self._style_plot(var, max_y)
            self._plot_lines(var)
            self._save_plot(var)
# --------------------------------------------
