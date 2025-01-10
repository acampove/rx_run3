'''
Module containing tests to:

    - Check that we can interface with triggercalib
'''
from importlib.resources import files

import yaml
import pytest

from triggercalib          import HltEff
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('rx_calibration:test_triggercalib')
# -------------------------------------------------
def _get_config(name : str, sample : str) -> dict:
    trg_cfg = files('rx_calibration_data').joinpath(f'triggercalib/{name}')
    trg_cfg = str(trg_cfg)
    with open(trg_cfg, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    d_sam   = cfg['samples']
    d_set   = cfg['settings']
    path    = d_sam[sample]

    d_set.update({'name' : sample, 'path' : path})

    return d_set
# -------------------------------------------------
def _print_conf(cfg : dict) -> None:
    tis = cfg['tis']
    tis = str(tis)

    tos = cfg['tos']
    tos = str(tos)

    log.info('')
    log.info(f'{"TIS":<20}{tis:<20}')
    log.info(f'{"TOS":<20}{tos:<20}')
# -------------------------------------------------
def test_reference():
    '''
    Test taken from triggercalib reference
    '''
    hlt_eff = HltEff(
            "simple_example",
            '/home/acampove/cernbox/Run3/calibration/triggercalib/reference/Bu2JpsiK_Jpsi2MuMu_block1_ntuple.root:Tuple',
            tos="Hlt1TrackMVA",
            tis=["Hlt1TrackMVA", "Hlt1TwoTrackMVA"],
            particle="B",
            binning={
                "B_PT" : {
                    "bins" : [
                        n*1e3 for n in [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 25]
                        ]
                    }
                },
            sideband={
                "B_DTF_Jpsi_MASS": {
                    "signal": [5280 - 55, 5280 + 55],
                    "sidebands": [
                        [5280 - 150, 5280 - 55],
                        [5280 + 55, 5280 + 150],
                        ]
                    }
                }
            )

    hlt_eff.counts()
    hlt_eff.efficiencies()
    hlt_eff.write('/tmp/triggercalib/reference.root')
# -------------------------------------------------
def test_config():
    '''
    Same as test_reference, but with config file
    '''
    cfg = _get_config(name='v0.yaml', sample='reference')

    hlt_eff = HltEff(**cfg)
    hlt_eff.counts()
    hlt_eff.efficiencies()
    hlt_eff.write('/tmp/triggercalib/config.root')
# -------------------------------------------------
@pytest.mark.parametrize('sample', ['data', 'simulation'])
def test_real(sample : str):
    '''
    Will run tests over MC
    '''
    cfg = _get_config(name='v1.yaml', sample=sample)

    hlt_eff = HltEff(**cfg)
    hlt_eff.counts()
    hlt_eff.efficiencies()
    hlt_eff.write(f'/tmp/triggercalib/{sample}.root')
# -------------------------------------------------
