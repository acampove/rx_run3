NJOBS=4
#'src/post_ap'
#'src/rx_pid',
PATHS=[
'src/ap_utilities',
#'src/dmu',
#'src/fitter',
#'src/rx_classifier',
#'src/rx_data',
#'src/rx_efficiencies',
#'src/rx_misid',
#'src/rx_plots',
#'src/rx_q2',
#'src/rx_selection',
]

rule all:
    input: 'report.txt'
rule test:
    output:
        'results/group_{index}.xml'
    params:
        path   = ' '.join(PATHS),
        ngroups= NJOBS,
        tst_dir= './temporary'
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:3dd5bb8ab'
    resources:
        kubernetes_memory_limit='4000Mi'
    shell:
        '''
        source setup.sh

        mkdir -p {params.tst_dir}

        pytest {params.path} --basetemp {params.tst_dir} --splits {params.ngroups} --group {wildcards.index} --junitxml={output} --splitting-algorithm=least_duration || true
        '''
rule report:
    input    : expand('results/group_{index}.xml', index=range(1, NJOBS + 1))
    output   : 'report.txt'
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:3dd5bb8ab'
    resources:
        kubernetes_memory_limit='1000Mi'
    shell:
        '''
        source setup.sh

        dmu check-jobs-status -f results
        '''
