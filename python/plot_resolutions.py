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

    plot_dir = utnr.make_dir_path('output/resolution/plots')
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
def main():
    for kind in data.l_kind:
        for brem in data.l_brem:
            for src in data.l_src:
                plot_reso(kind, brem, src)
#----------------------------
if __name__ == '__main__':
    main()

