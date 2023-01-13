from rk.dilep_reso import plot_reso as rk_plot_reso

import utils_noroot as utnr
#----------------------------
class data:
    version = 'v1'
    l_kind  = ['data', 'mc']
    l_brem  = [ 0, 1, 2 ]
    l_src   = ['par', 'res']
    d_prange= { 0 : ( 10,  35), 1 : ( 20,  60), 2 : ( 30,  65) }
    d_rrange= { 0 : (120, 155), 1 : (130, 165), 2 : (115, 175) }

    plot_dir = utnr.make_dir_path(f'output/resolution/plots/{version}')
    log      = utnr.getLogger(__name__)
#----------------------------
def par_to_res(d_par):
    d_res = {}
    for sbin, d_fit_par in d_par.items():
        if 'inf' in sbin:
            continue

        [sg, _] = d_fit_par['sg']

        d_res[sbin] = sg

    return d_res
#----------------------------
def plot_reso(kind, brem, src):
    par_path  = f'output/resolution/{kind}/json/{data.version}/{src}_brem_{brem}.json'
    d_par_str = utnr.load_json(par_path)

    if src == 'par':
        d_res_str = par_to_res(d_par_str)
        zrange    = data.d_prange[brem]
    else:
        d_res_str = d_par_str
        zrange    = data.d_rrange[brem]

    rk_plot_reso(d_res_str, data.plot_dir, suffix=f'{kind}_{brem}_{src}', title=f'Brem {brem}; From: {src},{kind}', rng=zrange)
#----------------------------
def plot_ratio(brem, src):
    json_path = f'output/resolution/ratio/{data.version}/{src}_brem_{brem}.json'
    d_rat_str = utnr.load_json(json_path)
    zrange    = (0, 2)

    rk_plot_reso(d_rat_str, data.plot_dir, suffix=f'rat_{brem}_{src}', title=f'Brem {brem}; From: {src}, ratio', rng=zrange)
#----------------------------
def plot_all_reso():
    for kind in data.l_kind:
        for brem in data.l_brem:
            for src in data.l_src:
                plot_reso(kind, brem, src)
#----------------------------
def plot_all_ratios():
    for brem in data.l_brem:
        for src in data.l_src:
            plot_ratio(brem, src)
#----------------------------
def main():
    plot_all_reso()
    plot_all_ratios()
#----------------------------
if __name__ == '__main__':
    main()

