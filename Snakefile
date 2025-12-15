mva_cmb=config['mva_cmb']
mva_prc=config['mva_prc']
qsq_bin=config['qsq_bin']
ntoys  =config['ntoys']

rule all:
    input: 
        expand(
        'results/file_{qsq}_{cmb}_{prc}.txt',
        qsq=qsq_bin,
        cmb=mva_cmb,
        prc=mva_prc)
rule toys:
    output: 'results/file_{qsq}_{cmb}_{prc}.txt'
    params:
        name  = 'toys',
        ntoys = ntoys,
        conf  = 'rare/rkst/electron'
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:8da03bc8d'
    resources:
        kubernetes_memory_limit='5000Mi'
    shell : 
        '''
        source setup.sh

        mkdir -p results

        touch {output}

        fit_rx_rare -c {params.conf} -q {wildcards.qsq} -C {wildcards.cmb} -P {wildcards.prc} -t toys/maker.yaml -N {params.ntoys} -g {params.name}
        '''
