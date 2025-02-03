'''
Script used to plot mass distributions
'''
from importlib.resources import files
from dataclasses         import dataclass

import yaml
from ROOT                    import RDataFrame
from dmu.plotting.plotter_1d import Plotter1D
from rx_data.rdf_getter      import RDFGetter

# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    trigger_mm = 'Hlt2RD_BuToKpMuMu_MVA'
    trigger_ee = 'Hlt2RD_BuToKpEE_MVA'

    RDFGetter.samples_dir = '/home/acampove/Data/RX_run3/NO_q2_bdt_mass_Q2_central_VR_v1'
# ---------------------------------
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger=Data.trigger_mm)
    rdf = gtr.get_rdf()

    return rdf
# ---------------------------------
def _get_bdt_cutflow_rdf(rdf : RDataFrame) -> dict[str,RDataFrame]:
    return {'all' : rdf}
# ---------------------------------
def _get_cfg() -> dict:
    config_path = files('rx_plotter_data').joinpath('bdt_cutflow.yaml')
    config_path = str(config_path)

    with open(config_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    return cfg
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    rdf   = _get_rdf()
    d_rdf = _get_bdt_cutflow_rdf(rdf)
    cfg   = _get_cfg()

    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
if __name__ == 'main':
    main()
