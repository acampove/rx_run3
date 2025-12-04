NJOBS=20
#TEST_PATH="src/rx_selection"
#TEST_PATH="src/rx_classifier"
#TEST_PATH="src/rx_data"
#TEST_PATH="src/rx_efficiencies"
TEST_PATH="src/rx_plots src/rx_q2 src/rx_misid"

rule all:
    input: expand("results/group_{index}.xml", index=range(1, NJOBS + 1))

rule test:
    output:
        'results/group_{index}.xml'
    params:
        path   = TEST_PATH,
        ngroups= NJOBS
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:b9615debb'
    resources:
        kubernetes_memory_limit="4000Mi"
    shell:
        """
        source setup.sh

        pytest {params.path} --splits {params.ngroups} --group {wildcards.index} --junitxml={output} --splitting-algorithm=least_duration
        """
