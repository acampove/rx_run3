#!/usr/bin/env bash

# This pre-commit hook is meant to swap xxx in:
#
# gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:XXX
#
# with the SHA of the current commit. This is meant to be done in the Snakefile

set -euo pipefail

BRANCH=$(git branch --show-current)
SHA=$(git rev-parse --short HEAD)

echo "-----------------"
echo "Found SHA: $SHA"
echo ""

if [[ ! -f .gitlab-ci.yml ]];then
    echo "Missing gitlab CI/CD file"
    exit 1
else
    echo "Will change CI/CD image tag"
fi
echo "-----------------"

sed -i -E "s|_IMAGE:[a-f0-9]{9}|_IMAGE:$SHA|"             .gitlab-ci.yml
sed -i -E "s|cal-rx-run3:[a-f0-9]{9}'|cal-rx-run3:$SHA'|" snakemake/rules/*.smk 

git update-index snakemake/rules/*.smk
git update-index .gitlab-ci.yml
