mva_cmb  = config['mva_cmb']
mva_prc  = config['mva_prc']
qsq_bin  = config['qsq_bin']

ntoys    = config['ntoys'  ]
conf_val = 'rare/rkst/electron'
kind_val = 'rare_rkst_electron'
out_path = '.eos/lhcb/wg/RD/RX_run3/fits/data'
name     = 'scan_003'

# ---------------------
rule all:
    input: 
        expand(
            out_path + '/' + name + '/' + '{cmb}_{prc}_all/' + conf_val + '/data/{qsq}/brem_x12/fit_linear.png',
            cmb      = mva_cmb,
            prc      = mva_prc,
            qsq      = qsq_bin)
# ---------------------
rule toys:
    output: out_path + '/' + name + '/' + '{cmb}_{prc}_all/' + conf_val + '/data/{qsq}/brem_x12/fit_linear.png'
    wildcard_constraints:
        cmb   = r'\d{3}', 
        prc   = r'\d{3}', 
        qsq   = '[a-z]+', 
    params:
        name  = name,
        ntoys = ntoys,
        conf  = conf_val,
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:baa650de2'
    resources:
        kubernetes_memory_limit='5000Mi'
    shell : 
        '''
        source setup.sh

        fit_rx_rare -c {params.conf}   \
                    -q {wildcards.qsq} \
                    -C {wildcards.cmb} \
                    -P {wildcards.prc} \
                    -t toys/maker.yaml \
                    -N {params.ntoys}  \
                    -g {params.name} || true

        REMOTE=$(echo {output} | sed 's/\.eos/\/eos/g')

        rxfitter make-dummy-plot -p $REMOTE  -t {wildcards.cmb}_{wildcards.prc}

        mkdir -p $(dirname {output})
        cp $REMOTE {output}
        '''
