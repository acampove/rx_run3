'''
Module holding ToyPlotter class
'''
import pandas as pnd
import numpy

from ROOT                  import RDataFrame, RDF # type: ignore
from omegaconf             import DictConfig
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:toy_plotter')
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
        self._rdf = self._rdf_from_df(df=df)
        self._cfg = cfg
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
            l_df.append(df_ref)

        df = pnd.concat(objs=l_df, axis=0)

        py_data = df.to_dict(orient='list')
        np_data = { name : numpy.array(vals, dtype='float') for name, vals in py_data.items() }
        rdf     = RDF.FromNumpy(np_data)

        rdf.Display().Print()

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

        d_name = {
            'Gen'   : f'{name}_gen',
            'Pull'  : f'{name}_pul',
            'Value' : f'{name}_val', 
            'Error' : f'{name}_err'}

        df = df.rename(columns=d_name) 
        # No need of name, it is in the other columns already
        df = df.drop(columns='Parameter') 

        return df
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
# ----------------------
