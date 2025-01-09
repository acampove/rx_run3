'''
Module containing tests to:

    - Check that we can interface with triggercalib
'''
from importlib.resources import files

import yaml

from triggercalib import HltEff

# -------------------------------------------------
def _get_config() -> dict:
    trg_cfg = files('rx_calibration_data').joinpath('triggercalib/v0.yaml')
    trg_cfg = str(trg_cfg)
    with open(trg_cfg, encoding='utf-8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# -------------------------------------------------
def test_reference():
    '''
    Test taken from triggercalib reference
    '''
    hlt_eff = HltEff(
            "simple_example",
            '/home/acampove/cernbox/Run3/calibration/triggercalib/Bu2JpsiK_Jpsi2MuMu_block1_ntuple.root:Tuple/DecayTree',
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
    config  = _get_config()

    cfg     = config['reference']
    hlt_eff = HltEff(**cfg)
    hlt_eff.counts()
    hlt_eff.efficiencies()
    hlt_eff.write('/tmp/triggercalib/config.root')
# -------------------------------------------------
