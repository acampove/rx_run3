'''
Module with BkkChecker class
'''

import yaml

# ---------------------------------
class BkkChecker:
    '''
    Class meant to check if samples exist in Bookkeeping using multithreading.
    This is useful with large lists of samples, due to low performance of Dirac
    '''
    # -------------------------
    def __init__(self, path : str):
        '''
        Takes the path to a YAML file with the list of samples
        '''

        with open(path, encoding='utf-8') as ifile:
            self._l_sample = yaml.safe_load(ifile)
    # -------------------------
    def save(self, path : str) -> None:
        '''
        Will save list of found samples to given path
        '''
# ---------------------------------

#for sample in l_sample:
#    sample_id, event_type, mc_path, polarity, conddb_tag, dddb_tag, nu_path, nuval, sim_version, generator = sample 
#
#    bk_query = f'/MC/{sample_id}/Beam6800GeV-{mc_path}-{polarity}-{nu_path}-25ns-{generator}/{sim_version}/HLT2-2024.W31.34/{event_type}/DST'
#
#
