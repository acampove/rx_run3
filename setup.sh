#!/usr/bin/env bash

set -euo pipefail

check_env()
{
    FAIL=0

    if ! command -v python &>/dev/null;then
        echo "Failed to find python"
        FAIL=1
    else
        echo "Found python at $(which python)"
    fi

    if ! command -v root   &>/dev/null;then
        echo "Failed to find ROOT"
        FAIL=1
    else
        echo "Found ROOT at $(which root)"
    fi

    if [[ $FAIL -ne 0 ]];then
        echo "Failed to find ROOT/Python"
        exit 1
    fi
}

export ZFIT_DISABLE_TF_WARNINGS=1
export MPLCONFIGDIR=/tmp/rx/mplt
export USER=rx_run3

# These variables are needed in China
# unset them for runs in REANA/Condor, etc
unset  https_proxy
unset  http_proxy
unset  HTTP_PROXY 
unset  HTTPS_PROXY 

source /root_install/bin/thisroot.sh
check_env
