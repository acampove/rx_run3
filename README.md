# Description

This project is used to:

- Filter, slim, trim the trees from a given AP production
- Rename branches

This is done using configurations in a YAML file and through Ganga jobs.

## Installation

This project cannot be installed and used from within a virtual environment. It depends on Ganga
and therefore one has to be installed in `$HOME/.local/` for this do:

```bash
. cvmfs/lhcb.cern.ch/lib/LbEnv

# Will install a few projects on top of what is already in the LHCb environment
pip install post_ap

# Make a proxy that lasts 100 hours
lhcb-proxy-init -v 100:00
```

In order to make Ganga aware of the `post_ap` package do:

```python
import sys
import os

home_dir = os.environ['HOME']
sys.path.append(f'{home_dir}/.local/lib/python3.12/site-packages')
sys.path.append('/cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/3386/stable/linux-64/lib/python3.12/site-packages')
```

in `.ganga.py`, where the path will depend on the version of python used by the LHCb environment.

To check that this is working, open ganga and run:

```bash
from post_ap.pfn_reader        import PFNReader
```

# Submitting jobs

For this one would run a line like:

```bash
job_filter_ganga -n job_name -N NJOBS -p PRODUCTION -s SAMPLE -c /path/to/config/file.yaml -b BACKEND -v VERSION_OF_ENV 
```

The code used to filter reside in the grid and the only thing the user has to do is to provide the latest version

## Check latest version of virtual environment

The jobs below will run with code from a virtual environment that is already in the grid. One should use the
latest version of this environment. To know the latest versions, run:

```bash
# In a separate terminal open a shell with access to dirac
post_shell

# Run this command for a list of environmets
list_venvs
```

The `post_shell` terminal won't be used to send jobs.

## Config file

Here is where all the configuration goes and an example of a config can be found [here](https://github.com/acampove/config_files/blob/main/post_ap/v3.yaml)

## Optional

- In order to improve the ganga experience use: 

```bash
# Minimizes messages when opening ganga
# Does not start monitoring of jobs by default
alias ganga='ganga --quiet --no-mon'
```

in the `$HOME/.bashrc` file. Monitoring can be turned on by hand as explained [here](https://twiki.cern.ch/twiki/bin/viewauth/LHCb/FAQ/GangaLHCbFAQ#How_can_I_run_the_monitoring_loo)
