mva_cmb  = config['mva_cmb']
mva_prc  = config['mva_prc']
qsq_bin  = config['qsq_bin']
ntoys    = config['ntoys']
conf     = ['rare/rkst/electron']
eos_path = ['/eos/lhcb/wg/RD/RX_run3']
name     = ['scan']

rule all:
    input: 
        expand(
        '{eos_path}/{conf}/{name}/{cmb}_{prc}_{qsq}.png',
        eos_path = eos_path,
        conf     = conf,
        name     = name,
        cmb      = mva_cmb,
        prc      = mva_prc,
        qsq      = qsq_bin)

rule collect:
    input : '{eos_path}/fits/data/{name}/{cmb}_{prc}_all/{conf}/data/{qsq}/brem_x12/fit_linear.png'
    output: '{eos_path}/{conf}/{name}/{cmb}_{prc}_{qsq}.png'
    wildcard_constraints:
        cmb ='\d{3}' ,
        prc ='\d{3}' ,
        qsq ='[a-z]+',
        name='[a-z]+',
    shell:
        '''
        cp {input} {output}
        '''
rule toys:
    output: '{eos_path}/fits/data/{name}/{cmb}_{prc}_all/{conf}/data/{qsq}/brem_x12/fit_linear.png'
    wildcard_constraints:
        cmb ='\d{3}' ,
        prc ='\d{3}' ,
        qsq ='[a-z]+',
        name='[a-z]+',
    params:
        ntoys = ntoys
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:e47449f1c'
    resources:
        kubernetes_memory_limit='5000Mi'
    shell : 
        '''
        source setup.sh

        #fit_rx_rare -c {wildcards.conf} -q {wildcards.qsq} -C {wildcards.cmb} -P {wildcards.prc} -t toys/maker.yaml -N {params.ntoys} -g {wildcards.name} || true
        rxfitter make-dummy-plot -p {output} -t {wildcards.cmb}"_"{wildcards.prc}
        '''
