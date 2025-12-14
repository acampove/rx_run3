mva_cmb=config['mva_cmb']
mva_prc=config['mva_prc']
ntoys  =config['ntoys']

rule all:
    input: 
        expand(
        'results/file_{cmb}_{prc}.txt',
        cmb=mva_cmb,
        prc=mva_prc)
rule run:
    output: 'results/file_{cmb}_{prc}.txt'
    params:
        name  = 'test_001',
        ntoys = ntoys,
        qsq   = 'central',
        conf  = 'rare/rkst/electron'
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:8da03bc8d'
    shell : 
        '''
        source setup.sh

        mkdir -p results

        touch {output}

        fit_rx_rare -c {params.conf} -q {params.qsq} -C {wildcards.cmb} -P {wildcards.prc} -t toys/maker.yaml -N {params.ntoys} -g {params.name}
        '''
