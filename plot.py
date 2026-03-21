from typing import Any
import yaml
import mplhep
import pandas            as pnd
import matplotlib.pyplot as plt

from pydantic  import TypeAdapter
from dmu.stats import Constraint1D, ConstraintND
from dmu.stats import FitResult
from dmu.stats import ConstraintType
from pathlib   import Path

plt.style.use(mplhep.style.LHCb2)
labels = {
    'nd_mu_hypexp_comb_main_1'       : r'$\mu_{comb}$',
    'nd_ap_hypexp_comb_main_1'       : r'$\alpha_{comb}$',
    'nd_bt_hypexp_comb_main_1'       : r'$\beta_{comb}$',
    'fr_bpkpee_block_x12_b1_flt'     : r'$f_1^{\mathrm{Block}}$',
    'fr_bpkpee_block_x12_b2_flt'     : r'$f_2^{\mathrm{Block}}$',
    'fr_bpkpee_block_x12_b3_flt'     : r'$f_3^{\mathrm{Block}}$',
    'fr_bpkpee_block_x12_b4_flt'     : r'$f_4^{\mathrm{Block}}$',
    'fr_bpkpee_block_x12_b5_flt'     : r'$f_5^{\mathrm{Block}}$',
    'fr_bpkpee_block_x12_b6_flt'     : r'$f_6^{\mathrm{Block}}$',
    'fr_bpkpee_block_x12_b7_flt'     : r'$f_7^{\mathrm{Block}}$',
    'fr_bpkpee_brem_xx1_b1_reso_flt' : r'$f_1^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b2_reso_flt' : r'$f_2^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b3_reso_flt' : r'$f_3^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b4_reso_flt' : r'$f_4^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b5_reso_flt' : r'$f_5^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b6_reso_flt' : r'$f_6^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b7_reso_flt' : r'$f_7^{\mathrm{Brem}_1}$',
    'fr_bpkpee_brem_xx1_b8_reso_flt' : r'$f_8^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b1_scale_flt': r'$\mu_1^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b2_scale_flt': r'$\mu_2^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b3_scale_flt': r'$\mu_3^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b4_scale_flt': r'$\mu_4^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b5_scale_flt': r'$\mu_5^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b6_scale_flt': r'$\mu_6^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b7_scale_flt': r'$\mu_7^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx1_b8_scale_flt': r'$\mu_8^{\mathrm{Brem}_1}$',
    'mu_bpkpee_brem_xx2_b1_scale_flt': r'$\mu_1^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b2_scale_flt': r'$\mu_2^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b3_scale_flt': r'$\mu_3^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b4_scale_flt': r'$\mu_4^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b5_scale_flt': r'$\mu_5^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b6_scale_flt': r'$\mu_6^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b7_scale_flt': r'$\mu_7^{\mathrm{Brem}_2}$',
    'mu_bpkpee_brem_xx2_b8_scale_flt': r'$\mu_8^{\mathrm{Brem}_2}$',
    's_bdkstkpiee'                   : r'$s_{B^0}$',
    's_bpkstkpiee'                   : r'$s_{B^+}$',
    'sg_bpkpee_brem_xx1_b1_reso_flt' : r'$\sigma_1^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b2_reso_flt' : r'$\sigma_2^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b3_reso_flt' : r'$\sigma_3^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b4_reso_flt' : r'$\sigma_4^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b5_reso_flt' : r'$\sigma_5^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b6_reso_flt' : r'$\sigma_6^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b7_reso_flt' : r'$\sigma_7^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx1_b8_reso_flt' : r'$\sigma_8^{\mathrm{Brem}_1}$',
    'sg_bpkpee_brem_xx2_b1_reso_flt' : r'$\sigma_1^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b2_reso_flt' : r'$\sigma_2^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b3_reso_flt' : r'$\sigma_3^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b4_reso_flt' : r'$\sigma_4^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b5_reso_flt' : r'$\sigma_5^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b6_reso_flt' : r'$\sigma_6^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b7_reso_flt' : r'$\sigma_7^{\mathrm{Brem}_2}$',
    'sg_bpkpee_brem_xx2_b8_reso_flt' : r'$\sigma_8^{\mathrm{Brem}_2}$',
    'yld_bpkkk'                      : r'$N_{KK}^{\mathrm{misID}}$',
    'yld_bpkpipi'                    : r'$N_{\pi\pi}^{\mathrm{misID}}$',
}
# ---------------------------------------
def _add_entries_1D(
    res : FitResult,
    cns : Constraint1D) -> dict[str,Any]:

    val, err = res[cns.name]

    data            = dict()
    data['par_nam'] = cns.name
    data['pre_val'] = 0
    data['pre_err'] = err / cns.sg

    data['pos_val'] = abs(val - cns.mu) / cns.sg
    data['pos_err'] = 1

    return data
