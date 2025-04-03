'''
Module holding RDFGetter class
'''
import os
import glob
import json
import hashlib
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
    def __init__(self, sample : str, trigger : str, tree : str):
        '''
        Sample: Sample's nickname, e.g. DATA_24_MagDown_24c2
        Trigger: HLT2 trigger, e.g. Hlt2RD_BuToKpEE_MVA
        Tree: E.g. DecayTree or MCDecayTree
        '''
        self._initialize()

        self._sample   = sample
        self._trigger  = trigger

        self._tmp_path    = self._get_tmp_path()
        self._tree_name   = tree
    # ---------------------------------------------------
    def _get_tmp_path(self) -> str:
        samples_str = json.dumps(RDFGetter.samples, sort_keys=True)
        samples_bin = samples_str.encode()
        hsh         = hashlib.sha256(samples_bin)
        hsh         = hsh.hexdigest()
        tmp_path    = f'/tmp/config_{self._sample}_{self._trigger}_{hsh}.json'

        log.debug(f'Using config JSON: {tmp_path}')

        return tmp_path
    # ---------------------------------------------------
    def _initialize(self) -> None:
        '''
        Function will:
        - Find samples, assuming they are in $DATADIR/samples/*.yaml
        - Add them to the samples member of RDFGetter

        If no samples found, will raise FileNotFoundError
        '''
        if hasattr(RDFGetter, 'samples'):
            log.debug('Samples dictionary already found in class, skipping initialization')
            return

        data_dir     = os.environ['DATADIR']
        cfg_wildcard = f'{data_dir}/samples/*.yaml'
        l_config     = glob.glob(cfg_wildcard)
        npath        = len(l_config)
        if npath == 0:
            raise FileNotFoundError(f'No files found in: {cfg_wildcard}')

        d_sample = {}
        log.info('Adding samples, found:')
        for path in l_config:
            file_name   = os.path.basename(path)
            sample_name = file_name.replace('.yaml', '')
            d_sample[sample_name] = path
            log.info(f'    {sample_name}')

        RDFGetter.samples = d_sample
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
            try:
                l_path_sample = d_data[sample][self._trigger]
            except KeyError as exc:
                raise KeyError(f'Cannot access {yaml_path}:{sample}/{self._trigger}') from exc

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
            json.dump(d_data, ofile, indent=4, sort_keys=True)
    # ---------------------------------------------------
    def get_rdf(self) -> RDataFrame:
        '''
        Returns ROOT dataframe
        '''
        self._get_json_conf()

        log.debug(f'Building datarame from {self._tmp_path}')
        rdf = RDF.Experimental.FromSpec(self._tmp_path)
        rdf = rdf.Define('Jpsi_const_mass_M' , 'TMath::Sqrt(TMath::Power(Jpsi_DTF_HEAD_PE, 2) - TMath::Power(Jpsi_DTF_HEAD_PX, 2) - TMath::Power(Jpsi_DTF_HEAD_PY, 2) - TMath::Power(Jpsi_DTF_HEAD_PZ, 2))')

        return rdf
# ---------------------------------------------------
