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

The jobs below will run with code from a virtual environment that is already in the grid. One should use the
latest version of this environment. To know the latest versions, run:

```bash
list_venvs
```

Unless you have made your own tarballs: 

```bash
export LXNAME=acampove
```

should have been ran before using `list_venvs`.

## Submit jobs

To run the filtering, after properly installing the project, as shown [here](doc/install.md) do:

```bash
# Local will create a local sandbox, use wms to send to the grid

# For data, this will process a single PFN locally
job_filter -n test_job -p rd_ap_2024 -s       data -c /home/acampove/Packages/config_files/post_ap/v3.yaml -e 025 -u acampove -m local -t

# For data, this will process all the PFNs in the grid 
job_filter -n data_job -p rd_ap_2024 -s       data -c /home/acampove/Packages/config_files/post_ap/v3.yaml -e 025 -u acampove -m wms

# For MC, this will process all the PFNs in the grid 
job_filter -n mc_job   -p rd_ap_2024 -s simulation -c /home/acampove/Packages/config_files/post_ap/v3.yaml -e 025 -u acampove -m wms
```

where the options mean:

```bash
  -h, --help            show this help message and exit
  -n NAME  --name NAME  Name of job, needed for dirac naming and to name output
  -p PROD, --prod PROD  Name of production, e.g. rd_ap_2024, this shoudl be the same as in the config section.
  -s SAMP, --samp SAMP  Sample nickname found in the config section `samples`
  -c CONF, --conf CONF  Path to config file, which should be a YAML file and a few examples are linked below.
  -e VENV, --venv VENV  Index of virtual environment, e.g. 023
  -u USER, --user USER  User associated to venv, currently acampove should be the only choice, but if you author your own virtual environment and upload it, then this should be your user name
  -d DRYR, --dryr DRYR  If used, submission will be skipped, needed for debugging.
  -M MAXJ, --maxj MAXJ  Maximum number of jobs, default 500. If 1000 PFNs are found, will do 500 jobs, if 100 PFNs are found, will do 100 jobs
  -m {local,wms}, --mode {local,wms} Run locally (for tests) or in the grid
  -t       --test       If used, will send only one job
```

Regarding the name, the output will go to a directory in EOS named `JOBNAME_SAMPLENAME`, e.g. `test_001_data` if
`-n test_001` is used on the data sample.
Some config files can be found [here](https://github.com/acampove/config_files/tree/main/post_ap)

# Downloading ntuples

A test would look like:

```bash
run3_download_ntuples -j dec_06_2024_data -n 20 -r 1 -m 5 [-d $PWD/files]
```

where:

`-j`: Is the name of the job, which has to coincide with the directory name, where the ntuples are in EOS, e.g. `/eos/lhcb/grid/user/lhcb/user/a/acampove/flt_004`.   
`-n`: Number of ntuples to download, if not pased, will download everything.   
`-d`: Directory where output ntuples will go, if not passed, directory pointed by `DOWNLOAD_NTUPPATH` will be used.   

A real download would look like:

```bash
run3_download_ntuples -j dec_06_2024_data -m 40
```

Where `-m` denotes the number of threads used to download, `-j` the name of the job.

# Removing old outputs

If outputs of old jobs need to be removed, it can be done with:

```bash
remove_job -n job_name -s sample_name
```

from the examples above this could look like:

```bash
remove_job -n dec_08_2024 -s simulation 
```

