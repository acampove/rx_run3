# $R_X$ common

The purpose of this project is to:

- Offer a thin layer of python code meant to interface and test the $R_X$ run3
[gitlab](https://gitlab.cern.ch/LHCb-RD/cal-rx-run3) tools, written in C++ and adapted from the Run1/2 code.

- Store new tools, needed for the Run3 analyses, that can be shared among run3 $R_X$ analyses.

## Installing and the code

The code is split into multiple projects, each doing a specific study and all belonging to a given group.
The code is available as a set of packages that can be cloned and installed through with:

```bash
# Define a path in your computer/cluster where the code will be clonned
# This should be placed in your .bashrc 
export SFTDIR=$HOME/run3_rx

git clone ssh://git@gitlab.cern.ch:7999/rx_run3/rx_common.git
# install
pip install rx_common 

rx_setup -k sync -i 1
```
