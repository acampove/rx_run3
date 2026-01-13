import os

ANADIR = os.environ['ANADIR']
WP     = '070_060'
BREM   = ['001', '002']
BLOCK  = list(range(1, 9))[:1]
NAME   = 'reso_non_dtf'
# ---------------------
rule all:
    input: 
        expand(
            ANADIR + '/fits/data/' + NAME + '/' + WP + '_b{block}/reso/rk/electron/data/jpsi/brem_{brem}/fit_linear.png',
            brem  = BREM,
            block = BLOCK)
# ---------------------
rule fits:
    output   : ANADIR + '/fits/data/' + NAME + '/' + WP + '_b{block}/reso/rk/electron/data/jpsi/brem_{brem}/fit_linear.png'
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
                    -C 0.70 \
                    -P 0.60 || true
        '''
