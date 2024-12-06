# Description

This project is used to:

- Filter, slim, trim the trees from a given AP production
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

currently, the latest is 023. Unless you have made your own tarballs, `LXNAME=acampove`.

## Submit jobs

To run the filtering, after properly installing the project, as shown [here](doc/install.md) do:

```bash
# Local will create a local sandbox, use wms to send to the grid

# For data, there are about 11K ROOT files in the input, 11K jobs should do one file per job, with -t, only first job will be done
job_filter -n data_test_job -p rd_ap_2024             -s       data -c /home/acampove/Packages/config_files/post_ap/v1.yaml -j 11000 -e 023 -u acampove -m local -t

# For real jobs, 1K jobs should be enough
job_filter -n data_job      -p rd_ap_2024             -s       data -c /home/acampove/Packages/config_files/post_ap/v1.yaml -j 1000 -e 023 -u acampove -m local

# For MC using noPID samples, there are only 44 input ROOT files, therefore at most 44 jobs are possible
job_filter -n mc_job        -p -btoxll_mva_2024_nopid -s simulation -c /home/acampove/Packages/config_files/post_ap/v1.yaml -j   44 -e 023 -u acampove -m local
```

where the options mean:

```bash
  -h, --help            show this help message and exit
  -n NAME  --name NAME  Name of job, needed for dirac naming and to name output
  -p PROD, --prod PROD  Name of production, e.g. rd_ap_2024, this shoudl be the same as in the config section.
  -s SAMP, --samp SAMP  Sample nickname found in the config section `samples`
  -c CONF, --conf CONF  Path to config file, which should be a YAML file and a few examples are linked below.
  -j NJOB, --njob NJOB  Number of grid jobs, this will depend on the number of files, for data typically 11K, and 1000 jobs would suffice
  -e VENV, --venv VENV  Index of virtual environment, e.g. 023
  -u USER, --user USER  User associated to venv, currently acampove should be the only choice, but if you author your own virtual environment and upload it, then this should be your user name
  -m {local,wms}, --mode {local,wms} Run locally (for tests) or in the grid
  -t       --test       If used, will send only one job
```

Regarding the name, the output will go to a directory in EOS named `JOBNAME_SAMPLENAME`.
Some config files can be found [here](https://github.com/acampove/config_files/tree/main/post_ap)

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

