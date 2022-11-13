#!/bin/bash

source $HOME/.bashrc
conda_init
conda activate py3p10

IJOB=$1
VERS=$2
CACHE_NAME=settings_$IJOB.txt

python python/print_settings.py -i $IJOB > $CACHE_NAME 

TRIG=$(cat $CACHE_NAME | grep TRIG | awk -F '=' '{print$2}')
YEAR=$(cat $CACHE_NAME | grep YEAR | awk -F '=' '{print$2}')
BREM=$(cat $CACHE_NAME | grep BREM | awk -F '=' '{print$2}')

echo "$VERS, $YEAR, $TRIG, $BREM"
python python/get_tables.py -v $VERS -y $YEAR -t $TRIG -b $BREM

