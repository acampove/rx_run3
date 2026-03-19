import os

mva_cmb  = config['mva_cmb']
mva_prc  = config['mva_prc']
qsq_bin  = config['qsq_bin']
project  = config['project']
channel  = config['channel']
# -------------
ANADIR    = os.environ['ANADIR']
out_path  = f'{ANADIR}/fits/data'
group_name= 'rare'
# ---------------------------------------
def _get_path(cmb, prc, prj, chn, qsq):
    conf     = f'rare/{prj}/{chn}'

    if   chn == 'ee':
        brem = 'brem_x12'
    elif chn == 'mm':
        brem = 'brem_xx0'
    else:
        raise ValueError(f'Invalid channel: {chn}')

    return f'{out_path}/{group_name}/{cmb}_{prc}_all/{conf}/{qsq}/data_24/fit/{brem}/{brem}/fit_linear.png'
# ---------------------------------------
paths = []
for qsq in qsq_bin:
    for cmb in mva_cmb:
        for prc in mva_prc:
            for prj in project:
                for chn in channel:
                    path = _get_path(cmb, prc, prj, chn, qsq)
                    paths.append(path)
# ---------------------------------------
rule all:
    input: paths
# ---------------------
rule fits:
    output: f'{out_path}/{group_name}/{{cmb}}_{{prc}}_all/{{conf}}/{{qsq}}/data_24/fit/{{brem}}/{{brem}}/fit_linear.png',
    log:     f'{out_path}/{group_name}/{{cmb}}_{{prc}}_all/{{conf}}/logs/{{qsq}}_{{brem}}.log',
    wildcard_constraints:
        cmb   = r'\d{3}',
        prc   = r'\d{3}',
        qsq   = '[a-z]+',
    params:
        group_name = group_name,
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:6d4387847'
    resources:
        kubernetes_memory_limit='5000Mi'
    shell :
        '''
        source setup.sh
        CMB_WP=$(rxfitter wp-translator -w {wildcards.cmb})
        PRC_WP=$(rxfitter wp-translator -w {wildcards.prc})
        fit_rx_rare \
                    -b -1              \
                    -g {params.group_name}   \
                    -c {wildcards.conf}\
                    -q {wildcards.qsq} \
                    -C $CMB_WP         \
                    -P $PRC_WP         \
                    > {log} 2>&1        \
                    || true
        '''
# ---------------------
