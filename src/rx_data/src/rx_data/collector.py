'''
Module with data structures meant to store partial outputs of tests
'''
import pandas as pnd

# -------------------------------
class Collector:
    '''
    Used to collect outputs of tests
    '''
    data = {}
    # -------------------------------
    @staticmethod
    def add_dataframe(df : pnd.DataFrame, test_name : str):
        '''
        Appends dataframe to existing dataframe for given test
        '''
        data = Collector.data

        if test_name not in data:
            data[test_name] = df
            return

        df_old = data[test_name]

        data[test_name] = pnd.concat([df_old, df], ignore_index=True)
# -------------------------------
