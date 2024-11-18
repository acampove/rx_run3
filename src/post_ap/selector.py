'''
Module containing selector class
'''

import pprint
from typing                import Union
from ROOT                  import RDataFrame

from dmu.rdataframe.atr_mgr import AtrMgr
from dmu.logging.log_store  import LogStore

import post_ap.utilities as utdc

log = LogStore.add_logger('post_ap:selector')
# -------------------------------------------------------------------
class selector:
    '''
    Class used to apply selections to ROOT dataframes
    '''
    # -------------------------------------------------------------------
    def __init__(self, rdf : RDataFrame, cfg_nam : str, is_mc : bool):
        '''
        rdf    : ROOT dataframe
        cfg_nam: Name without extension of toml config file
        is_mc  : MC or real data?
        '''

        self._rdf       = rdf
        self._cfg_nam   = cfg_nam
        self._is_mc     = is_mc

        self._proc      : str
        self._atr_mgr   : AtrMgr
        self._d_sel     : dict
        self._d_rdf     : dict[str,   RDataFrame]

        self._initialized = False
    # -------------------------------------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        if self._is_mc not in [True, False]:
            log.error(f'Invalid value for is_mc: {self._is_mc}')
            raise ValueError

        self._atr_mgr = AtrMgr(self._rdf)

        self._set_process()

        log.debug(f'Using config: {self._cfg_nam}')
        cfg_dat       = utdc.load_config(self._cfg_nam)
        self._d_sel   = cfg_dat['selection']
        self._fix_bkgcat()

        self._initialized = True
    # -------------------------------------------------------------------
    def _set_process(self):
        '''
        Will set the attribute self._proc based on the name of the HLT line in the dataframe
        The attribute is used to apply a selection based on the config files content
        '''
        hlt_line = self._rdf.name
        d_line_proc = {
            'Hlt2RD_BuToKpEE'                          : 'bukee',
            'Hlt2RD_BuToKpEE_MVA'                      : 'bukee',
            # ---
            'Hlt2RD_BuToKpMuMu'                        : 'bukmm',
            'Hlt2RD_BuToKpMuMu_MVA'                    : 'bukmm',
            # ---
            'Hlt2RD_B0ToKpPimMuMu'                     : 'bdkstmm',
            'Hlt2RD_B0ToKpPimMuMu_MVA'                 : 'bdkstmm',
            'Hlt2RD_BdToKstJpsi_KstToKpPim_JpsiToMuMu' : 'bdkstmm',
            # ---
            'Hlt2RD_B0ToKpPimEE'                       : 'bdkstee',
            'Hlt2RD_B0ToKpPimEE_MVA'                   : 'bdkstee',
            'Hlt2RD_BuToKpJpsi_JpsiToEE'               : 'bdkstee',
            'Hlt2RD_BdToKstJpsi_KstToKpPim_JpsiToEE'   : 'bdkstee',
            # ---
            'Hlt2RD_LbToLEE_LL'                        : 'lbpkee',
            'Hlt2RD_LbToLEE_LL_MVA'                    : 'lbpkee',
            'Hlt2RD_LbToPKJpsi_JpsiToEE'               : 'lbpkee',
            # ---
            'Hlt2RD_LbToLMuMu_LL'                      : 'lbpkmm',
            'Hlt2RD_LbToLMuMu_LL_MVA'                  : 'lbpkmm',
            'Hlt2RD_LbToPKJpsi_JpsiToMuMu'             : 'lbpkmm',
        }

        if hlt_line not in d_line_proc:
            log.debug(f'Line not implemented for selection: {hlt_line}')
            return

        proc = d_line_proc[hlt_line]
        log.debug(f'Found process {proc} for line {hlt_line}')

        self._proc = proc
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

        d_cut    = self._d_sel['cuts']
        skip_cut = True
        for key, cut in d_cut.items():
            # Skip selection if this block of cuts does not
            # correspond to current tree
            if key not in ['any', self._proc]:
                continue

            skip_cut = False
            if len(cut) == 0:
                log.debug(f'Empty selection for process: {self._proc}')

            for name, cut_val in cut.items():
                rdf = rdf.Filter(cut_val, f'{name}:{key}')

            self._d_rdf[key] = rdf

        if skip_cut:
            log.info(40 * '-')
            log.warning(f'Process \"{self._proc}\" not found among:')
            for proc in d_cut:
                log.info(f'    \"{proc}\"')
            log.info(40 * '-')

        self._rdf = rdf
    # --------------------------------------
    def _fix_bkgcat(self):
        '''
        If data, will set cut to (1).
        If MC, will find BKGCAT branch in dataframe (e.g. Lb_BKGCAT)
        Will rename BKGCAT in cuts dictionary, such that truth matching cut can be applied
        '''
        if not self._is_mc:
            return

        if 'BKGCAT' not in self._d_sel['cuts']['any']:
            log.debug('Not renaming BKGCAT')
            return

        log.debug('Fixing BKGCAT')
        bkgcat_cut = self._d_sel['cuts']['any']['BKGCAT']
        bkgcat_var = self._get_bkgcat_name()
        bkgcat_cut = bkgcat_cut.replace('BKGCAT', bkgcat_var)

        log.debug(f'Using truth matching cut: {bkgcat_cut}')
        self._d_sel['cuts']['any']['BKGCAT'] = bkgcat_cut
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
    def run(self, as_cutflow=False) -> Union[RDataFrame, dict[str,RDataFrame]]:
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
