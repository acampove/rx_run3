'''
Module holding ToyPlotter class
'''
import copy
import numpy
import omegaconf
import pandas            as pnd
import seaborn           as sns
import matplotlib.pyplot as plt

from ROOT                    import RDataFrame, RDF # type: ignore
from omegaconf               import DictConfig
from dmu.logging.log_store   import LogStore
from dmu.plotting.plotter_1d import Plotter1D

log=LogStore.add_logger('fitter:toy_plotter')
# ----------------------
class MissingVariableConfiguration(Exception):
    '''
    This exception will be risen if a variable is needed that
    does not have a configuration associated 
    '''
    def __init__(self, message : str) -> None:
        super().__init__(message)
        self.message = message
# ----------------------
class ToyPlotter:
    '''
    Class in charge of:

    - Picking up pandas dataframe with fit results
    - Plotting
        - Pulls
        - Distributions of variables
        - Errors of fitted variables
        - Correlation matrix
    '''
    # ----------------------
    def __init__(self, df : pnd.DataFrame, cfg : DictConfig):
        '''
        Parameters
        -------------
        df : Pandas dataframe with information from toy fits
        cfg: Configuration specifying how to plot
        '''
        self._l_par = df['Parameter'].unique().tolist()
        self._df    = self._preprocess_df(df=df)
        self._d_tex = self._get_latex_names(cfg=cfg)
        self._d_gen = self._get_gen_values(df=df)

        cfg = self._add_pull_config(cfg=cfg)
        cfg = self._add_gen_config(cfg=cfg)
        self._cfg   = cfg
    # ----------------------
    def _get_latex_names(
        self, 
        cfg: DictConfig) -> dict[str,str]:
        '''
        Parameters
        -------------
        cfg: Dictionary with configuration

        Returns
        -------------
        Dictionary mapping parameter names with their latex version
        '''
        d_latex    = {} 
        for par_name in self._l_par:
            try:
                latex_name = cfg.plots[f'{par_name}_val'].labels[0]
            except omegaconf.errors.ConfigKeyError as exc:
                self._print_params()
                raise MissingVariableConfiguration(f'Failed to find configuration for {par_name}') from exc

            d_latex[par_name] = latex_name

        return d_latex 
    # ----------------------
    def _get_gen_values(self, df : pnd.DataFrame) -> dict[str,float]:
        '''
        Parameters
        -------------
        df: DataFrame with parameter information

        Returns
        -------------
        Dictionary mapping parameter name to generated value (used to make toys)
        '''
        fail = False
        d_gen= {}
        for par_name, df_par in df.groupby('Parameter'):
            l_gen_val = df_par['Gen'].unique().tolist()
            if len(l_gen_val) != 1:
                fail = True
                log.error(f'Not one and only one Gen value for: {par_name}')
                continue

            d_gen[par_name] = l_gen_val[0]

        if fail:
            raise ValueError('Multiple generating values')

        return d_gen
    # ----------------------
    def _add_gen_config(self, cfg : DictConfig) -> DictConfig:
        '''
        Parameters
        -------------
        cfg : Config file pased to initializer

        Returns
        -------------
        Config file with `vline` sections added to `_val` plots
        and with the generation value
        '''
        cfg_gen = cfg.generated
        cfg_gen = copy.deepcopy(cfg_gen)

        for par_name in self._d_tex:
            cfg_gen.x = self._d_gen[par_name] 

            cfg.plots[f'{par_name}_val']['vline'] = cfg_gen

        return cfg
    # ----------------------
    def _add_pull_config(self, cfg : DictConfig) -> DictConfig:
        '''
        Parameters
        -------------
        cfg : Plotting configuration dictionary

        Returns
        -------------
        Same dictionary as input with the configuration for the pull plots
        added to the `plots` field.
        '''
        for par_name, latex_name in self._d_tex.items():
            cfg_pul = cfg.pulls
            xlabel  = cfg_pul.labels[0]
            xlabel  = xlabel.replace('VAR', latex_name)

            cfg_pul = copy.deepcopy(cfg_pul)
            cfg_pul.labels[0] = xlabel

            cfg.plots[f'{par_name}_pul'] = cfg_pul

        return cfg
    # ----------------------
    def _preprocess_df(self, df : pnd.DataFrame) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        df: Pandas dataframe from user

        Returns
        -------------
        Dataframe with columns added, renamed, etc
        '''
        l_df = []
        for name, df in df.groupby('Parameter'):
            name = str(name)
            df_ref = self._reformat_df(df=df, name=name)
            df_ref = df_ref.reset_index(drop=True)
            l_df.append(df_ref)

        conv_sr    = df['Converged'].reset_index(drop=True)
        df         = pnd.concat(objs=l_df, axis=1, ignore_index=False)
        df['conv'] = conv_sr

        return df
    # ----------------------
    def _get_rdf(self) -> RDataFrame:
        '''
        Returns
        -------------
        ROOT dataframe meant to be passed to Plotter1d
        '''
        py_data    = self._df.to_dict(orient='list')
        np_data    = { name : numpy.array(vals, dtype='float') for name, vals in py_data.items() }
        rdf        = RDF.FromNumpy(np_data)

        return rdf
    # ----------------------
    def _reformat_df(self, df : pnd.DataFrame, name : str) -> pnd.DataFrame:
        '''
        Parameters
        -------------
        df  : Pandas dataframe with quanities (Value, Error, ...) as columns and parameters in rows
        name: Name of parameter associated to this dataframe

        Returns
        -------------
        Dataframe with columns as quantities associated to one parameter (`a_val`, `a_err`, `a_pul`...)
        '''
        df['Pull'] = (df['Value'] - df['Gen']) / df['Error']
        df['Unc' ] = 100 * df['Error'] / df['Value'].abs()

        d_name = {
            'Gen'   : f'{name}_gen',
            'Pull'  : f'{name}_pul',
            'Unc'   : f'{name}_unc',
            'Value' : f'{name}_val', 
            'Error' : f'{name}_err'}

        df = df.rename(columns=d_name) 
        # No need of name, it is in the other columns already
        df = df.drop(columns='Parameter') 

        return df
    # ----------------------
    def _print_params(self):
        '''
        Prints columns available in input dataframe
        '''
        l_parameter = sorted(self._l_par)

        log.info('Parameters found:')
        for name in l_parameter:
            log.info(name)
    # ----------------------
    def _plot_correlation_matrix(self, plt_path : str) -> None:
        '''
        - Formats the config for the correlation matrix plotting
        - Calculates correlation matrix from input data
        - Plots the correlations through MatrixPlotter
        '''
        cfg_cor = self._cfg.correlation
        cfg_cor = copy.deepcopy(cfg_cor)
        xrot    = cfg_cor.rotation.x
        size    = cfg_cor.size
        title   = cfg_cor.title
        del cfg_cor['size']
        del cfg_cor['title']
        del cfg_cor['rotation']

        d_tex = { f'{key}_val' : val for key, val in self._d_tex.items() }
        data  = rdf.AsNumpy(list(d_tex))
        df    = pnd.DataFrame(data)
        df    = df.rename(columns=d_tex)
        corr  = df.corr()
        mask  = numpy.triu(numpy.ones_like(corr, dtype=bool), k=1)

        plt.figure(figsize=size)
        sns.heatmap(corr, mask=mask, **cfg_cor)
        plt.title(title)
        plt.xticks(rotation=xrot)
        plt.savefig(plt_path)
        plt.close()
    # ----------------------
    def plot(self) -> None:
        '''
        Parameters
        -------------
        none

        Returns
        -------------
        none
        '''
        try:
            ptr = Plotter1D(d_rdf={'Toys' : self._rdf}, cfg=self._cfg)
            ptr.run()
        except ValueError as exc:
            self._print_params()
            raise MissingVariableConfiguration('Cannot plot variable, one of the variables was likely missing') from exc

        plt_path = f'{self._cfg.saving.plt_dir}/correlations.png'
        self._plot_correlation_matrix(plt_path=plt_path)
# ----------------------
