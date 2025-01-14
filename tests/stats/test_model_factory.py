'''
Module containing tests for ZModel class
'''

from dataclasses import dataclass

import zfit
from dmu.stats.utilities     import print_pdf
from dmu.logging.log_store   import LogStore
from dmu.stats.model_factory import ModelFactory

log=LogStore.add_logger('dmu:stats:test_model_factory')
#--------------------------
@dataclass
class Data:
    '''
    Data class used to share
    '''
    obs = zfit.Space('mass', limits=(5080, 5680))

    l_arg_run3 = [
            ('2024', 'ETOS', 'ctrl'),
            ]

    l_arg_simple = [
            ('2018', 'MTOS', 'ctrl'),
            ('2018', 'MTOS', 'psi2'),
            ('2018', 'ETOS', 'ctrl'),
            ('2018', 'ETOS', 'psi2'),
            ]

    l_arg_syst = [
            ('2018', 'MTOS', 'ctrl'),
            ('2018', 'ETOS', 'ctrl'),
            ]

    l_arg_prc = [
            ('2018', 'ETOS', 'ctrl'),
            ('2018', 'ETOS', 'psi2'),
            ]

    l_arg_signal = [
            ('2018', 'ETOS', 'ctrl'),
            ('2018', 'ETOS', 'psi2'),
            ('2018', 'MTOS', 'ctrl'),
            ('2018', 'MTOS', 'psi2'),
            ]
#--------------------------
def test_unique_pdf():
    '''
    Will test only signal builder
    '''
    l_pdf = ['cbr', 'cbl', 'dscb']
    l_shr = ['mu', 'sg']
    mod   = ModelFactory(obs = Data.obs, l_pdf = l_pdf, l_shared=l_shr)
    pdf   = mod.get_pdf()

    print_pdf(pdf)
#--------------------------
def test_repeated_pdf():
    '''
    Will test only signal builder
    '''
    l_pdf = ['cbr'] + 2 * ['cbl']
    l_shr = ['mu', 'sg']
    mod   = ModelFactory(obs = Data.obs, l_pdf = l_pdf, l_shared=l_shr)
    pdf   = mod.get_pdf()

    print_pdf(pdf)
#--------------------------
