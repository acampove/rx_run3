mva_cmb=config['mva_cmb']
mva_prc=config['mva_prc']

rule all:
    input: 
        expand(
        'results/file_{cmb}_{prc}.txt',
        cmb=mva_cmb,
        prc=mva_prc)
rule run:
    output: 'results/file_{cmb}_{prc}.txt'
    params:
        ntoys = 3,
        qsq   = 'central',
        conf  = 'rare/rkst/electron'
    shell : 
        '''
        mkdir -p results

        touch {output}

        fit_rx_rare -c {params.conf} -q {params.qsq} -C {wildcards.cmb} -P {wildcards.prc} -t toys/maker.yaml -N {params.ntoys}
        '''
