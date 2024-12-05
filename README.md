# Description

This project is used to:

- Filter, slim, trim the trees files from a given AP production
- Rename branches
- Download the outputs

This is done using configurations in a YAML file and through DIRAC jobs.

Check [this](doc/install.md) for installation instructions
and for instructions on how to setup an environment to use this project.

# Submitting jobs

## Check latest version of virtual environment

All the jobs below require code that lives in a virtual environment, there should be multiple versions of this
environment and the latest one should be obtained by running:

```bash
dirac-dms-user-lfns -w dcheck.tar -b /lhcb/user/${LXNAME:0:1}/$LXNAME/run3/venv
```

currently, the latest. Unless you have made your own tarballs, `LXNAME=acampove`.

## Submit jobs

Run a test job with:

```bash
job_filter -d dt_2024_turbo -c comp -j 1211 -e 003 -m local -n test_flt -u acampove
```

where `-u` specifies the user who authored the environment that the job will use.
The flag `-j` specifies the number of jobs. For tests, this is the number of files to process, thus, the test job does only one file.
The `-n` flag is the name of the job, for tests it will do/send only one job if either:

1. Its name has the substring `test`.
1. It is a local job.

Thus one can do local or grid tests running over a single file.

For real jobs:

```bash
job_filter -d dt_2024_turbo -c comp -j 200 -e 003 -m wms -n flt_001 -u acampove
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


