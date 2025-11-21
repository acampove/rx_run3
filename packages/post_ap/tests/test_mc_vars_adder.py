'''
Module containing tests for MCVarsAdder class
'''
import numpy
import pytest
from ROOT                    import RDataFrame
from ROOT                    import RDF
from dmu.logging.log_store   import LogStore

from post_ap.mc_vars_adder   import MCVarsAdder

log = LogStore.add_logger('post_ap:test_mc_vars_adder')
_SAMPLES=[
    'mc_24_w40_41_magdown_sim10f_12143001_bu_jpsik_mm_eq_dpc_tuple',
    'mc_24_w40_41_magdown_sim10f_12153001_bu_jpsik_ee_eq_dpc_tuple',
    'mc_24_w40_41_magdown_sim10f_12583022_bu_d0enu_kenu_eq_tc_tuple',
    'mc_24_w40_41_magdown_sim10f_13144025_bs_jpsikst_update2012_mm_eq_dpc_tuple',
    'mc_24_w41_42_magup_sim10f_11144001_bd_jpsikst_mm_eq_dpc_tuple',
    'mc_24_w41_42_magup_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple',
    'mc_24_w41_42_magup_sim10f_12125000_bu_kpipiee_eq_dpc_lsflat_tuple',
    'mc_24_w41_42_magup_sim10f_12143001_bu_jpsik_mm_eq_dpc_tuple',
    'mc_24_w41_42_magup_sim10f_12153001_bu_jpsik_ee_eq_dpc_tuple',
    'mc_24_w41_42_magup_sim10f_12583022_bu_d0enu_kenu_eq_tc_tuple',
    'mc_24_w41_42_magup_sim10f_13144025_bs_jpsikst_update2012_mm_eq_dpc_tuple',
    'mc_24_w31_magup_sim10f_11144001_bd_jpsikst_mm_eq_dpc_tuple',
    'mc_24_w31_magup_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple',
    'mc_24_w31_magup_sim10f_12125000_bu_kpipiee_eq_dpc_lsflat_tuple',
    'mc_24_w31_magup_sim10f_12143001_bu_jpsik_mm_eq_dpc_tuple',
    'mc_24_w31_magup_sim10f_12153001_bu_jpsik_ee_eq_dpc_tuple',
    'mc_24_w31_magup_sim10f_12583022_bu_d0enu_kenu_eq_tc_tuple',
    'mc_24_w31_magup_sim10f_13144025_bs_jpsikst_update2012_mm_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_12143001_bu_jpsik_mm_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_12153001_bu_jpsik_ee_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_12583022_bu_d0enu_kenu_eq_tc_tuple',
    'mc_24_w32_34_magup_sim10f_11144001_bd_jpsikst_mm_eq_dpc_tuple',
    'mc_24_w32_34_magup_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple',
    'mc_24_w32_34_magup_sim10f_12125000_bu_kpipiee_eq_dpc_lsflat_tuple',
    'mc_24_w32_34_magup_sim10f_12143001_bu_jpsik_mm_eq_dpc_tuple',
    'mc_24_w32_34_magup_sim10f_12153001_bu_jpsik_ee_eq_dpc_tuple',
    'mc_24_w32_34_magup_sim10f_12583022_bu_d0enu_kenu_eq_tc_tuple',
    'mc_24_w32_34_magup_sim10f_13144025_bs_jpsikst_update2012_mm_eq_dpc_tuple',
    'mc_24_w35_37_magup_sim10f_11144001_bd_jpsikst_mm_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_13144025_bs_jpsikst_update2012_mm_eq_dpc_tuple',
    'mc_24_w40_41_magdown_sim10f_11144001_bd_jpsikst_mm_eq_dpc_tuple',
    'mc_24_w40_41_magdown_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple',
    'mc_24_w40_41_magdown_sim10f_12125000_bu_kpipiee_eq_dpc_lsflat_tuple',
    'mc_24_w35_37_magup_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple',
    'mc_24_w35_37_magup_sim10f_12125000_bu_kpipiee_eq_dpc_lsflat_tuple',
    'mc_24_w35_37_magup_sim10f_12143001_bu_jpsik_mm_eq_dpc_tuple',
    'mc_24_w35_37_magup_sim10f_12153001_bu_jpsik_ee_eq_dpc_tuple',
    'mc_24_w35_37_magup_sim10f_12583022_bu_d0enu_kenu_eq_tc_tuple',
    'mc_24_w35_37_magup_sim10f_13144025_bs_jpsikst_update2012_mm_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_11144001_bd_jpsikst_mm_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple',
    'mc_24_w37_39_magdown_sim10f_12125000_bu_kpipiee_eq_dpc_lsflat_tuple',
]
# -------------------------------------------------
class Data:
    '''
    Class used to hold shared attributes
    '''
    ngen    = 1000
    nrec    = 1000
    rng     = numpy.random.default_rng(seed=10)
    arr_bpt = rng.uniform(0, 10_000, ngen)
