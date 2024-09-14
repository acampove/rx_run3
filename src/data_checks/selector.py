import pprint
import data_checks.utilities as utdc

from log_store  import log_store
from atr_mgr    import mgr as amgr

log = log_store.add_logger('data_checks:selector')
# -------------------------------------------------------------------
class selector:
    '''
    Class used to apply selections to ROOT dataframes
    '''
    # -------------------------------------------------------------------
    def __init__(self, rdf=None, cfg_nam=None, is_mc=None):
        '''
        rdf          : ROOT dataframe
        cfg_nam (str): Name without extension of toml config file
        is_mc (bool) : MC or real data?
        '''

        self._rdf       = rdf
        self._cfg_nam   = cfg_nam
        self._is_mc     = is_mc

        self._atr_mgr   = None
        self._d_sel     = None
        self._d_rdf     = dict()

        self._initialized = False
    # -------------------------------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        if self._is_mc not in [True, False]:
            log.error(f'Invalid value for is_mc: {self._is_mc}')
            raise ValueError

        self._atr_mgr = amgr(self._rdf)

        log.debug(f'Using config: {self._cfg_nam}')
        cfg_dat       = utdc.load_config(self._cfg_nam)
        self._d_sel   = cfg_dat['selection']
        self._fix_bkgcat()

        self._initialized = True
    # -------------------------------------------------------------------
    def _apply_selection(self):
        '''
        Loop over cuts and apply selection
        Save intermediate dataframes to self._d_rdf
        Save final datafrme to self._rdf
        '''
        rdf = self._rdf
        log.debug(20 * '-')
        log.debug('Applying selection:')
        log.debug(20 * '-')
        for key, cut in self._d_sel['cuts'].items():
            log.debug(f'{"":<4}{key}')
            rdf = rdf.Filter(cut, key)
            self._d_rdf[key] = rdf

        self._rdf = rdf
    # --------------------------------------
    def _fix_bkgcat(self):
        '''
        If data, will set cut to (1).
        If MC, will find BKGCAT branch in dataframe (e.g. Lb_BKGCAT)
        Will rename BKGCAT in cuts dictionary, such that truth matching cut can be applied
        '''

        if 'BKGCAT' not in self._d_sel['cuts']:
            log.debug('Not renaming BKGCAT')

            return

        log.debug('Fixing BKGCAT')
        if not self._is_mc:
            self._d_sel['cuts']['BKGCAT'] = '(1)'
            return

        bkgcat_cut = self._d_sel['cuts']['BKGCAT']
        bkgcat_var = self._get_bkgcat_name()
        bkgcat_cut = bkgcat_cut.replace('BKGCAT', bkgcat_var)

        log.debug(f'Using truth matching cut: {bkgcat_cut}')
        self._d_sel['cuts']['BKGCAT'] = bkgcat_cut
    # --------------------------------------
    def _get_bkgcat_name(self):
        '''
        Will return name of branch in tree, holding the background category for the B meson, i.e.:

        X_BKGCAT
        '''
        v_col  = self._rdf.GetColumnNames()
        l_col  = [ col.c_str() for col in v_col ]
        l_bkg  = [ col         for col in l_col if col.endswith('BKGCAT') ]

        try:
            [name] = [ col for col in l_col if col in ['Lb_BKGCAT', 'B_BKGCAT'] ]
        except ValueError:
            log.error('Could not find one and only one BKGCAT branch for B meson, found:')
            pprint.pprint(l_bkg)
            raise

        log.debug(f'Found background category branch: {name}')

        return name
    # -------------------------------------------------------------------
    def _prescale(self):
        '''
        Will pick up a random subset of entries from the dataframe if 'prescale=factor' found in selection section
        '''

        if 'prescale' not in self._d_sel:
            log.debug('Not prescaling')
            return

        prs = self._d_sel['prescale']
        log.debug(f'Prescaling by a factor of: {prs}')

        rdf = self._rdf.Define('prs', f'gRandom->Integer({prs})')
        rdf = rdf.Filter('prs==0')

        self._rdf = rdf
    # -------------------------------------------------------------------
    def _print_info(self, rdf):
        log_lvl = log.level
        if log_lvl < 20:
            rep = rdf.Report()
            rep.Print()
    # -------------------------------------------------------------------
    def run(self, as_cutflow=False):
        '''
        Will return ROOT dataframe(s)

        Parameters
        -------------------
        as_cutflow (bool): If true will return {cut_name -> rdf} dictionary
        with cuts applied one after the other. If False (default), it will only return
        the dataframe after the full selection
        '''
        self._initialize()
        self._prescale()

        self._apply_selection()

        self._d_rdf = { key : self._atr_mgr.add_atr(rdf) for key, rdf in self._d_rdf.items() }

        self._print_info(self._rdf)

        if as_cutflow:
            return self._d_rdf

        return self._rdf
# -------------------------------------------------------------------
