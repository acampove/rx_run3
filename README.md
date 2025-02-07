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

## Organizing paths

### Building directory structure

All the ntuples will be downloaded in a single directory.
In order to group them by sample and trigger run:

```bash
make_tree_structure -i /path/to/downloaded/.data/v1 -o /path/to/directory/structure
```

this will not make a copy of the ntuples, it will only create symbolic links to them.

### Making YAML with files list

If instead one does:

```bash
make_tree_structure -i /path/to/downloaded/.data/v1 -f samples.yaml
```

the links won't be made, instead a YAML file will be created with the list of files for each sample and trigger.

### Lists from files in the grid

If instead of taking the downloaded files, one wants the ones in the grid, one can do:

```bash
make_tree_structure -v v4 -f samples.yaml
```

where `v4` is the version of the JSON files holding the LFNs. In case one needs the old naming, used in Run1 and Run2
one would run:

```bash
make_tree_structure -v v4 -f samples.yaml -n old
```

**This will likely drop samples that have no old naming, because they were not used in the past.**

### Dropping triggers

The YAML outputs of the commands above will be very large and not all of it will be needed. One can drop triggers by:

```bash
# This will dump a list of triggers to triggers.yaml
# You can optionally remove not needed triggers
list_triggers -v v4 -o triggers.yaml

# This will use those triggers only to make samples.yaml
make_tree_structure -v v4 -f samples.yaml -t triggers.yaml
```

## Samples naming

The samples were named after the DecFiles names for the samples and:

- Replacing certain special charactes as shown [here](https://github.com/acampove/ap_utilities/blob/main/src/ap_utilities/decays/utilities.py#L24)
- Adding a `_SS` suffix for split sim samples. I.e. samples where the photon converts into an electron pair.

A useful guide showing the correspondence between event type and name is [here](https://github.com/acampove/ap_utilities/blob/main/src/ap_utilities_data/evt_form.yaml)

# Accessing ntuples

If the ntuples are stored in a directory where each tuple is accompanied by a friend tree, a preliminary
step that attaches all friend trees is needed. This is done by `RDFGetter` as shown below:


```python
from rx_data.rdf_getter     import RDFGetter

# This is where the directories with the samples are
RDFGetter.samples_dir = '/publicfs/ucas/user/campoverde/Data/RX_run3/v4/NO_q2_bdt_mass_Q2_central_VR_v1'

# This picks one sample for a given trigger
# The sample accepts wildcards, e.g. `DATA_24_MagUp_24c*` for all the periods
gtr = RDFGetter(sample='DATA_24_MagUp_24c2', trigger='Hlt2RD_BuToKpMuMu_MVA')
rdf = gtr.get_rdf()
```

In the case of the MVA friend trees the branches added would be `mva.mva_cmb` and `mva.mva_prc`.

## Accessing metadata 

Information on the ntuples can be accessed through the `metadata` instance of the `TStringObj` class, which is
stored in the ROOT files. This information can be dumped in a YAML file for easy access with:


```bash
dump_metadata -f root://x509up_u12477@eoslhcb.cern.ch//eos/lhcb/grid/user/lhcb/user/a/acampove/2025_02/1044184/1044184991/data_24_magdown_turbo_24c2_Hlt2RD_BuToKpEE_MVA_4df98a7f32.root
```

which will produce `metadata.yaml`.
















