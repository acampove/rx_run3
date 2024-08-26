import data_checks.utilities as utdc

from log_store  import log_store
from atr_mgr    import mgr as amgr

log=log_store.add_logger('data_checks:selector')
#-------------------------------------------------------------------
class selector:
    '''
    Class used to apply selections to ROOT dataframes
    '''
    #-------------------------------------------------------------------
    def __init__(self, rdf=None, cfg_nam=None):
        '''
        rdf          : ROOT dataframe
        cfg_nam (str): Name without extension of toml config file
        '''

        self._rdf     = rdf
        self._cfg_nam = cfg_nam

        self._cfg_dat = None 
        self._atr_mgr = None

        self._initialized=False
    #-------------------------------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._atr_mgr = amgr(self._rdf)
        self._cfg_dat = utdc.load_config(self._cfg_nam)

        self._initialized=True
    #-------------------------------------------------------------------
    def _apply_selection(self):
        self._prescale()

    #-------------------------------------------------------------------
    def _prescale(self):
        if 'prescale' not in self._cfg_dat['selection']:
            log.debug('Not prescaling')
            return

        prs = self._cfg_dat['selection']['prescale']
        log.debug(f'Prescaling by a factor of: {prs}')

        rdf = self._rdf.Define('prs', f'gRandom->Integer({prs})')
        rdf = rdf.Filter('prs==0')

        self._rdf = rdf
    #-------------------------------------------------------------------
    def run(self):
        '''
        Will return ROOT dataframe after selection
        '''
        self._initialize()

        self._apply_selection()

        rdf = self._atr_mgr.add_atr(self._rdf)

        return rdf
#-------------------------------------------------------------------
