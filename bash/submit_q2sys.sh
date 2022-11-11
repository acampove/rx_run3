#!/bin/bash

source $HOME/.bashrc
conda_init
conda activate py3p10

IJOB=$1
VERS=$2

python python/print_settings.py -i $IJOB > .settings.log

TRIG=$(cat .settings.log | grep TRIG | awk -F '=' '{print$2}')
YEAR=$(cat .settings.log | grep YEAR | awk -F '=' '{print$2}')
BREM=$(cat .settings.log | grep BREM | awk -F '=' '{print$2}')

echo "$VERS, $YEAR, $TRIG, $BREM"
python python/get_tables.py -v $VERS -y $YEAR -t $TRIG -b $BREM

