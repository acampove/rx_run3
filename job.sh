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
JOBKIND=$1
VERS=$2

source $HOME/.bashrc
conda_init
conda activate py3p10

python python/print_settings.py -i 0 -d > .settings.log

NJOB=$(cat .settings.log | grep NJOB | awk -F '=' '{print$2}')

JOBDIR=/publicfs/ucas/user/campoverde/Jobs/$JOBKIND
DATE=$(date | sed "s|\s|_|g" | sed "s|:|_|g")
JOBDIR=$JOBDIR"_"$DATE
mkdir -p $JOBDIR

CDIR=$(pwd -P)
link $CDIR/bash    $JOBDIR/bash
link $CDIR/cached  $JOBDIR/cached
link $CDIR/plots   $JOBDIR/plots
link $CDIR/python  $JOBDIR/python
link $CDIR/output  $JOBDIR/output
cd $JOBDIR

OFILE=$JOBKIND_%{ClusterId}_%{ProcId}
MEMORY=4000

echo "NJOB=$NJOB"

if   [[ "$JOBKIND" == "q2sys" ]];then
    SYST=$3
    hep_sub -n $NJOB -g lhcb -e $OFILE".err" -o $OFILE".out" -argu %{ProcId} $VERS $SYST -mem $MEMORY bash/submit_q2sys.sh
elif [[ "$JOBKIND" == "q2mom" ]];then
    ISSM=$3
    hep_sub -n $NJOB -g lhcb -e $OFILE".err" -o $OFILE".out" -argu %{ProcId} $VERS $ISSM -mem $MEMORY bash/submit_q2mom.sh
else
    echo "Invalid job type: $JOBKIND"
    kill -INT $$
fi