# -------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def _initialize():
    LogStore.set_level('post_ap:mc_vars_adder'     , 10)
    LogStore.set_level('post_ap:test_mc_vars_adder', 10)
# -------------------------------------------------
def _get_rdf(kind : str, with_block : bool) -> RDataFrame:
    nentries = {'gen' : Data.ngen, 'rec' : Data.nrec}[kind]
    d_data   = {}

    arr_bpt        = Data.rng.choice(Data.arr_bpt, size=nentries, replace=False)
    if kind == 'gen':
        d_data['B_PT']     = numpy.sort(arr_bpt)
    else:
        d_data['B_TRUEPT'] = numpy.sort(arr_bpt)

    if kind == 'rec':
        d_data['EVENTNUMBER'] = Data.rng.integers(0, 1000_000, size=nentries)

    if with_block:
        d_data['block']       = Data.rng.choice([1,2], size=nentries)

    return RDF.FromNumpy(d_data)
# -------------------------------------------------
def _check_overlap(gen : numpy.ndarray, rec : numpy.ndarray):
    gen = gen.tolist()
    rec = rec.tolist()

    frq_1 = 0
    for elm in rec:
        if elm in gen:
            frq_1 += 1

    frq_2 = 0
    for elm in gen:
        if elm in rec:
            frq_2 += 1

    assert frq_1 == Data.nrec
    assert frq_2 == Data.nrec
# -------------------------------------------------
def test_add_to_gen():
    '''
    Tests addition of columns to MCDT
    '''
    sample  = 'mc_24_w41_42_magup_sim10f_11154001_bd_jpsikst_ee_eq_dpc_tuple'

    rdf_gen = _get_rdf(kind='gen', with_block=False)
    rdf_rec = _get_rdf(kind='rec', with_block=True)

    obj = MCVarsAdder(
        sample_name = sample,
        rdf_rec     = rdf_rec,
        rdf_gen     = rdf_gen)
    rdf_gen = obj.get_rdf()

    arr_gen_blk = rdf_gen.AsNumpy([      'block'])[      'block']
    arr_rec_blk = rdf_rec.AsNumpy([      'block'])[      'block']
    arr_gen_evt = rdf_gen.AsNumpy(['EVENTNUMBER'])['EVENTNUMBER']
    arr_rec_evt = rdf_rec.AsNumpy(['EVENTNUMBER'])['EVENTNUMBER']

    arr_rec     = numpy.array([arr_rec_evt, arr_rec_blk]).T
    arr_gen     = numpy.array([arr_gen_evt, arr_gen_blk]).T

    _check_overlap(gen=arr_gen, rec=arr_rec)
# -------------------------------------------------
@pytest.mark.parametrize('sample', _SAMPLES)
def test_add_to_rec(sample : str):
    '''
    Tests addition of columns to DecayTree
    '''

    rdf_rec = _get_rdf(kind='rec', with_block=False)

    obj = MCVarsAdder(
        sample_name = sample,
        rdf_rec     = rdf_rec)
    rdf       = obj.get_rdf()
    arr_block = rdf.AsNumpy(['block'])['block']

    assert any( block in arr_block for block in range(1,9) )
# -------------------------------------------------
