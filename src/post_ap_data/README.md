[[_TOC_]]

# Description

These are YAML and TOML config files used for different reasons:


## For filtering

### Information on LHCb

These files look like `lhcb_year.yaml` and contain information about correspondences
between blocks and runs or fills.

## For unit tests

### JSON files with lists of PFNs

These look like mc...json or dt...json and are used to store lists of PFNs, they are meant to be used by tests.

## After filtering

These files should be sent to the `config_files` project, eventually

### Linking and merging of files

**link_confg.yaml**: By the `link_merge` utility to assign readable directory name from event type.

### Plotting before and after selections

**hlt_cmp.yaml**: Used to plot run number after full selection with and without BDT 
**hlt_cmp_raw.yaml**: Used to plot run number before selection, out of DaVinci 

