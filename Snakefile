NJOBS=25
# src/post_ap
PATHS=[
#'src/ap_utilities',
'src/dmu',
#'src/fitter',
#'src/rx_classifier',
#'src/rx_data',
#'src/rx_efficiencies',
#'src/rx_misid',
#'src/rx_pid',
#'src/rx_plots',
#'src/rx_q2',
#'src/rx_selection',
]

rule all:
    input: expand("results/group_{index}.xml", index=range(1, NJOBS + 1))

rule test:
    output:
        'results/group_{index}.xml'
    params:
        path   = ' '.join(PATHS),
        ngroups= NJOBS
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:435c74635'
    resources:
        kubernetes_memory_limit="4000Mi"
    shell:
        """
        source setup.sh

        pytest {params.path} --splits {params.ngroups} --group {wildcards.index} --junitxml={output} --splitting-algorithm=least_duration
        """
