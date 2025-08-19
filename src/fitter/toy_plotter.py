'''
Module holding ToyPlotter class
'''
import copy
import pandas as pnd
import numpy
import omegaconf

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
        self._rdf   = self._rdf_from_df(df=df)
        self._cfg   = self._add_pull_config(cfg=cfg)
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
        for par_name in self._l_par:
            cfg_pul    = cfg.pulls
            xlabel     = cfg_pul.labels[0]
            try:
                latex_name = cfg.plots[f'{par_name}_val'].labels[0]
            except omegaconf.errors.ConfigKeyError as exc:
                self._print_params()
                raise MissingVariableConfiguration(f'Failed to find configuration for {par_name}') from exc

            xlabel     = xlabel.replace('VAR', latex_name)

            cfg_pul = copy.deepcopy(cfg_pul)
            cfg_pul.labels[0] = xlabel

            cfg.plots[f'{par_name}_pul'] = cfg_pul

        return cfg
    # ----------------------
    def _rdf_from_df(self, df : pnd.DataFrame) -> RDataFrame:
        '''
        Parameters
        -------------
        df : Pandas dataframe produced by ToyMaker

        Returns
        -------------
        ROOT dataframe meant to be passed to Plotter1d
        '''
        l_df = []
        for name, df in df.groupby('Parameter'):
            name = str(name)
            df_ref = self._reformat_df(df=df, name=name)
            df_ref = df_ref.reset_index(drop=True)
            l_df.append(df_ref)

        df      = pnd.concat(objs=l_df, axis=1, ignore_index=False)
        py_data = df.to_dict(orient='list')
        np_data = { name : numpy.array(vals, dtype='float') for name, vals in py_data.items() }
        rdf     = RDF.FromNumpy(np_data)

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
        df['Unc' ] = 100 * df['Error'] / df['Value']

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
        s_parameter = set(self._l_par)
        l_parameter = sorted(list(s_parameter))

        log.info('Parameters found:')
        for name in l_parameter:
            log.info(name)
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
# ----------------------
