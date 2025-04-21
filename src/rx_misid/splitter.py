'''
Module holding SampleSplitter class
'''

import pandas as pnd
from ROOT                   import RDataFrame
from dmu.stats.wdata        import Wdata
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
        self._is_bplus = is_bplus
        self._hadron_id= hadron_id
        self._cfg      = cfg
        self._l_kind   = ['PassFail', 'FailPass', 'FailFail']

        self._rdf      = self._filter_rdf(rdf)
    # --------------------------------
    def _filter_rdf(self, rdf : RDataFrame) -> RDataFrame:
        bid = 312 if self._is_bplus else -312
        rdf = rdf.Filter(f'B_ID=={bid}')

        return rdf
    # --------------------------------
    def _get_cuts(self, kind : str) -> tuple[str,str]:
        pass_cut = self._cfg['lepton_tagging']['pass']
        fail_cut = self._cfg['lepton_tagging']['fail']
        hadr_tag = self._cfg['hadron_tagging'][self._hadron_id]

        pass_cut = f'({pass_cut}) && ({hadr_tag})'
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
    def _rdf_to_wdata(self, rdf : RDataFrame) -> Wdata:
        l_branch = self._get_branches()
        obs_name = self._cfg['observable']
        data     = rdf.AsNumpy(l_branch + [obs_name])

        df       = pnd.DataFrame(data)
        wdata    = Wdata(data=data[obs_name], extra_columns=df)

        return wdata
    # --------------------------------
    def get_samples(self) -> dict[str,Wdata]:
        '''
        Returns dictionary wth the identifier of the split sample as the key, values among:

        PassFail
        FailPass
        FailFail

        and the weighted dataset
        '''
        d_sample = {}

        rdf = self._rdf
        for kind in self._l_kind:
            cut_os, cut_ss = self._get_cuts(kind=kind)

            rdf = self._rdf.Filter(cut_os, f'OS {kind}')
            rdf = self._rdf.Filter(cut_ss, f'SS {kind}')

            d_sample[kind] = self._rdf_to_wdata(rdf=rdf)

        return d_sample
# --------------------------------
