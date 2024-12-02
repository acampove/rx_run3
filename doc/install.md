# Installation

In order to use this project one needs access to the grid and therefore DIRAC. For this do:

```bash
. /cvmfs/lhcb.cern.ch/lib/LbEnv

# Open a new shell, let's call it, dirac_shell
lb-dirac

# Make sure that machine will find locally installed binaries
export PATH+=:$HOME/.local/bin
```

to setup the LHCb software, then:

```bash
# From withint he dirac_shell
dirac-proxy-init -v 100:00
```

to make a token that lasts 100 hours.

## For users

This project lives in `pypi`, install it with:

```bash
pip install post_ap
```

this should install the project in `$HOME/.local`, given that we are not working in a virtual environment or conda.

## Development 

Once the code is ready to be used, i.e. it has been tested locally one needs to pack it into a tarball and upload it
to the grid. First set the `VENVS` environment variable:

```bash
export VENVS=/some/path/
export LXNAME=$USER # This is the username when running in LXPLUS
```

where the virtual environment will reside. Then setup the LHCb software and build the tarball with the environment

```bash
# This will skip uploading to the Grid
update_tarball -v 001 -s 1

# This will build and upload to the grid 
update_tarball -v 001
```

to upload it to the grid. For this one needs to know the version with which to upload it. To find which versions have
already been uploaded run:

```bash
lb-dirac dirac-dms-user-lfns -w dcheck.tar -b /lhcb/user/${LXNAME:0:1}/$LXNAME/run3/venv
```

this will dump a text file with the list of LFNs in that particular path. Then one would pick the next name in the list.
