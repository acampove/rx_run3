'''
Script used to validate PDFs needed to fit combinatorial
'''
import os
import argparse
from typing import cast

import matplotlib.pyplot as plt
from dmu.logging.log_store  import LogStore
from dmu.generic            import utilities as gut
from dmu.generic            import naming
from dmu.stats.zfit         import zfit
from dmu.stats.fitter       import Fitter
from dmu.stats.zfit_plotter import ZFitPlotter

from ROOT                   import RDF
from zfit.interface         import ZfitData  as zdata
from zfit.interface         import ZfitPDF   as zpdf
from zfit.interface         import ZfitSpace as zobs
from rx_data.rdf_getter     import RDFGetter
from rx_selection           import selection as sel
from fitter                 import models

log=LogStore.add_logger('rx_fitter:validate_cmb')
# --------------------------------
class Data:
    '''
    Dataclass
    '''
    minx   : float
    maxx   : float
    mass   : str

    wp_cmb : float
    wp_prc : float

    obs    : zobs
    cfg    : dict
    out_dir: str
    q2bin  : str
    q2_kind: str|None=None
    model  : str
    config : str
    sample : str
    trigger: str
    initial: int
    final  : int
    ntries : int
    cutflow: str
    loglvl : int
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to perform fits to validate choice of PDF for combinatorial')
    parser.add_argument('-q', '--q2bin'  , type=str, help='Q2bin'         , choices=['low', 'cen_low', 'central', 'cen_high', 'jpsi', 'psi2', 'high'], required=True)
    parser.add_argument('-m', '--model'  , type=str, help='Fitting model' , choices=['HypExp', 'ModExp', 'Exp', 'Pol2', 'Pol3', 'SUJohnson'], default='SUJohnson')
    parser.add_argument('-k', '--q2_kind', type=str, help='Kind of q2 cut')
    parser.add_argument('-C', '--cutflow', type=str, help='Kind of cutflow'                               , default='mix')
    parser.add_argument('-c', '--config' , type=str, help='Name of config file'                           , default='validation')
    parser.add_argument('-s', '--sample' , type=str, help='Name of sample'                                , default='DATA*')
    parser.add_argument('-t', '--trigger', type=str, help='Name of trigger'                               , default='Hlt2RD_BuToKpEE_SameSign_MVA')
    parser.add_argument('-i', '--initial', type=int, help='Index of initial fit'                          , default=0)
    parser.add_argument('-f', '--final'  , type=int, help='Index of final fit, if not passed, will do all', default=1000)
    parser.add_argument('-n', '--ntries' , type=int, help='Maximum number of tries, default 1'            , default=1)
    parser.add_argument('-w', '--wpoint' , nargs=2 , help='Array with two working points, combinatorial and prec')
    parser.add_argument('-l', '--loglvl' , type=int, help='Logging level'                                 , default=20)
    args = parser.parse_args()

    if args.wpoint is not None:
        [ cmb, prc] = args.wpoint
        Data.wp_cmb = float(cmb)
        Data.wp_prc = float(prc)

    Data.q2bin  = args.q2bin
    Data.model  = args.model
    Data.cutflow= args.cutflow
    Data.q2_kind= args.q2_kind
    Data.config = args.config
    Data.sample = args.sample
    Data.trigger= args.trigger
    Data.initial= args.initial
    Data.final  = args.final
    Data.ntries = args.ntries
    Data.loglvl = args.loglvl
# --------------------------------
def _get_rdf() ->  RDF.RNode:
    gtr = RDFGetter(sample=Data.sample, trigger=Data.trigger)
    rdf = gtr.get_rdf(per_file=False)
    rdf = sel.apply_full_selection(
        rdf    =rdf,
        trigger=Data.trigger,
        q2bin  =Data.q2bin,
        process=Data.sample)

    return rdf
# --------------------------------
@gut.timeit
def _fit(pdf : zpdf, data : zdata) -> None:
    fit_cfg = Data.cfg['fitting']

    obj = Fitter(pdf, data)
    res = obj.fit(cfg=fit_cfg)

    return res
# --------------------------------
def _get_out_dir() -> str:
    ana_dir = os.environ['ANADIR']
    out_dir = Data.cfg['output']['path']

    if Data.q2_kind is not None:
        out_dir = f'{ana_dir}/{out_dir}/{Data.q2_kind}/{Data.q2bin}'
    else:
        out_dir = f'{ana_dir}/{out_dir}/{Data.q2bin}'

    os.makedirs(out_dir, exist_ok=True)

    return out_dir
# --------------------------------
def _plot(pdf : zpdf, data : zdata, name : str) -> None:
    suffix   = naming.clean_special_characters(name=name)
    nentries = data.value().shape[0]
    ext_text = f'Entries={nentries}\n{Data.sample}\n{Data.trigger}'

    obj= ZFitPlotter(data=data, model=pdf)
    rng= Data.cfg['fitting']['ranges']
    obj.plot(nbins=50, ranges=rng, title=name, ext_text=ext_text, d_leg={'ZPDF' : Data.model})

    obj.axs[0].axvline(x=5280, linestyle='--', color='gray', label='$B^+$')

    obj.axs[1].set_ylim((-5, +5))
    obj.axs[1].plot([Data.minx, Data.maxx], [+3, +3], linestyle='--', color='red')
    obj.axs[1].plot([Data.minx, Data.maxx], [-3, -3], linestyle='--', color='red')

    plot_path = f'{Data.out_dir}/fit_{suffix}.png'
    log.info(f'Saving to: {plot_path}')
    plt.savefig(plot_path)
