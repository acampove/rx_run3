#!/usr/bin/env bash

# This pre-commit hook is meant to swap xxx in:
#
# gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:XXX
#
# with the SHA of the current commit. This is meant to be done in the Snakefile

set -euo pipefail

SHA=$(git rev-parse --short HEAD)

echo "-----------------"
echo "Found SHA: $SHA"
echo ""
if [[ ! -f Snakefile ]];then
    echo "Missing Snakefile"
    exit 1
else
    echo "Will change Snakefile's image tag"
fi

if [[ ! -f .gitlab-ci.yml ]];then
    echo "Missing gitlab CI/CD file"
    exit 1
else
    echo "Will change CI/CD image tag"
fi
echo "-----------------"

sed -i -E "s|:[a-f0-9]{9}'|:$SHA'|"            Snakefile
sed -i -E "s|_IMAGE:[a-f0-9]{9}|_IMAGE:$SHA|" .gitlab-ci.yml

git add Snakefile
git add .gitlab-ci.yml
