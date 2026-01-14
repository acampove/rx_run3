'''
Script used to transform JSON files with fitting parameters to create a dataframe
with the widths and means of the Jpsi peak, fitted from data and MC samples
'''
import os
import math
import argparse

import mplhep
import jacobi
import matplotlib.pyplot     as plt
import pandas                as pnd

from dmu             import LogStore
from dmu.generic     import utilities        as gut
from typing          import Callable
from pathlib         import Path
from omegaconf       import DictConfig
from functools       import cache
from matplotlib.axes import Axes
from fitter          import FitSummary
from rx_q2           import ScalesConf
from rx_q2           import ParameterReader 
from fitter          import ParameterReader as FitParameterReader
from rx_common       import Project
from rx_common       import Trigger 
from rx_common       import Qsq 

log=LogStore.add_logger('rx_q2:dump_q2_ratios')

ARGS     : DictConfig | argparse.Namespace | None = None
PROJECTS : list[str] = ['rk_ee', 'rk_mm', 'rkst_ee', 'rkst_mm']
#-------------------------------------
@cache
def _load_config() -> ScalesConf:
    '''
    Returns
    -------------
    config class
    '''
    if ARGS is None:
        raise ValueError('Arguments not found')

    data         = gut.load_data(package='rx_q2_data', fpath=f'plots/scales_{ARGS.kind}.yaml')
    data['kind'] = ARGS.kind
    data['vers'] = ARGS.vers
    data['year'] = ARGS.year
    data['proj'] = ARGS.project

    _add_paths(data = data)

    cfg  = ScalesConf(**data)

    return cfg 
#-------------------------------------
def _add_paths(data : dict) -> None:
    '''
    Updates configuration data with paths and regex

    Parameters
    ---------------
    data: Dictionary with settings from:

    - Config file
    - User defined through CLI flags
    '''
    if ARGS is None:
        raise ValueError('Arguments not found')

    ana_dir = Path(os.environ['ANADIR'])

    if   ARGS.kind == 'q2':
        inp_dir = ana_dir / f'q2/fits/{ARGS.vers}'
        out_dir = ana_dir / f'q2/fits/{ARGS.vers}/plots/{ARGS.project}'
        regex   = r'(\d)_(\d)_nom'
    elif ARGS.kind == 'B':
        inp_dir = ana_dir / f'fits/data/reso_non_dtf/{ARGS.vers}/{ARGS.project}'
        out_dir = ana_dir / f'fits/data/reso_non_dtf/{ARGS.vers}/{ARGS.project}/plots'
        regex   = r'(\d{3})_(\d{3})_b(\d)'
    else:
        raise ValueError(f'Invalid scale type: {ARGS.kind}')

    out_dir.mkdir(parents=True, exist_ok=True)

    data['inp_dir'] = inp_dir
    data['out_dir'] = out_dir
    data['regex'  ] = regex
#-------------------------------------
def _get_Bx_df() -> pnd.DataFrame:
    name   = 'reso_non_dtf'
    signal = 'jpsi'
    cmb    = '070'
    prc    = '060'
    brems  = [1, 2]
    blocks = range(1, 9)
    kinds  = ['dat', 'sim']

    smr = FitSummary(name = name, signal = signal)
    smr.get_df()

    rdr = FitParameterReader(name = name)

    l_data = []
    for brem in brems:
        for block in blocks:
            for kind in kinds:
                msr = rdr(
                    brem     = brem, 
                    block    = block, 
                    cmb      = cmb,
                    prc      = prc,
                    kind     = kind,
                    trigger  = Trigger.rk_ee_os, 
                    project  = Project.rk,
                    q2bin    = Qsq.jpsi)

                mu_val, mu_err = msr.get_values(prefix = f'mu_{signal}')
                sg_val, sg_err = msr.get_values(prefix = f'sg_{signal}')

                data = {
                    'mu_val' : mu_val,
                    'mu_err' : mu_err,
                    'sg_val' : sg_val,
                    'sg_err' : sg_err,
                    'block'  : block,
                    'brem'   : brem,
                    'sample' : kind,
                }

                l_data.append(data)

    return pnd.DataFrame(l_data) 
