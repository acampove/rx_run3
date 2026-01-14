'''
Script used to transform JSON files with fitting parameters to create a dataframe
with the widths and means of the Jpsi peak, fitted from data and MC samples
'''
import os
import re
import math
import argparse

import mplhep
import jacobi
import matplotlib.pyplot     as plt
import pandas                as pnd

from dmu                   import LogStore
from dmu.generic           import utilities        as gut
from typing                import Callable, Any
from pathlib               import Path
from omegaconf             import DictConfig
from functools             import cache
from pydantic              import BaseModel, ConfigDict
from matplotlib.axes       import Axes

log=LogStore.add_logger('rx_q2:dump_q2_ratios')

ARGS     : DictConfig | argparse.Namespace | None = None
PROJECTS : list[str] = ['rk_ee', 'rk_mm', 'rkst_ee', 'rkst_mm']
#-------------------------------------
class ScalesConf(BaseModel):
    '''
    Class storing configuration
    '''
    model_config = ConfigDict(frozen=True)
    jpsi_mass : float

    mm      : dict[str,list[float]]
    ee      : dict[str,list[float]]

    kind    : str
    vers    : str
    year    : str
    proj    : str

    inp_dir : Path
    out_dir : Path
    regex   : str
    # ------------------------------
    def get_range(self, var : str) -> tuple[float,float]:
        '''
        Parameters
        -----------------
        var: Variable name, e.g. mu, sg, dm, ss

        Returns
        -----------------
        Y axis range for plot
        '''
        if   self.proj.endswith('_mm'):
            data = self.mm
        elif self.proj.endswith('_ee'):
            data = self.ee
        else:
            raise ValueError(f'Invalid project: {self.proj}')

        if var not in data:
            raise ValueError(f'Invalid variable: {var}')

        [low, high] = data[var]

        return low, high
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

    data         = gut.load_data(package='rx_q2_data', fpath='plots/scales.yaml')
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
        inp_dir = ana_dir / f'fits/data/reso_no_dtf/{ARGS.vers}/{ARGS.project}'
        out_dir = ana_dir / f'fits/data/reso_no_dtf/{ARGS.vers}/{ARGS.project}/plots'
        regex   = r'(\d{3})_(\d{3})_b(\d)'
    else:
        raise ValueError(f'Invalid scale type: {ARGS.kind}')

    out_dir.mkdir(parents=True, exist_ok=True)

    data['inp_dir'] = inp_dir
    data['out_dir'] = out_dir
    data['regex'  ] = regex
#-------------------------------------
def _row_from_path(path : Path) -> list[Any]:
    data = gut.load_json(path)

    [[mu_val, mu_err]] = [ val for name, val in data.items() if name.startswith('mu_')]
    [[sg_val, sg_err]] = [ val for name, val in data.items() if name.startswith('sg_')]

    brem, block = _brem_block_from_path(path=path)

    return [mu_val, mu_err, sg_val, sg_err, brem, block]
#-------------------------------------
def _brem_block_from_path(path : Path) -> tuple[str,str]:
    dir_name = os.path.dirname(path)
    sample   = os.path.basename(dir_name)

    mtch     = re.match(Data.regex, sample)
    if not mtch:
        raise ValueError(f'Cannot extract information from {sample} using {Data.regex}')

    [brem, block] = mtch.groups()

    return brem, block
#-------------------------------------
def _get_df(
    sample : str,
    project: str,
    year   : str) -> pnd.DataFrame:
    '''
    Arguments
    --------------
    sample : dat or sim
    project: e.g. rk_ee
    year   : 2024
    '''
    path_wc = f'{Data.inp_dir}/{sample}/{project}_{year}/*/parameters.json'
    l_path  = glob.glob(path_wc)
    nfiles  = len(l_path)

    df = pnd.DataFrame(columns=['mu_val', 'mu_err', 'sg_val', 'sg_err', 'brem', 'block'])
    if nfiles == 0:
        raise ValueError(f'Cannot find any parameters file in: {path_wc}')

    log.info(f'Found {nfiles} parameters files')
    for path in l_path:
        df.loc[len(df)] = _row_from_path(path=path)

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

    ax.fill_between(
        df['block'],
        df[val] - df[err],
        df[val] + df[err],
        color=color,
        label=f'Brem {brem}',
        alpha=0.5)

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
    plt.savefig(f'{Data.out_dir}/{quantity}.png')
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
    plt.savefig(f'{Data.out_dir}/{quantity}_{kind}.png')
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
    Starts here
    '''
    Data.args = args

    cfg = _load_config()
    _initialize(cfg=cfg)

    out_path = f'{Data.out_dir}/parameters.json'
    if os.path.isfile(out_path):
        log.warning(f'Dataframe already found, reusing: {out_path}')
        df = pnd.read_json(out_path)
        _plot(df=df)

        return

    log.info('Dataframe already not found, making it')
    l_df : list[pnd.DataFrame] = []
    for sample in ['dat', 'sim']:
        df           = _get_df(sample=sample, project=cfg.proj, year=cfg.year)
        df['sample'] = sample
        l_df.append(df)

    df=pnd.concat(l_df, axis=0, ignore_index=True)
    df.to_json(out_path, indent=2)

    _plot(df=df)
#-------------------------------------
if __name__ == '__main__':
    main()
