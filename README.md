[[_TOC_]]

# Description

This project is used to carry out checks on Run3 data.

# Preparing environment

First set the `VENVS` environment variable:

```bash
export VENVS=/some/path/
```

where the virtual environment will reside. Then setup the LHCb software and build the tarball with the environment

```bash
. /cvmfs/lhcb.cern.ch/lib/LbEnv

update_tarball -v 001 -s 1
```

to upload it to the grid as version `001` one would run wit `-s 0`

## Updating code and virtual environment

If the code changes, the venv needs to change. To do that run:

```bash
update_tarball -v 002
```

Where `-v` will be used to pass the version of the tarball in order to upload it to the grid. 
This argument is optional and if not passed the tarball won't be uploaded.
In order to find out what versions of the tarball are in the grid do:

```bash
lb-dirac dirac-dms-user-lfns -w dcheck.tar -b /lhcb/user/a/acampove/run3/venv
```

## Updating config file

The configuration and the code are separate. The configuuration file is updated with:

```bash
update_config -u 1
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

The instructions below need to be done outside the virtual environment in an environment with access to `dirac` and in the `post_ap_grid`
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

A test would look like:

```bash
run3_download_ntuples -j flt_004 -n 3 [-d $PWD/files]
```

where:

`-j`: Is the name of the job, which has to coincide with the directory name, where the ntuples are in EOS, e.g. `/eos/lhcb/grid/user/lhcb/user/a/acampove/flt_004`.   
`-n`: Number of ntuples to download, if not pased, will download everything.    
`-d`: Directory where output ntuples will go, if not passed, directory pointed by `DOWNLOAD_NTUPPATH` will be used.


A real download would look like:

```bash
run3_download_ntuples -j flt_001 -m 40
```

Where `-m` denotes the number of threads used to download, `-j` the name of the job.

## Notes

- The downloads can be ran many times, if a file has been downloaded already, it will not be downloaded again.

# Linking and merging

Once the ntuples are downloaded these need to be linked and merged with:

```bash
link_merge -j flt_002 -v v1
```

where `-j` is the name of the job and the files are linked to a directory named as `-v v1`. For tests run:

```bash
link_merge -j flt_002 -d 1 -m 10 -v v1
```

which will do the same with at most `10` files, can use debug messages with `-l 10`.

# Making basic plots

For this run:

```bash
plot_vars -y 2024 -v v2 -c bukee_opt -d data_ana_cut_bp_ee:Data ctrl_BuToKpEE_ana_ee:Simulation
```

which will run the plotting of the variables in a config specified by `bukee_opt` where also axis, names, ranges, etc are
specified. This config is in `post_ap_data`.
The script above will overlay data and MC.

