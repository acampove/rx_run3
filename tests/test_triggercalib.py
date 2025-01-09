'''
Module containing tests to:

    - Check that we can interface with triggercalib
'''

from triggercalib import HltEff

def test_simple():
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
    hlt_eff.write('output.root')

