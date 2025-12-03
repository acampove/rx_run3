#!/usr/bin/env bash

# This pre-push hook is meant to swap xxx in:
#
# gitlab-registry.cern.ch/lhcb-rd/cal-rx-run3:XXX
#
# with the SHA of the current commit. This is meant to be done in the Snakefile

set -euo pipefail

REMOTE=$1
BRANCH=$(git branch --show-current)

if [[ -f .already_renamed ]];then
    echo "SHA was already updated, pushing to $REMOTE/$BRANCH"
    rm .already_renamed
    exit 0
fi

echo "-----------------"
echo "Running SHA replacement on push to: $REMOTE/$BRANCH"

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

SHA=$(git rev-parse --short HEAD)

echo $SHA > .already_renamed

sed -i -E "s|:[a-f0-9]{9}'|:$SHA'|"            Snakefile
sed -i -E "s|_IMAGE:[a-f0-9]{9}|_IMAGE:$SHA|" .gitlab-ci.yml

git add Snakefile
git add .gitlab-ci.yml

git commit -m "Update image version with $SHA"
git push $REMOTE $BRANCH
