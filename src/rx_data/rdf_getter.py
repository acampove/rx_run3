'''
Module holding RDFGetter class
'''

import json
import fnmatch

import yaml
from ROOT                  import RDF, RDataFrame
from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_data:rdf_getter')
# ---------------------------------------------------
class RDFGetter:
    '''
    Class meant to load dataframes with friend trees
    '''
    samples : dict[str,str]
    # ---------------------------------------------------
    def __init__(self, sample : str, trigger : str):
        self._sample   = sample
        self._trigger  = trigger

        self._tmp_path    = '/tmp/config.json'
        self._tree_name   = 'DecayTree'
    # ---------------------------------------------------
    def _get_section(self, yaml_path : str) -> dict:
        d_section = {'trees' : [self._tree_name]}

        with open(yaml_path, encoding='utf-8') as ifile:
            d_data = yaml.safe_load(ifile)

        l_path = []
        nopath = False
        nosamp = True
        for sample in d_data:
            if not fnmatch.fnmatch(sample, self._sample):
                continue

            nosamp = False
            l_path_sample = d_data[sample][self._trigger]
            nsamp = len(l_path_sample)
            if nsamp == 0:
                log.error(f'No paths found for {sample} in {yaml_path}')
                nopath = True
            else:
                log.debug(f'Found {nsamp} paths for {sample} in {yaml_path}')

            l_path += l_path_sample

        if nopath:
            raise ValueError('Samples with paths missing')

        if nosamp:
            raise ValueError(f'Could not find any sample matching {self._sample} in {yaml_path}')

        d_section['files'] = l_path

        return d_section
    # ---------------------------------------------------
    def _get_json_conf(self):
        d_data = {'samples' : {}, 'friends' : {}}

        log.info('Adding samples')
        for sample, yaml_path in RDFGetter.samples.items():
            d_section = self._get_section(yaml_path)

            log.debug(f'    {sample}')
            if sample == 'main':
                d_data['samples'][sample] = d_section
            else:
                d_data['friends'][sample] = d_section

        with open(self._tmp_path, 'w', encoding='utf-8') as ofile:
            json.dump(d_data, ofile)
    # ---------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe
        '''
        self._get_json_conf()

        log.debug(f'Building datarame from {self._tmp_path}')
        rdf = RDF.Experimental.FromSpec(self._tmp_path)

        return rdf
# ---------------------------------------------------
