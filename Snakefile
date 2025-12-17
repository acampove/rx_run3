mva_cmb  = config['mva_cmb']
mva_prc  = config['mva_prc']
qsq_bin  = config['qsq_bin']
ntoys    = config['ntoys'  ]

conf_val = 'rare/rkst/electron'
kind_val = 'rare_rkst_electron'

out_paths = ['/home/acampove/plots']
names     = ['scan']

rule all:
    input: 
        expand(
            f'{{out_path}}/{{name}}_summary/{kind_val}/{{qsq}}/{{cmb}}_{{prc}}.png',
            out_path = out_paths,
            name     = names,
            cmb      = mva_cmb,
            prc      = mva_prc,
            qsq      = qsq_bin)
rule collect:
    input : f'{{out_path}}/{{name}}/{{cmb}}_{{prc}}_all/{conf_val}/data/{{qsq}}/brem_x12/fit_linear.png'
    output: f'{{out_path}}/{{name}}_summary/{kind_val}/{{qsq}}/{{cmb}}_{{prc}}.png'
    wildcard_constraints:
        cmb = r'\d{3}', 
        prc = r'\d{3}', 
        qsq = '[a-z]+', 
        name= '[a-z]+'
    shell:
        '''
        cp {input} {output}
        '''
rule toys:
    output: f'{{out_path}}/{{name}}/{{cmb}}_{{prc}}_all/{conf_val}/data/{{qsq}}/brem_x12/fit_linear.png'
    wildcard_constraints:
        cmb   = r'\d{3}', 
        prc   = r'\d{3}', 
        qsq   = '[a-z]+', 
        name  = '[a-z]+'
    params:
        ntoys = ntoys,
        conf  = conf_val
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:f19be3238'
    resources:
        kubernetes_memory_limit='5000Mi'
    shell : 
        '''
        #source setup.sh

        #fit_rx_rare -c {params.conf} \
                    -q {wildcards.qsq} \
                    -C {wildcards.cmb} \
                    -P {wildcards.prc} \
                    -t toys/maker.yaml \
                    -N {params.ntoys} \
                    -g {wildcards.name} || true
        
        rxfitter make-dummy-plot -p {output} -t {wildcards.cmb}_{wildcards.prc}
        '''
