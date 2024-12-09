# Installation

## Environment with DIRAC

In order to use this project one needs access to the grid and therefore DIRAC. 
The easiest thing currently to achieve this is:

- Create an rc file with a basic environment
- Create a few functions to load this environment

### RC with environment

The file will be stored in `~/.bashrc_dirac` and will look like:

```bash
#!/usr/bin/env bash

export PS1='\u@\h\$ '
export PATH+=:$HOME/.local/bin
# This will be your username in LXPLUS
export LXNAME=acampove

# This should be the directory containing the post_ap directory, which contains YAML confg files
# needed to configure this project later
export CONFPATH=/home/acampove/Packages/config_files/
```
### Helper Functions 

In the `.bashrc` you should add:

```bash
#------------------------------------------------------------------
lb_dirac()
{
    # Will create a shell with dirac and some basic environment specified by .bashrc_dirac
    
    which lb-dirac > /dev/null 2>&1

    if [[ $? -ne 0 ]];then
        echo "Cannot find lb-dirac, LHCb software not set, setting it"
        setLbEnv
    fi

    if [[ ! -f $HOME/.bashrc_dirac ]];then
        echo "Cannnot find ~/.bashrc_dirac"
        exit 1
    fi

    lb-dirac bash -c "source ~/.bashrc_dirac && exec bash --norc"
}
#------------------------------------------------------------------
setLbEnv()
{
    # This function will setup the LHCb environment

    LBENV_PATH=/cvmfs/lhcb.cern.ch/lib/LbEnv

    if [[ ! -f $LBENV_PATH ]]; then
        echo "Cannot find $LBENV_PATH"
        kill INT $$
    fi

    . $LBENV_PATH
}
```

### Setup environment

Once this is in place, in a fresh terminal do

```bash
lhcb_dirac

# This should create a grid token 
dirac-proxy-init -v 100:00
```

this will create a shell with DIRAC, where you will work.

## Installing project

This project lives in `pypi`, install it with:

```bash
pip install post_ap

# This is the value of VENVS used to create the virtual environment that will be used
export VENVS=/afs/ihep.ac.cn/users/c/campoverde/VENVS
```

this should install the project in `$HOME/.local`.

## Updating code

If you change the code, the version of the code running in the grid will have to be updated with:

```bash
# This will skip uploading to the Grid
update_tarball

# This will build and upload to the grid
update_tarball -v 001
```

For this one needs to know the version with which to upload it. To find which versions have
already been uploaded run:

```bash
list_versions
```
