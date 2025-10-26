'''
Module with tests for Reader class in inclusive_decays_weights module
'''

import os
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from ROOT                            import RDF
from dmu.logging.log_store           import LogStore
from rx_data.rdf_getter              import RDFGetter
from fitter.inclusive_decays_weights import Reader

log=LogStore.add_logger('fitter:test_inclusive_decays_weights')
#-----------------------------------------------
class Data:
    '''
    Data class
    '''
    l_sample = [
        ('Bu_JpsiX_ee_eq_JpsiInAcc', 'Hlt2RD_BuToKpEE_MVA'),
        ('Bd_JpsiX_ee_eq_JpsiInAcc', 'Hlt2RD_BuToKpEE_MVA'),
        ('Bs_JpsiX_ee_eq_JpsiInAcc', 'Hlt2RD_BuToKpEE_MVA'),
        ]

    out_dir = '/tmp/tests/fitter/inclusive_decays_weights'
#-----------------------------------------------
def _rdf_to_idf(rdf : RDF.RNode) -> pnd.DataFrame:
    rdf   =rdf.Define('mass', 'B_const_mass_M')
    v_name=rdf.GetColumnNames()
    l_name=[ name.c_str() for name in v_name ]
    l_name=[ name for name in l_name if 'TRUEID' in name or 'MOTHER_ID' in name] + ['mass']

    d_id = rdf.AsNumpy(l_name)

    df = pnd.DataFrame(d_id)

    return df
#-----------------------------------------------
def _get_df(sample : str, trigger : str) -> pnd.DataFrame:
    with RDFGetter.max_entries(10_000):
        gtr = RDFGetter(sample = sample, trigger = trigger)
        rdf = gtr.get_rdf(per_file=False)

    df  = _rdf_to_idf(rdf)

    return df
#-----------------------------------------------
def _plot_mass(df : pnd.DataFrame, sample : str, test : str):
    _, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))
    ax1.hist(df.mass, bins=50, range=[4500, 6000], histtype='step', density=True, label='Unweighted', )
    ax1.hist(df.mass, bins=50, range=[4500, 6000], histtype='step', density=True, label='Weighted'  , weights=df.weight)

    nevs=df.weight.size
    area=df.weight.sum()

    title = f'{sample}; evs: {nevs}; sum: {area:.0f}'

    ax1.set_title(title)
    ax1.legend()

    ax2.hist(df.weight, bins=50, edgecolor='black')

    out_dir = f'{Data.out_dir}/{test}'
    os.makedirs(out_dir, exist_ok=True)

    out_path = f'{out_dir}/{sample}.png'

    plt.savefig(out_path)
    plt.close()
#-----------------------------------------------
@pytest.mark.parametrize('sample, trigger', Data.l_sample)
def test_simple(sample : str, trigger : str):
    '''
    Simplest test of addition of weights
    '''
    df           = _get_df(sample, trigger)
    df['weight'] = df.apply(Reader.read_weight, args=('L1', 'L2', 'H'), axis=1)

    _plot_mass(df, sample, 'simple')
#-----------------------------------------------