# --------------------------------
def _override_q2(cuts : dict[str,str]) -> dict[str,str]:
    '''
    Parameters
    ------------------
    cuts: Dictionary storing selection

    Returns
    ------------------
    Dictionary with cuts, modifier for:

    - q2 kind: Can use track q2, etc
    '''
    if Data.q2_kind is None:
        log.debug('Not overriding q2 cut')
        return cuts

    cut = Data.cfg['q2_kind'][Data.q2_kind]
    log.info(f'Using q2 cut: {cut}')
    cuts['q2'] = cut

    return cuts
# --------------------------------
def _set_logs() -> None:
    '''
    Silence loggers to reduce noise
    '''
    LogStore.set_level('dmu:zfit_plotter'      , 30)
    LogStore.set_level('dmu:statistics:fitter' , 30)
    LogStore.set_level('rx_data:rdf_getter'    , 30)
    LogStore.set_level('rx_selection:selection', Data.loglvl)
    LogStore.set_level('rx_fitter:validate_cmb', Data.loglvl)
# --------------------------------
def _initialize() -> None:
    _set_logs()

    Data.cfg = gut.load_data(package='fitter_data', fpath=f'combinatorial/{Data.config}.yaml')
    d_obs    = Data.cfg['fits'][Data.q2bin]

    Data.minx= d_obs['observable']['minx']
    Data.maxx= d_obs['observable']['maxx']
    Data.mass= d_obs['observable']['name']

    Data.out_dir = _get_out_dir()
    Data.obs     = zfit.Space(Data.mass, limits=(Data.minx, Data.maxx))

    gut.TIMER_ON = False
# --------------------------------
def _skip_fit(index : int) -> bool:
    if Data.initial <= index <= Data.final:
        return False

    return True
# --------------------------------
def _get_mva_cuts() -> dict[str,str]:
    '''
    Returns
    -----------------
    Dictionary with MVA selections.
    If WP was set by user, will use that
    Otherwise pick it from config
    '''
    if hasattr(Data, 'wp_cmb') and hasattr(Data, 'wp_prc'):
        key = f'$BDT_{{cmb}} > {Data.wp_cmb:.2f}$ && $BDT_{{prc}} > {Data.wp_prc:.2f}$'
        cut = f'     mva_cmb > {Data.wp_cmb:.2f}  &&      mva_prc > {Data.wp_prc:.2f}'
        log.warning(f'Overriding WP with: {cut}')

        return {key : cut}

    log.debug('Picking up cutflow from YAML')
    return Data.cfg['cutflow'][Data.cutflow]
# --------------------------------
@gut.timeit
def _data_from_rdf(
    name : str,
    rdf : RDF.RNode) -> zdata:
    '''
    Parameters
    -------------
    name: Identifier of current dataset
    rdf : ROOT dataframe with original data in a given MVA region

    Returns
    -------------
    zfit data object
    '''
    log.info(f'  {name}')
    if log.getEffectiveLevel() < 20:
        rep = rdf.Report()
        rep.Print()

    arr_mass = rdf.AsNumpy([Data.mass])[Data.mass]
    data     = zfit.Data.from_numpy(obs=Data.obs, array=arr_mass)
    data     = cast(zdata, data)

    return data
# --------------------------------
def _get_zfit_data() -> dict[str,zdata]:
    '''
    Returns
    -------------
    Dictionary with:
        key: String identifying the bin where the data belongs
        value: zfit data object, that can be used to fit alongside PDF
    '''
    d_cut = Data.cfg['selection']
    d_cut = _override_q2(cuts=d_cut)

    d_mva_cuts = _get_mva_cuts()
    with sel.custom_selection(d_sel=d_cut),\
        RDFGetter.multithreading(nthreads=8):
        rdf   = _get_rdf()
        d_rdf = { name : rdf.Filter(expr, name)  for name, expr in d_mva_cuts.items()}

        log.info('Extracting zfit data from dataframes')
        d_data= { name : _data_from_rdf(rdf=rdf, name=name) for name, rdf in d_rdf.items()}

    return d_data
# --------------------------------
@gut.timeit
def main():
    '''
    Entry point
    '''
    _parse_args()
    _initialize()

    index = 0
    pdf   = models.get_pdf(obs=Data.obs, name=Data.model)
    d_data= _get_zfit_data()
    for name, data in d_data.items():
        if _skip_fit(index):
            log.info(f'Skipping {name}/{index}')
            index += 1
            continue

        log.info(f'Fitting {name}/{index}')

        _fit(pdf, data)
        _plot(pdf, data, name)

        index += 1
# --------------------------------
if __name__ == '__main__':
    main()
