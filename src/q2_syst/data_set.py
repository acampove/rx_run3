
#----------------------------------------------
class dset:
    def __init__(self, is_mc=None, trigger=None, dset=None):
        self._is_mc   = is_mc
        self._trigger = trigger 
        self._dset    = dset 

        self._initialized = False
    #----------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        if self._is_mc   not in [True, False]:
            sel.log.error(f'Invalid value for is_mc: {self._is_mc}')
            raise ValueError

        if self._trigger not in ['ETOS', 'GTIS']:
            sel.log.error(f'Invalid value for trigger: {self._trigger}')
            raise ValueError

        if self._dset    not in ['r1', 'r2p1', '2017', '2018']:
            sel.log.error(f'Invalid value for dset: {self._dset}')
            raise ValueError

        self._initialized = True
    #----------------------------------------------
    def get_rdf(self):
        self._initialize()

        return 
#----------------------------------------------

