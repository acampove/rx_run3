'''
Module containing tests for MVACalculator class
'''
import os
import pytest
import matplotlib.pyplot as plt
from pathlib                      import Path
from ROOT                         import RDF # type: ignore
from dmu.logging.log_store        import LogStore
from rx_data.rdf_getter           import RDFGetter
from rx_data.mva_calculator       import MVACalculator
from rx_common.types              import Trigger

log=LogStore.add_logger('rx_data:test_mva_calculator')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    l_trigger= [Trigger.rk_mm_os, Trigger.rk_ee_os]
    nentries = 30_000
    version  = 'v8'
    l_mc     = [
        ('Hlt2RD_BuToKpEE_MVA'  , 'Bu_Kee_eq_btosllball05_DPC'  ),
        ('Hlt2RD_BuToKpMuMu_MVA', 'Bu_Kmumu_eq_btosllball05_DPC')]

    l_nopid  = [('Hlt2RD_BuToKpEE_MVA', 'Bu_JpsiK_ee_eq_DPC')]
# ----------------------
def _validate_rdf(
    rdf     : RDF.RNode,
    name    : str,
    out_dir : Path) -> None:
    '''
    Parameters
    -------------
    rdf: ROOT dataframe with MVA scores
    '''
    log.info('Validating dataframe')

    os.makedirs(out_dir, exist_ok=True)

    nentries = rdf.Count().GetValue()
    assert nentries == Data.nentries

    log.info('Found columns:')
    for col_name in rdf.GetColumnNames():
        log.info(f'    {col_name}')

    log.info('Extracting data')
    data = rdf.AsNumpy()

    log.info('Plotting')

    plt.hist(data['mva_cmb'], bins=50, range=(-1,+1), histtype='step', label='Combinatorial')
    plt.hist(data['mva_prc'], bins=50, range=(-1,+1), histtype='step', label='Part reco')
    plt.savefig(f'{out_dir}/{name}.png')
    plt.close()
# ----------------------
@pytest.mark.parametrize('trigger', Data.l_trigger)
def test_data(trigger : Trigger, tmp_path : Path) -> None:
    '''
    Test MVACalculator with data
    '''
    sample = 'DATA_24*'

    with RDFGetter.max_entries(value=Data.nentries):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    cal = MVACalculator(
    rdf     = rdf,
    sample  = sample,
    trigger = trigger,
    version = Data.version)

    rdf = cal.get_rdf(kind = 'root')
    _validate_rdf(rdf=rdf, out_dir=tmp_path, name=f'data_{trigger}')
# --------------------------------
@pytest.mark.parametrize('trigger, sample', Data.l_mc)
def test_mc(trigger : Trigger, sample : str, tmp_path : Path) -> None:
    '''
    Test MVACalculator with simulation
    '''
    with RDFGetter.max_entries(value=Data.nentries):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    cal = MVACalculator(
    rdf     = rdf,
    sample  = sample,
    trigger = trigger,
    version = Data.version)

    rdf = cal.get_rdf(kind = 'root')
    _validate_rdf(rdf=rdf, out_dir=tmp_path, name=f'{sample}_{trigger}')
# ----------------------
@pytest.mark.parametrize('trigger, sample', Data.l_mc)
def test_mc_noversion(trigger : Trigger, sample : str, tmp_path : Path) -> None:
    '''
    Test MVACalculator with default (should be lates) version
    '''
    with RDFGetter.max_entries(value=Data.nentries):
        gtr = RDFGetter(sample=sample, trigger=trigger)
        rdf = gtr.get_rdf(per_file=False)

    cal = MVACalculator(
    rdf     = rdf,
    sample  = sample,
    trigger = trigger)

    rdf = cal.get_rdf(kind = 'root')
    _validate_rdf(rdf=rdf, out_dir=tmp_path, name=f'{sample}_{trigger}')
# --------------------------------
@pytest.mark.parametrize('trigger, sample', Data.l_nopid)
def test_nopid(trigger : Trigger, sample : str, tmp_path : Path) -> None:
    '''
    Test MVACalculator with default (should be lates) version
    '''
    with RDFGetter.max_entries(value=Data.nentries),\
        RDFGetter.exclude_friends(names=['mva']):
        gtr = RDFGetter(sample=sample, trigger=trigger) 
        rdf = gtr.get_rdf(per_file=False)

    cal = MVACalculator(
    rdf     = rdf,
    sample  = sample,
    trigger = trigger)

    rdf = cal.get_rdf(kind = 'root')
    _validate_rdf(rdf=rdf, out_dir=tmp_path, name=f'{sample}_{trigger}')
# --------------------------------
