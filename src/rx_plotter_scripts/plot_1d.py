'''
Script used to plot mass distributions
'''
import argparse
from importlib.resources import files
from dataclasses         import dataclass

import yaml
import mplhep
from ROOT                    import RDataFrame, EnableImplicitMT
from dmu.plotting.plotter_1d import Plotter1D
from rx_data.rdf_getter      import RDFGetter
from rx_selection.selection  import load_selection_config

# ---------------------------------
@dataclass
class Data:
    '''
    Class used to share attributes
    '''
    nthreads   = 13
    trigger_mm = 'Hlt2RD_BuToKpMuMu_MVA'
    trigger_ee = 'Hlt2RD_BuToKpEE_MVA'
    d_reso     = {'jpsi' : 'B_const_mass_M', 'psi2' : 'B_const_mass_psi2S_M'}

    mplhep.style.use('LHCb1')

    RDFGetter.samples_dir = '/home/acampove/Data/RX_run3/NO_q2_bdt_mass_Q2_central_VR_v1'

    chanel  : str
    trigger : str
    q2_bin  : str
    q2_cut  : str
# ---------------------------------
def _get_rdf() -> RDataFrame:
    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger=Data.trigger)
    rdf = gtr.get_rdf()

    return rdf
# ---------------------------------
def _get_bdt_cutflow_rdf(rdf : RDataFrame) -> dict[str,RDataFrame]:
    d_rdf = {}
    for cmb in [0.2, 0.4, 0.6, 0.8]:
        rdf = rdf.Filter(f'mva.mva_cmb > {cmb}')
        d_rdf [f'$MVA_{{cmb}}$ > {cmb}'] = rdf

    for prc in [0.2, 0.4, 0.6]:
        rdf = rdf.Filter(f'mva.mva_prc > {prc}')
        d_rdf [f'$MVA_{{prc}}$ > {prc}'] = rdf

    return d_rdf
# ---------------------------------
def _q2cut_from_q2bin(q2bin : str) -> str:
    cfg = load_selection_config()
    cut = cfg['q2_common'][q2bin]

    return cut
# ---------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to make plots')
    parser.add_argument('-q', '--q2bin' , type=str, help='q2 bin' , choices=['low', 'central', 'jpsi', 'psi2', 'high'], required=True)
    parser.add_argument('-c', '--chanel', type=str, help='Channel', choices=['ee', 'mm'], required=True)
    args = parser.parse_args()

    Data.q2_bin = args.q2bin
    Data.q2_cut = _q2cut_from_q2bin(args.q2bin)
    Data.trigger= Data.trigger_mm if args.chanel == 'mm' else Data.trigger_ee
    Data.chanel = args.chanel
# ---------------------------------
def _get_cfg(kind : str) -> dict:
    config_path = files('rx_plotter_data').joinpath('bdt_q2_mass.yaml')
    config_path = str(config_path)

    with open(config_path, encoding='utf=8') as ifile:
        cfg = yaml.safe_load(ifile)

    cfg = _override_cfg(cfg)

    if kind != 'last':
        return cfg

    d_plt        = cfg['plots']
    d_mas        = d_plt['B_M']
    if Data.chanel == 'mm':
        [_, maxx, nbins] = d_mas['binning']
        d_mas['binning'] = [5180, maxx, nbins]

    name         = d_mas['name']
    d_mas['name']= f'{name}_last'
    cfg['plots'] = {'B_M' : d_mas}

    return cfg
# ---------------------------------
def _add_reso_q2(cfg : dict) -> dict:
    d_mass              = cfg['plots']['B_M']
    d_mass              = dict(d_mass)
    d_mass['labels'][0] = r'M${}_{DTF}(B^+)$'
    d_mass['name'  ]    = f'DTF_mass_{Data.q2_bin}'

    reso_mass = Data.d_reso[Data.q2_bin]
    cfg['plots'][reso_mass] = d_mass

    return cfg
# ---------------------------------
def _override_cfg(cfg : dict) -> dict:
    plt_dir                    = cfg['saving']['plt_dir']
    cfg['saving']['plt_dir']   = f'{plt_dir}/{Data.trigger}'
    cfg['selection']['cuts']   = {'q2' : Data.q2_cut}
    cfg['style']['skip_lines'] = Data.chanel == 'mm'

    if Data.q2_bin in Data.d_reso:
        cfg = _add_reso_q2(cfg)

    for d_plot in cfg['plots'].values():
        if 'title' not in d_plot:
            d_plot['title'] = ''

        title = d_plot['title']
        title = f'{Data.q2_bin} $q^2$ {title}'
        d_plot['title'] = title

        name           = d_plot['name']
        d_plot['name'] = f'{Data.q2_bin}_{name}'

    return cfg
# ---------------------------------
def _plot(kind : str, d_rdf : dict[str,RDataFrame]) -> None:
    cfg   = _get_cfg(kind)

    if kind == 'last':
        key, val = d_rdf.popitem()
        d_rdf    = {key : val}

        cfg['stats'] = {'nentries' : '\nEntries: {}'}

    ptr=Plotter1D(d_rdf=d_rdf, cfg=cfg)
    ptr.run()
# ---------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    EnableImplicitMT(Data.nthreads)

    rdf   = _get_rdf()
    d_rdf = _get_bdt_cutflow_rdf(rdf)

    _plot(kind='cutflow', d_rdf = d_rdf)
    _plot(kind='last'   , d_rdf = d_rdf)
# ---------------------------------
if __name__ == 'main':
    main()
