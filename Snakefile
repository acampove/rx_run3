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
    shell : 
        '''
        mkdir -p results

        touch {output}
        '''