#-------------------------------------
def _get_q2_df(sample : str | None = None) -> pnd.DataFrame:
    '''
    Arguments
    --------------
    sample : dat or sim
    '''
    if sample is None:
        df_dat = _get_q2_df(sample = 'dat')
        df_dat['sample'] = 'dat' 

        df_sim = _get_q2_df(sample = 'sim')
        df_sim['sample'] = 'sim' 

        return pnd.concat([df_dat, df_sim], axis=0, ignore_index=True)
    
    cfg     = _load_config()
    path_wc = cfg.inp_dir / f'{sample}/{cfg.proj}_{cfg.year}'
    l_path  = list(path_wc.glob(pattern = '*/parameters.json'))
    nfiles  = len(l_path)

    df = pnd.DataFrame(columns=['mu_val', 'mu_err', 'sg_val', 'sg_err', 'brem', 'block'])
    if nfiles == 0:
        raise ValueError(f'Cannot find any parameters file in: {path_wc}')

    log.info(f'Found {nfiles} parameters files')

    rdr = ParameterReader(cfg = cfg)
    for path in l_path:
        df.loc[len(df)] = rdr.read(path=path)

    return df
#-------------------------------------
def _get_scales(df : pnd.DataFrame) -> pnd.DataFrame:
    l_df_scale : list[pnd.DataFrame] = []
    for (block, brem), df_group in df.groupby(['block', 'brem']):
        try:
            iblock = int(block) # type: ignore[arg-type]
            ibrem  = int(brem)  # type: ignore[arg-type]
        except Exception:
            raise ValueError('Cannot cast block and brem as int')

        df_scale          = _scales_from_df(df=df_group)
        df_scale['block'] = iblock
        df_scale['brem' ] = ibrem

        l_df_scale.append(df_scale)

    if not l_df_scale:
        raise ValueError('No dataframes found to concatenate')

    df = pnd.concat(l_df_scale, ignore_index=True)

    return df
#-------------------------------------
def _reorder_blocks(df : pnd.DataFrame) -> pnd.DataFrame:
    custom_order = ['0', '4', '3', '1', '2', '1', '5', '6', '7', '8']
    df['block']  = df['block'].astype(str)

    order_map = {val: i for i, val in enumerate(custom_order)}
    df['order'] = df['block'].map(order_map)

    df= df.sort_values('order')

    return df
#-------------------------------------
def _scales_from_df(df : pnd.DataFrame) -> pnd.DataFrame:
    d_mu = _get_scale(df=df, name='mu', fun=lambda x : x[0] - x[1])
    d_sg = _get_scale(df=df, name='sg', fun=lambda x : x[0] / x[1])
    df   = pnd.DataFrame({**d_mu, **d_sg})

    return df
#-------------------------------------
def _get_scale(
    df   : pnd.DataFrame, 
    name : str, 
    fun  : Callable) -> dict[str,list[float]]:
    '''
    Parameters
    ------------------
    df  : Dataframe with mass scales and resolutions
    name: Variable whose 
    fun : Function used to calculate scale or resolution

    Returns
    ------------------
    Dictionary mapping:

    key  : Name of scale/resolution
    value: Numerical value
    '''
    df_dat  = df[df['sample'] == 'dat']
    df_sim  = df[df['sample'] == 'sim']

    dat_val, dat_err = _get_entry(name=name, df=df_dat)
    sim_val, sim_err = _get_entry(name=name, df=df_sim)

    cov : list[list[float]] = [
            [dat_err ** 2,            0],
            [0           , sim_err ** 2]]

    val, var = jacobi.propagate(fun, [dat_val, sim_val], cov) # type: ignore
    val      = float(val)
    err      = math.sqrt(var)

    if name == 'sg' and err > 100:
        log.warning('-----------------')
        log.info(f'Error: {err:.0f}')
        print(df)
        log.warning('-----------------')

    return {f's{name}_val' : [val], f's{name}_err' : [err]}
# ----------------------
def _get_entry(name : str, df : pnd.DataFrame) -> tuple[float,float]:
    '''
    Parameters
    -------------
    name: Quantity associated to column, e.g. mu, sg
    df  : DataFrame with scales and resolutions

    Returns
    -------------
    Tuple with value and error
    '''
    if len(df) != 1:
        log.error(df)
        raise ValueError('Expected one and only one row')

    try:
        val = df[f'{name}_val'].iloc[0]
        err = df[f'{name}_err'].iloc[0]
    except Exception:
        log.error(df)
        raise ValueError(f'Cannot find value or error of {name}')

    return val, err