# ---------------------------------------
def _add_entries_ND(
    res : FitResult,
    cns : ConstraintND) -> list[dict[str,Any]]:

    values : list[dict[str,Any]] = []
    for name, mu, sg in zip(cns.parameters, cns.values, cns.errors, strict=True):
        data            = dict()
        norm            = abs(mu)
        val, err        = res[name]

        data['par_nam'] = f'nd_{name}'
        data['pre_val'] = 0 
        data['pre_err'] = err / sg 

        data['pos_val'] = abs(val - norm) / err
        data['pos_err'] = 1

        values.append(data)

    return values 
# ---------------------------------------
def constraint_type_constructor(loader, node):
    values = loader.construct_sequence(node)
    return ConstraintType(values[0])

yaml.add_constructor(
    'tag:yaml.org,2002:python/object/apply:dmu.stats.types.ConstraintType',
    constraint_type_constructor,
    Loader=yaml.SafeLoader
)
# ---------------------------------------
def _plot(regex : str, df : pnd.DataFrame, limits : tuple[float,float]) -> None:
    df = df[df['par_nam'].str.contains(regex, regex=True)]

    df['par_nam'] = df['par_nam'].map(labels)

    plt.figure(figsize=(7, 35))
    plt.fill_betweenx(
        y     = df['par_nam'], 
        x1    = df['pre_val'] - df['pre_err'],
        x2    = df['pre_val'] + df['pre_err'],
        label = 'Pre fit',
        alpha = 0.5,
        color = 'gray',
    )
    
    plt.errorbar(
        y     = df['par_nam'], 
        x     = df['pos_val'],
        xerr  = df['pos_err'],
        fmt   = 'o',
        label = 'Post fit',
        color = 'blue',
    )
    
    plt.axvline(x = -1, color = 'red', linestyle = ':', linewidth = 1)
    plt.axvline(x = +1, color = 'red', linestyle = ':', linewidth = 1)

    plt.axvline(x = -2, color = 'red', linestyle = ':', linewidth = 1)
    plt.axvline(x = +2, color = 'red', linestyle = ':', linewidth = 1)

    ymin, ymax = plt.ylim()
    if len(df) > 4:
        plt.ylim(ymin - 1, ymax + 2)
    else:
        plt.ylim(ymin - 1, ymax + 1)

    plt.xlabel(r'$\frac{\hat{\theta}-\theta_0}{\Delta\theta}$')
    plt.xlim(limits)
    plt.legend(loc='upper left')
    plt.show()
# ---------------------------------------
adapter = TypeAdapter(Constraint1D | ConstraintND)
pst_fit = Path('/home/acampove/external_02/fits/data/rare/900_500_all/rare/rk/ee/central/data_24/fit/brem_x12/parameters.yaml')
pre_fit = Path('/home/acampove/external_02/fits/data/rare/900_500_all/rare/rk/ee/central/constraints/all.yaml')

res = FitResult.from_json(path = pst_fit)
with open(pre_fit) as ifile:
    all_cons_data = yaml.safe_load(ifile)

data : list[dict] = []
for name, cons_data in all_cons_data.items():
    cns = adapter.validate_python(cons_data)

    if   isinstance(cns, Constraint1D):
        val = _add_entries_1D(cns = cns, res = res)
        data.append(val)
    elif isinstance(cns, ConstraintND):
        vals = _add_entries_ND(cns = cns, res = res)
        data+= vals
    else:
        raise ValueError(f'Invalid entry for: {name}')

df = pnd.DataFrame(data)
df = df.dropna(subset=['par_nam'])

_plot(regex =        '^nd_.*', df = df, limits = (-7., +7.))
_plot(regex = '^fr_.*block.*', df = df, limits = (-3., +3.))
_plot(regex =  '^fr_.*brem.*', df = df, limits = (-3., +3.))
_plot(regex =        '^mu_.*', df = df, limits = (-3., +3.))
_plot(regex =        '^sg_.*', df = df, limits = (-3., +3.))
_plot(regex =       '^yld_.*', df = df, limits = (-3., +3.))
_plot(regex =         '^s_.*', df = df, limits = (-3., +3.))
