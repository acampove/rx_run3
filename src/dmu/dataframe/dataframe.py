'''
Class re-implementing dataframe, meant to be a thin layer on top of polars
'''

import polars as pl

# ------------------------------------------
class DataFrame(pl.DataFrame):
    '''
    Class reimplementing dataframes from polars
    '''
    # ------------------------------------------
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
    # ------------------------------------------
    def define(self, name  : str, expr : str):
        '''
        Function will define new column in dataframe

        name (str): Name of new column
        expr (str): Expression depending on existing columns
        '''

        for col in self.columns:
            expr = expr.replace(col,  f'pl.col("{col}")')

        self = self.with_columns(eval(expr).alias(name))

        return self
# ------------------------------------------
