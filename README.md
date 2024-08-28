[[_TOC_]]

# Description

This project is used to carry out checks on Run3 data.


# Filtering ntuples

## Preparing environment

First create a virtual environment with the project:

```bash
. /cvmfs/lhcb.cern.ch/lib/LbEnv
lb-conda-dev virtual-env default dcheck

pip install -e /home/acampove/Packages/RK/scripts
pip install -e /home/acampove/Packages/RK/data_checks
```

Make a tarball with the virtual environment and upload it to the grid:

```
tar -zcf dcheck.tar dcheck

dirac-dms-add-file LFN:/lhcb/user/a/acampove/run3/venv/001/dcheck.tar /home/acampove/Test/venv/dcheck.tar CERN-USER
```

## Updating code and virtual environment

If the code changes, the venv needs to change. To do that run:

```bash
update_tarball
```

in the directory where the environment (and tarball) is.

## Updating config file

The configuration and the code are separate. The configuuration file is updated with:

```bash
update_config -f /path/to/toml/file.toml
```

This script needs to be ran in a shell with access to both dirac (do `lb-dirac bash`) and with a valid grid token.
