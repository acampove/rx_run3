'''
Module testing the MassCalculator class
'''
import os
import pytest
import numpy
import pandas            as pnd
import matplotlib.pyplot as plt
from pathlib import Path

from ROOT                    import RDataFrame, RDF # type: ignore
from dmu.logging.log_store   import LogStore
from rx_data.rdf_getter      import RDFGetter
from rx_data.mass_calculator import MassCalculator

log=LogStore.add_logger('rx_data:test_mass_calculator')

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_data:test_mass_calculator', 10)
    LogStore.set_level('rx_data:mass_calculator'     , 10)
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    user     = os.environ['USER']
    plot_dir = Path(f'/tmp/{user}/tests/rx_data/mass_calculator')
# ----------------------
def _check_entries(rdf_in : RDF.RNode, rdf_ot : RDF.RNode) -> None:
    '''
    Parameters
    -------------
    rdf_in/ot: Input/Output dataframe
    '''
    log.debug('Checking number of entries')

    nent_in = rdf_in.Count().GetValue()
    nent_ot = rdf_ot.Count().GetValue()

    assert nent_in == nent_ot
# ----------------------
def _closure_check(df : pnd.DataFrame) -> None:
    '''
    Parameters
    -------------
    df : Dataframe with arrays with masses
    '''
    log.debug('Run closure check')

    arr_mass_def = df['B_M'         ].dropna().to_numpy()
    arr_mass_cal = df['B_Mass_check'].dropna().to_numpy()

    assert numpy.allclose(arr_mass_def, arr_mass_cal, rtol=0.001)
# ----------------------
def _validate_rdf(
    test   : str,
    name   : str,
    rdf_in : RDataFrame|RDF.RNode,
    rdf_ot : RDataFrame|RDF.RNode) -> None:
    '''
    Parameters
    -------------
    test  : Test name, for naming output directory
    name  : Name of output in test, used for naming output plots
    rdf_in: Original dataframe
    rdf_ot: DataFrame with EVENTNUMBER, RUNNUMBER and masses
    '''
    _check_entries(rdf_in=rdf_in, rdf_ot=rdf_ot)

    s_col = { name.c_str() for name in rdf_ot.GetColumnNames() }
    assert s_col == {'EVENTNUMBER', 'RUNNUMBER', 'B_Mass_kkk', 'B_Mass_kpipi', 'B_M', 'B_Mass_check'}

    data = rdf_ot.AsNumpy(['B_M', 'B_Mass_kpipi', 'B_Mass_kkk', 'B_Mass_check'])
    df   = pnd.DataFrame(data)

    arr_mass_def = df['B_M'         ].dropna().to_numpy()
    arr_mass_cal = df['B_Mass_check'].dropna().to_numpy()

    assert numpy.allclose(arr_mass_def, arr_mass_cal, rtol=0.001)

    df= df.drop(columns=['B_Mass_check'])
    df.plot.hist(range=(4500, 6500), bins=100, histtype='step')
    plt.axvline(x=5280, label='PDG', linestyle=':')

    out_dir = Data.plot_dir/test
    out_dir.mkdir(parents=True, exist_ok=True)

    plt.title(f'Sample={name}')
    plt.savefig(out_dir/f'{name}.png')
    plt.close()
# ----------------------
@pytest.mark.parametrize('sample', [
    'Bu_Kee_eq_btosllball05_DPC',
    'Bu_piplpimnKpl_eq_sqDalitz_DPC',
    'Bu_KplKplKmn_eq_sqDalitz_DPC'])
def test_hadronic_mc(sample : str):
    '''
    Will run test where
    Kee -> KKK   in B_Mass_kkk
    Kee -> Kpipi in B_Mass_kpipi

    for simulated samples
    '''
    with RDFGetter.max_entries(value=10_000):
        gtr = RDFGetter(
            sample  = sample,
            trigger = 'Hlt2RD_BuToKpEE_MVA_noPID')
        rdf_in = gtr.get_rdf(per_file=False)

    cal    = MassCalculator(rdf=rdf_in, with_validation=True)
    rdf_ot = cal.get_rdf()

    _validate_rdf(rdf_in=rdf_in, rdf_ot=rdf_ot, test='hadronic_mc', name=sample)
# ----------------------
@pytest.mark.parametrize('sample' , ['DATA_24_MagDown_24c2'])
@pytest.mark.parametrize('trigger', ['Hlt2RD_BuToKpEE_MVA', 'Hlt2RD_BuToKpEE_MVA_misid'])
# ----------------------
def test_hadronic_data(sample : str, trigger : str):
    '''
    Will run test where
    Kee -> KKK   in B_Mass_kkk
    Kee -> Kpipi in B_Mass_kpipi

    for data samples
    '''
    with RDFGetter.max_entries(value=10_000):
        gtr = RDFGetter(
            sample  = sample,
            trigger = trigger)
        rdf_in = gtr.get_rdf(per_file=False)

    cal    = MassCalculator(rdf=rdf_in, with_validation=True)
    rdf_ot = cal.get_rdf()

    _validate_rdf(rdf_in=rdf_in, rdf_ot=rdf_ot, test='hadronic_data', name=f'{sample}_{trigger}')
