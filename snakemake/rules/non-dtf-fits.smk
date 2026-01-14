import os

ANADIR = os.environ['ANADIR']
WP     = [
    '050_060',     # Loose SR 
    '070_060',     # Representative of SR 
    '090_060',     # Tight SR 
    '010-050_080', # Check combinatorial
    ]
BLOCK  = list(range(1, 9))[:1]
NAME   = 'reso_non_dtf'
# ---------------------
rule all:
    input: 
        expand(
            [ANADIR + '/fits/data/' + NAME + '/' + '{wp}_b{block}/reso/rk/electron/data/jpsi/brem_001/fit_linear.png',
             ANADIR + '/fits/data/' + NAME + '/' + '{wp}_b{block}/reso/rk/electron/data/jpsi/brem_002/fit_linear.png'],
            wp    = WP,
            block = BLOCK)
# ---------------------
rule fits:
    output   : 
        ANADIR + '/fits/data/' + NAME + '/' + '{wp}_b{block}/reso/rk/electron/data/jpsi/brem_001/fit_linear.png',
        ANADIR + '/fits/data/' + NAME + '/' + '{wp}_b{block}/reso/rk/electron/data/jpsi/brem_002/fit_linear.png',
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:6d4387847'
    params:
        name = NAME
    resources:
        kubernetes_memory_limit='4000Mi'
    shell :
        '''
        source setup.sh

        fit_rx_reso -b {wildcards.block}\
                    -g {params.name}\
                    -c reso/rk/electron/data_non_dtf.yaml \
                    -q jpsi \
                    -C $(rxfitter wp-translator -w {wildcards.wp} -k cmb)\
                    -P $(rxfitter wp-translator -w {wildcards.wp} -k prc) || true
        '''
