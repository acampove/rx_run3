'''
Module holding SampleSplitter class
'''

import pandas as pnd
from ROOT                   import RDataFrame
from dmu.rdataframe         import utilities as ut
from dmu.logging.log_store  import LogStore

log=LogStore.add_logger('rx_misid:splitter')
# --------------------------------
class SampleSplitter:
    '''
    Class meant to split a dataframe into PassFail, FailPass and FailFail samples
    based on a configuration
    '''
    # --------------------------------
    def __init__(self,
                 rdf      : RDataFrame,
                 hadron_id: str,
                 is_bplus : bool,
                 cfg      : dict):
        '''
        rdf : Input dataframe with data to split
        is_bplus: True if the sam ple that will be returned will contain B+ mesons, false for B-
        cfg     : Dictionary with configuration specifying how to split the samples
        '''
        self._b_id     = 521
        self._is_bplus = is_bplus
        self._hadron_id= hadron_id
        self._cfg      = cfg
        self._l_kind   = ['PassFail', 'FailPass', 'FailFail']

        self._rdf      = self._filter_rdf(rdf)
    # --------------------------------
    def _filter_rdf(self, rdf : RDataFrame) -> RDataFrame:
        bid = self._b_id if self._is_bplus else - self._b_id
        rdf = rdf.Filter('block > 0', 'block')
        rdf = rdf.Filter(f'B_ID=={bid}', f'B_ID=={bid}')

        return rdf
    # --------------------------------
    def _get_cuts(self, kind : str) -> tuple[str,str]:
        pass_cut = self._cfg['lepton_tagging']['pass']
        fail_cut = self._cfg['lepton_tagging']['fail']
        hadr_tag = self._cfg['hadron_tagging'][self._hadron_id]

        fail_cut = f'({fail_cut}) && ({hadr_tag})'

        lep_ss   = self._cfg['tracks']['ss']
        lep_os   = self._cfg['tracks']['os']

        if   kind == 'PassFail':
            cut_ss   = pass_cut.replace('LEP_', lep_ss + '_')
            cut_os   = fail_cut.replace('LEP_', lep_os + '_')
        elif kind == 'FailPass':
            cut_ss   = fail_cut.replace('LEP_', lep_ss + '_')
            cut_os   = pass_cut.replace('LEP_', lep_os + '_')
        elif kind == 'FailFail':
            cut_ss   = fail_cut.replace('LEP_', lep_ss + '_')
            cut_os   = fail_cut.replace('LEP_', lep_os + '_')
        else:
            raise ValueError(f'Invalid kind: {kind}')

        log.debug(f'Kind: {kind}')
        log.debug(f'SS cut: {cut_ss}')
        log.debug(f'OS cut: {cut_os}')

        return cut_ss, cut_os
    # --------------------------------
    def _get_branches(self) -> list[str]:
        l_all_branches = [ name.c_str() for name in self._rdf.GetColumnNames() ]

        l_branch = []
        for par in self._cfg['maps']['pars']:
            l_branch += [ name for name in l_all_branches if par in name ]

        return l_branch
    # --------------------------------
    def _rdf_to_df(self, rdf : RDataFrame) -> pnd.DataFrame:
        l_branch = self._cfg['branches']
        l_branch+= self._get_branches()
        data     = rdf.AsNumpy(l_branch)
        df       = pnd.DataFrame(data)

        if len(df) == 0:
            rep      = rdf.Report()
            cutflow  = ut.rdf_report_to_df(rep)
            log.warning('Empty dataset:')
            log.info('\n' + str(cutflow))

        return df
    # --------------------------------
    def get_samples(self) -> pnd.DataFrame:
        '''
        Returns pandas dataframe with data split by:

        PassFail: Pass (SS), Fail (OS)
        FailPass: Fail (SS), Pass (OS)
        FailFail: Both electrons fail the PID cut

        Where:
            - SS means same sign as the B and OS is opposite sign
            - These strings are stored in the column "kind"
        '''
        l_df = []
        for kind in self._l_kind:
            rdf            = self._rdf
            cut_os, cut_ss = self._get_cuts(kind=kind)

            rdf = rdf.Filter(cut_os, f'OS {kind}')
            rdf = rdf.Filter(cut_ss, f'SS {kind}')

            df = self._rdf_to_df(rdf=rdf)
            df['kind'] = kind
            l_df.append(df)

        df_tot = pnd.concat(l_df)

        return df_tot
# --------------------------------
