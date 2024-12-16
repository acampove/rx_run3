# $R_X$ data

This repository contains:

- Versioned lists of LFNs
- Utilities to download them and link them into a tree structure

for all the $R_X$ like analyses.

## Installation

To install this project run:

```bash
pip install rx_data

# The line below will upgrade it, in case new samples are available, the list of LFNs is part of the
# project itself
pip install --upgrade rx_data
```

The download would require a grid proxy, which can be made with:

```bash
. /cvmfs/lhcb.cern.ch/lib/LbEnv

# This will create a 100 hours long proxy
lhcb-proxy-init -v 100:00
```

## Listing available triggers

In order to see what triggers are present in the current version of the ntuples do:

```bash
list_triggers -v v1

# And this will save them to a yaml file
list_triggers -v v1 -o triggers.yaml
```

## Downloading the ntuples

For this, run:

```bash
download_rx_data -m 5 -p /path/to/downloaded/.data -v v1 -d -t triggers.yaml
```

which will use 5 threads to download the ntuples associated to the triggers in `triggers.yaml`
and version `v1` to the specified path.

**IMPORTANT**: 
- In order to prevent deleting the data, save it in a hiden folder, e.g. one starting with a period. Above it is `.data`.
- This path is optional, one can export `DOWNLOAD_NTUPPATH` and the path will be picked up

**Potential problems**:
The download happens through XROOTD, which will try to pick a kerberos token. If authentication problems happen, do:

```bash
which kinit
```

and make sure that your kinit does not come from a virtual environment but it is the one in the LHCb stack or the native one.

## Building directory structure

All the ntuples will be downloaded in a single directory.
In order to group them by sample and trigger run:

```bash
make_tree_structure -i /path/to/downloaded/.data/v1 -o /path/to/directory/structure
```

this will not make a copy of the ntuples, it will only create symbolic links to them.
