#!/bin/bash

#--------------------------
link()
{
    SOURCE=$1
    TARGET=$2

    rm -f $TARGET

    ln -s $SOURCE $TARGET
}
#--------------------------
source $HOME/.bashrc
conda_init
conda activate py3p10

python python/print_settings.py -i 0 > .settings.log
NJOB=$(cat .settings.log | grep NJOB | awk -F '=' '{print$2}')

JOBDIR=/publicfs/ucas/user/campoverde/Jobs/q2sys
DATE=$(date | sed "s|\s|_|g" | sed "s|:|_|g")
JOBDIR=$JOBDIR"_"$DATE
mkdir -p $JOBDIR
link $PWD/bash    $JOBDIR/bash
link $PWD/cached  $JOBDIR/cached
link $PWD/plots   $JOBDIR/plots
link $PWD/python  $JOBDIR/python
cd       $JOBDIR

OFILE=q2syst_%{ClusterId}_%{ProcId}
MEMORY=4000
VERS=v2

echo "NJOB=$NJOB"

hep_sub -n $NJOB -g lhcb -e $OFILE".err" -o $OFILE".out" -argu %{ProcId} $VERS -mem $MEMORY bash/submit_q2sys.sh

