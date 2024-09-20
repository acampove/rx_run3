'''
Module containing plotter class
'''

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

        self._d_rdf = d_rdf
        self._d_cfg = cfg
    # --------------------------------------------
    def run(self):
        '''
        Will run plotting
        '''

# --------------------------------------------
