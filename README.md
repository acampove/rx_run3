[TOC]

# $R_X$ data

This repository contains:

- Versioned lists of LFNs
- Utilities to download them and link them into a tree structure

for all the $R_X$ like analyses.

## Installation

To install this project run:

```bash
pip install git+ssh://git@gitlab.cern.ch:7999/rx_run3/rx_data.git
```

The code below assumes that all the data is in `ANADIR`. If you want to use the data
in EOS do:

```bash
export ANADIR=/eos/lhcb/wg/RD/RX_run3
```

preferably in `~/.bashrc`.

# Accessing ntuples

Once 

```python
from rx_data.rdf_getter     import RDFGetter

# This picks one sample for a given trigger
# The sample accepts wildcards, e.g. `DATA_24_MagUp_24c*` for all the periods
gtr = RDFGetter(
    sample   ='DATA_24_Mag*_24c*', 
    analysis = 'rx',                    # This is the default, could be nopid
    tree     = 'DecayTree'              # This is the default, could be MCDecayTre
    trigger  ='Hlt2RD_BuToKpMuMu_MVA')

# If False (default) will return a single dataframe for the sample
rdf = gtr.get_rdf(per_file=False)

# If True, will return a dictionary with an entry per file. They key is the full path of the ROOT file
d_rdf = gtr.get_rdf(per_file=True)
```

The way this class will find the paths to the ntuples is by using the `DATADIR` environment
variable. This variable will point to a path `$DATADIR/samples/` with the `YAML` files
mentioned above.

In the case of the MVA friend trees the branches added would be `mva.mva_cmb` and `mva.mva_prc`.

Thus, one can easily extend the ntuples with extra branches without remaking them.

## Unique identifiers

In order to get a string that fully identifies the underlying sample, 
i.e. a hash, do:

```python
gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger='Hlt2RD_BuToKpMuMu_MVA')
uid = gtr.get_uid()
```

## Excluding datasets

One can also exclude a certain type of friend trees with:
```python
from rx_data.rdf_getter     import RDFGetter

wih RDFGetter.exclude_friends(names=['mva']):
    gtr = RDFGetter(sample='DATA_24_Mag*_24c*', trigger='Hlt2RD_BuToKpMuMu_MVA')
    rdf = gtr.get_rdf(per_file=False)
```

that should leave the MVA branches out of the dataframe.

## Defining custom columns

Given that this `RDFGetter` can be used across multiple modules, the safest way to
add extra columns is by specifying their definitions once at the beggining of the
process (i.e. the initializer function called within the main function). 
This is done with:

```python
from rx_data.rdf_getter     import RDFGetter

RDFGetter.set_custom_columns(d_def = d_def)
```

If custom columns are defined in more than one place in the code, the function will
raise an exception, thus ensuring a unique definition for all dataframes.

## Accessing metadata

Information on the ntuples can be accessed through the `metadata` instance of the `TStringObj` class, which is
stored in the ROOT files. This information can be dumped in a YAML file for easy access with:


```bash
dump_metadata -f root://x509up_u12477@eoslhcb.cern.ch//eos/lhcb/grid/user/lhcb/user/a/acampove/2025_02/1044184/1044184991/data_24_magdown_turbo_24c2_Hlt2RD_BuToKpEE_MVA_4df98a7f32.root
```

which will produce `metadata.yaml`.
