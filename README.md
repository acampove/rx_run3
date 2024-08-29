[[_TOC_]]

# Description

This project is used to carry out checks on Run3 data.

# Preparing environment

First create a virtual environment with the project:

```bash
. /cvmfs/lhcb.cern.ch/lib/LbEnv
lb-conda-dev virtual-env default dcheck

pip install -e /home/acampove/Packages/RK/scripts
pip install -e /home/acampove/Packages/RK/data_checks
```

Make a tarball with the virtual environment and upload it to the grid:

```bash
tar -zcf dcheck.tar dcheck

dirac-dms-add-file LFN:/lhcb/user/a/acampove/run3/venv/001/dcheck.tar /home/acampove/Test/venv/dcheck.tar CERN-USER
```

## Updating code and virtual environment

If the code changes, the venv needs to change. To do that run:

```bash
update_tarball
```

in the directory where the environment (and tarball) is. The new tarball will need to be sent to the grid again with:

```bash
dirac-dms-add-file LFN:/lhcb/user/a/acampove/run3/venv/002/dcheck.tar /home/acampove/Test/venv/dcheck.tar CERN-USER
```

## Updating config file

The configuration and the code are separate. The configuuration file is updated with:

```bash
update_config -f /home/acampove/Packages/RK/data_checks/src/data_checks_data/dt_2024_turbo_004.toml -u 1
```

The `-u` flag will update the config file if its LFN is alrdeady in the grid.
The script runs with:

1. The LHCb environment set up.
1. With a valid grid token.
1. Within the working virtual environment. 
`lb-dirac` and the script need to be used. No conflict between the VENV and the LHCb environments seems to happen.

# Save lists of PFNs

The PFNs to be processed will be stored once with the AP api and will be read as package data when processing ntuples. 
The list of PFNs is created with, e.g.:

```bash
save_pfns -c dt_2024_turbo_comp
```

where `-c` will correspond to the config file.

# Submitting jobs

The instructions below need to be done outside the virtual environment in an environment with access to `dirac` and in the `data_checks_grid`
directory.

First run a test job with:

```bash
./job_filter -d dt_2024_turbo -c comp -j 1211 -e 003 -m local -n test_flt
```

where `-j` specifies the number of jobs. For tests, this is the number of files to process, thus, the test job does only one file. 
The `-n` flag is the name of the job, for tests it will do/send only one job if either:

1. Its name has the substring `test`.
1. It is a local job.

Thus one can do local or grid tests running over a single file.

For real jobs:

```bash
./job_filter -d dt_2024_turbo -c comp -j 200 -e 003 -m wms -n flt_001
```

# Downloading ntuples

Run:

```bash
run3_download_ntuples -j flt_27_08_2024_dt_2024_turbo -n 3 -d $PWD/files
```

in an environment with a valid grid token. `-j` is the name of the job and the downloaded will download `3` files.
