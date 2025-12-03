#!/usr/bin/env bash

# This hook is meant to swap xxx in:
#
# gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:XXX
#
# with the SHA of the current commit. This is meant to be done in the Snakefile

set -euo pipefail

echo "Running SHA replacement"

if [[ ! -f Snakefile ]];then
    echo "Missing Snakefile"
    exit 1
else
    echo "Will change Snakefile's image tag"
fi

SHA=$(git rev-parse --short HEAD)

echo $SHA > file.txt
