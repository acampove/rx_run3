# $R_X$ common

The purpose of this project is to:

- Offer a thin layer of python code meant to interface and test the $R_X$ run3
[gitlab](https://gitlab.cern.ch/LHCb-RD/cal-rx-run3) tools, written in C++ and adapted from the Run1/2 code.

- Store new tools, needed for the Run3 analyses, that can be shared among run3 $R_X$ analyses.

## Installation

It is **strongly** recommended to use conda/mamba/micromamba to create a virtual environment where the code will go.
Instructions for micromamba can be found
[here](https://mamba.readthedocs.io/en/latest/installation/micromamba-installation.html).

The code is split into multiple projects, each doing a specific study and all belonging to a given group.
The code is available as a set of packages that can be cloned and installed through with:

```bash
# Define a path in your computer/cluster where the code will be clonned
# This should be placed in your .bashrc
export SFTDIR=$HOME/run3_rx

git clone ssh://git@gitlab.cern.ch:7999/rx_run3/rx_common.git
# install
pip install -e ./rx_common/

# This will install all the packages in your system, in editable mode
rx_setup -k sync -i 1 -f projects.txt
```

Where `projects.txt` is the file with the list of branches and project names to run over, e.g.:

```
master rx_classifier
master rx_calibration
master rx_selection
master rx_common
master rx_data
master post_ap
master dmu
```

The `sync` flag will make sure that:

- All the remote changes are pulled to your local branch.
- The code gets reinstalled.
- All your local changes have been committed.
- All the changes you committed locally are pushed to whatever branch you specified.

Thus, making it easier to synchronize your local changes with the remote ones.
To check if the project you will run over are the right ones, one can run a dry run with the `-d 1` flag:

```bash
rx_setup -k sync -i 1 -f projects.txt -d 1
```
