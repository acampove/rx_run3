#!/bin/bash

source $HOME/.bashrc
conda_init
conda activate py3p10

IJOB=$1
VERS=$2
ISSM=$3
CACHE_NAME=settings_$IJOB.txt

python python/print_settings.py -d -i $IJOB > $CACHE_NAME 

DSET=$(cat $CACHE_NAME | grep YEAR | awk -F '=' '{print$2}')
TRIG=$(cat $CACHE_NAME | grep TRIG | awk -F '=' '{print$2}')
BREM=$(cat $CACHE_NAME | grep BREM | awk -F '=' '{print$2}')

echo "$DSET, $TRIG, $BREM, $VERS, $ISSM"

if [[ $ISSM -eq 1 ]];then
    echo "Doing simulation"
    python python/get_resolutions.py -d $DSET -t $TRIG -b $BREM -v $VERS -s
else
    echo "Doing data"
    python python/get_resolutions.py -d $DSET -t $TRIG -b $BREM -v $VERS
fi

