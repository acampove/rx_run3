NJOBS=40
TEST_PATH='$PWD/src/rx_data'

rule all:
    input:
        expand(".test_{index}.txt", index=range(NJOBS))

rule test:
    output:
        ".test_{index}.txt"
    params:
        path   = TEST_PATH,
        ngroups= NJOBS
    container:
        'gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:d5acfd033'
    resources:
        kubernetes_memory_limit="4000Mi"
    shell:
        """
        source setup.sh

        cmd=$(cmd_from_index -i "{wildcards.index}" -n "{params.ngroups}" -p "{params.path}")

        > {output} 
        for cmd in $(jq -r '.[]' .commands_{wildcards.index}.json); do
            pytest "$cmd"
            echo "$cmd" >> {output} 
        done
        """
