'''
Module with tests for DBMaker
'''
import pytest

from ROOT         import RDataFrame, RDF
from typing       import Any
from pathlib      import Path
from rx_selection import selection
from rx_common    import Cuts
from rx_common    import Trigger
from rx_common    import Component
from rx_data      import DBMaker, DBMakerConf
from rx_data      import db_maker

from dmu.rdataframe import utilities as rut
from dmu.generic    import utilities as gut
from dmu.generic    import Mode
from dmu            import LogStore

log=LogStore.add_logger('rx_data:test_dbmaker')
# -------------------------
class TestRDFGetter:
    '''
    Class meant to replace RDFGetter in tests
    '''
    def __init__(self, tree : Any, trigger : Any, sample : Any):
        pass

    def get_rdf(self, per_file : bool):
        _   = per_file

        rdf = RDataFrame(10_000)
        rdf = rdf.Define('EVENTNUMBER', '1')
        rdf = rdf.Define('mva_cmb', 'gRandom->Uniform(0,   1)')
        rdf = rdf.Define('mva_prc', 'gRandom->Uniform(0,   1)')
        rdf = rdf.Define('mass'   , 'gRandom->Uniform(0, 100)')

        return rdf
# -------------------------
def selector(
    q2bin   : Any,
    trigger : Any,
    process : Any,
    out_path: Path | int,
    rdf     : RDF.RNode) -> RDF.RNode:

    _ = q2bin
    _ = trigger
    _ = process

    cuts = {
        Cuts.mass : 'mass    >  50',
        Cuts.prc  : 'mva_prc > 0.5',
        Cuts.cmb  : 'mva_cmb > 0.5',
    }

    overriding = selection.Data.d_custom_selection
    if overriding:
        cuts.update(overriding)

    for name, cut in cuts.items():
        rdf = rdf.Filter(cut, name)

    setattr(rdf, 'sel', cuts) # noqa: B010
    if out_path == 0:
        return rdf

    if not isinstance(out_path, Path):
        raise ValueError(f'Output path not a path: {out_path}')

    rep = rdf.Report()
    df  = rut.rdf_report_to_df(rep=rep)
    df.to_markdown(out_path / 'cuts.md')

    gut.dump_json(data = cuts, path = out_path / 'cuts.json')

    return rdf
# -------------------------
@pytest.fixture(autouse = False)
def _patch_rdfgetter(monkeypatch):
    monkeypatch.setattr(db_maker , 'RDFGetter'           , TestRDFGetter)
    monkeypatch.setattr(selection, 'apply_full_selection', selector)
# -------------------------------------
def test_make_db(
    monkeypatch,
    _patch_rdfgetter,
    tmp_path : Path):
    '''
    Test making of database
    '''
    monkeypatch.setenv('ANADIR', str(tmp_path))

    cfg = DBMakerConf(
        component    = Component.bpkpee,
        trigger      = Trigger.rk_ee_os,
        ncores       = 1,
        cuts         = {Cuts.cmb : 'mva_cmb > 0.1'},
        dropped_cuts = [Cuts.prc])

    mkr = DBMaker(cfg = cfg)
    mkr.run(mode = Mode.write)
# -------------------------------------
def test_read_db(
    monkeypatch,
    _patch_rdfgetter,
    tmp_path : Path):
    '''
    Test making of database
    '''
    monkeypatch.setenv('ANADIR', str(tmp_path))

    cfg = DBMakerConf(
        component    = Component.bpkpee,
        trigger      = Trigger.rk_ee_os,
        ncores       = 1,
        dropped_cuts = [Cuts.prc])

    mkr = DBMaker(cfg = cfg)
    with pytest.raises(ValueError):
        mkr.run(mode = Mode.read)

    mkr.run(mode = Mode.write)

    rdf = mkr.run(mode = Mode.read)
    assert hasattr(rdf, 'sel')
    assert hasattr(rdf, 'uid')

    assert isinstance(rdf.uid, str)
# -------------------------------------
