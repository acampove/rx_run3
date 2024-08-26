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

        self._rdf       = rdf
        self._cfg_nam   = cfg_nam
        self._l_branch  = rdf.l_branch
        self._line_name = rdf.name

        self._cfg_dat   = None 
        self._atr_mgr   = None

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
        self._apply_prescale()
        self._apply_mass_cut()
        self._apply_pid()
    #--------------------------------------
    def _apply_pid(self):
        '''
        Will apply PID and set self._rdf as filtered one
        '''
        if 'pid' not in self._cfg_dat['selection']['cuts']:
            return

        log.debug('Applying PID cuts')

        rdf = self._rdf
        if 'EE'         in self._line_name:
            rdf = rdf.Filter('L1_PID_E  > 2.0', 'lep_1_pid_e')
            rdf = rdf.Filter('L2_PID_E  > 2.0', 'lep_1_pid_e')

        if 'K_PROBNN_K' in self._l_branch:
            rdf = rdf.Filter('K_PROBNN_K> 0.1', 'had_probn_k')

        self._rdf = rdf
    #--------------------------------------
    def _apply_mass_cut(self):
        if 'mass' not in self._cfg_dat['selection']['cuts']:
            return

        log.debug('Applying mass cuts')

        rdf = self._rdf
        if ('Lb_M' in self._l_branch) and ('EE'   in self._line_name):
            rdf = rdf.Filter('Lb_M > 4500', 'Lb mass dn')
            rdf = rdf.Filter('Lb_M < 6000', 'Lb mass up')

        if ('Lb_M' in self._l_branch) and ('MuMu' in self._line_name):
            rdf = rdf.Filter('Lb_M > 5000', 'Lb mass dn')
            rdf = rdf.Filter('Lb_M < 6000', 'Lb mass up')

        if ('B_M'  in self._l_branch) and ('EE'   in self._line_name):
            rdf = rdf.Filter('B_M  > 4500', 'mass_dn')
            rdf = rdf.Filter('B_M  < 6000', 'mass_up')

        if ('B_M'  in self._l_branch) and ('MuMu' in self._line_name):
            rdf = rdf.Filter('B_M  > 5000', 'mass_dn')
            rdf = rdf.Filter('B_M  < 6000', 'mass_up')

        if 'Kst_M' in self._l_branch:
            rdf = rdf.Filter('Kst_M >  700', 'Kst mass dn')
            rdf = rdf.Filter('Kst_M < 1100', 'Kst mass up')

        self._rdf = rdf
    #-------------------------------------------------------------------
    def _apply_prescale(self):
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