#-------------------------------------
def _plot_df(
    df       : pnd.DataFrame,
    quantity : str,
    brem     : str,
    ax       : Axes) -> Axes:

    if brem == 0 and quantity == 'ssg':
        return ax

    color = {'0' : '#1f77b4', '1' : '#ff7f0e', '2' : '#2ca02c'}[brem]
    val   = f'{quantity}_val'
    err   = f'{quantity}_err'

    try:
        ax.fill_between(
            df['block'],
            df[val] - df[err],
            df[val] + df[err],
            color=color,
            label=f'Brem {brem}',
            alpha=0.5)
    except TypeError as exc:
        log.error(df)
        log.error(df.dtypes)
        raise TypeError('Cannot plot data in dataframe') from exc

    return ax
#-------------------------------------
def _plot_scales(df : pnd.DataFrame, quantity : str) -> None:
    _, ax = plt.subplots(figsize=(15, 10))
    for brem, df_brem in df.groupby('brem'):
        df_brem = _reorder_blocks(df=df_brem)
        brem    = str(brem)
        ax      = _plot_df(df=df_brem, quantity=quantity, brem=brem, ax=ax)

    ax.legend()

    cfg = _load_config()
    if quantity == 'smu':
        plt.ylabel(r'$\Delta\mu$[MeV]')
        rng = cfg.get_range(var='smu')
        plt.ylim(rng)

    if quantity == 'ssg':
        plt.ylabel(r'$s_{\sigma}$')
        rng = cfg.get_range(var='ssg')
        plt.ylim(rng)

    plt.grid()
    plt.savefig(cfg.out_dir / f'{quantity}.png')
    plt.close()
#-------------------------------------
def _plot_variables(df : pnd.DataFrame, quantity : str, kind : str) -> None:
    _, ax = plt.subplots(figsize=(15, 10))
    for brem, df_brem in df.groupby('brem'):
        brem    = str(brem)
        df_brem = _reorder_blocks(df = df_brem)
        ax      = _plot_df(df=df_brem, quantity=quantity, brem=brem, ax=ax)

    ax.legend()

    name = {'dat' : 'Data', 'sim' : 'MC'}[kind]

    cfg = _load_config()
    if quantity == 'mu':
        plt.ylabel(f'$\\mu^{{{name}}}$[MeV]')
        ax.axhline(y=cfg.jpsi_mass, color='black', linestyle=':', label='PDG')

        rng = cfg.get_range(var='mu')
        plt.ylim(rng)

    if quantity == 'sg':
        plt.ylabel(f'$\\sigma^{{{name}}}$[MeV]')
        rng = cfg.get_range(var='sg')
        plt.ylim(rng)

    plt.grid()
    plt.legend()
    plt.savefig(cfg.out_dir / f'{quantity}_{kind}.png')
    plt.close()
#-------------------------------------
def _plot(df : pnd.DataFrame):
    for kind, df_kind in df.groupby('sample'):
        kind = str(kind)

        _plot_variables(df=df_kind, quantity='mu', kind = kind)
        _plot_variables(df=df_kind, quantity='sg', kind = kind)

    df_scale = _get_scales(df)
    _plot_scales(df=df_scale, quantity='ssg')
    _plot_scales(df=df_scale, quantity='smu')
#-------------------------------------
def main(args : DictConfig | None = None):
    '''
    Parameters
    ----------------------
    args: Arguments passed to this script when using it as a module (e.g. with LAW),
    It should hold:

    - kind
    - vers
    - project
    - year (default 2024)
    '''
    global ARGS
    if   args is not None:
        ARGS = args
    else:
        parser = argparse.ArgumentParser(description='Used to create pandas dataframe with information from fits needed to smear q2')
        parser.add_argument('-k', '--kind'   , type=str, help='Type of scales' , required=True, choices=['q2', 'B'])
        parser.add_argument('-v', '--vers'   , type=str, help='Version'        , required=True)
        parser.add_argument('-p', '--project', type=str, help='Name of project', required=True, choices=PROJECTS)
        parser.add_argument('-y', '--year'   , type=str, help='Year for data whose corrections are needed', default='2024')
        ARGS = parser.parse_args()

    plt.style.use(mplhep.style.LHCb2)

    cfg  = _load_config()

    out_path = cfg.out_dir / 'parameters.json'
    if out_path.exists():
        log.warning(f'Dataframe already found, reusing: {out_path}')
        df = pnd.read_json(out_path)
        _plot(df=df)

        return

    log.info('Dataframe not found, making it')

    if   cfg.kind == 'q2':
        df = _get_q2_df()
    elif cfg.kind ==  'B':
        df = _get_Bx_df()
    else:
        raise ValueError(f'Invalid kind: {cfg.kind}')

    df.to_json(out_path, indent=2)

    _plot(df=df)
#-------------------------------------
if __name__ == '__main__':
    main()
