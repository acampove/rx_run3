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

## Listing available triggers

In order to see what triggers are present in the current version of the ntuples do:

```bash
list_triggers -v v1 -k rx

# And this will save them to a yaml file
list_triggers -v v1 -k rx -o triggers.yaml
```

The code below assumes that all the data is in `ANADIR`. If you want to use the data
in EOS do:

```bash
export ANADIR=/eos/lhcb/wg/RD/RX_run3
```

preferably in `~/.bashrc`.

## Downloading the ntuples

For this, run:

```bash
download_rx_data -m 5 -v v1 -d -t triggers.yaml -k rx
```

which will use 5 threads to download the ntuples associated to the triggers in `triggers.yaml`
and version `v1` to the specified path.

The full options are:

```
options:
  -h, --help              show this help message and exit
  -t TRIG, --trig TRIG    Path to YAML file with list of triggers
  -v VERS, --vers VERS    Version of LFNs
  -k KIND, --kind KIND    Type of production
  -n NFILE, --nfile NFILE Number of files to download
  -l {10,20,30,40}, --log {10,20,30,40} Log level, default 20
  -m MTH, --mth MTH       Number of threads to use for downloading, default 1
  -r, --ran               When picking a subset of files, with -n, pick them randomly
  -d, --dryr              If used, it will skip downloads, but do everything else
  -f, --force             If used, it will download even if output already exists
```

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
list_triggers -v v4 -o triggers.yaml -k rx

# This will use those triggers only to make samples.yaml
make_tree_structure -v v4 -f samples.yaml -t triggers.yaml
```

### Sending files to user's CERNBOX

In order to share files one can:

- Use the CERNBOX [website](https://cernbox.cern.ch) to upload the files. These files will endup in EOS. One can upload entire directories.
- Use `make_tree_structure` to dump to YAML the list of PFNs with:

```bash
make_tree_structure -i /publicfs/ucas/user/campoverde/Data/RX_run3/v5/mva/v1 -f rx_mva.yaml -p /eos/user/a/acampove/Data/mva/v1
```

Where `-p` is the directory in EOS where the files will go.

### Summary

Due to the complexity of this command, the table below was made to sumarize its functionality:

| To build:                   | Input                                                                     | Output                                |
| --------------------------- | ------------------------------------------------------------------------- | ------------------------------------- |
| Tree structure              | `-i /path/to/directory/*root`                                             | `-o /path/to/start/of/tree/structure` |
| YAML list with local files  | `-i /path/to/directory/*root`                                             | `-f /path/to/samples.yaml`            |
| YAML list with PFNs         | `-v v4` where the LFNs are files, part of the project                     | `-f samples.yaml`                     |
| YAML with PFNs in users EOS | `-i /path/to/directory/*.root -p /eos/user/x/uname/directory_with_files/` | `-f samples.yaml`                     |

## Samples naming

The samples were named after the DecFiles names for the samples and:

- Replacing certain special charactes as shown [here](https://github.com/acampove/ap_utilities/blob/main/src/ap_utilities/decays/utilities.py#L24)
- Adding a `_SS` suffix for split sim samples. I.e. samples where the photon converts into an electron pair.

A useful guide showing the correspondence between event type and name is [here](https://github.com/acampove/ap_utilities/blob/main/src/ap_utilities_data/evt_form.yaml)

# Accessing ntuples

Assuming that all the tnuples for data and simulation are in a given directory, the line below:

```bash
make_tree_structure -i /directory/with/ntuples -f samples.yaml
```

Will create a `samples.yaml` with the list of paths to ROOT files, per trigger and sample.
If a second set of branches can be obtained, e.g. with MVA scores, one can run the same command:

```bash
make_tree_structure -i /directory/with/mva/ntuples -f mva.yaml
```

and in order to attach the main ntuples to the MVA ntuples:

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

## Printing information on samples

Use:

```bash
check_sample_stats -p /path/to/rx_samples.yaml
```

to print a table to markdown with the sizes of each sample in Megabytes. e.g.:

```markdown
| Sample                                      | Trigger                        |   Size |
|:--------------------------------------------|:-------------------------------|-------:|
| Bu_JpsiK_mm_eq_DPC                          | Hlt2RD_BuToKpMuMu_MVA          |  15829 |     ■■■■ 'BuToKpMuMu': Possible spelling mistake found.
| Bs_Jpsiphi_mm_eq_CPV_update2016_DPC         | Hlt2RD_BuToKpMuMu_MVA          |  11164 |     ■■■■ 'BuToKpMuMu': Possible spelling mistake found.
| Bd_JpsiKst_mm_eq_DPC                        | Hlt2RD_BuToKpMuMu_MVA          |   9945 |     ■■■■ 'BuToKpMuMu': Possible spelling mistake found.
| Bu_JpsiK_ee_eq_DPC                          | Hlt2RD_BuToKpEE_MVA_cal        |   8873 |     ■■■■■ 'BuToKpEE': Possible spelling mistake found.
| Bu_JpsiK_ee_eq_DPC                          | Hlt2RD_BuToKpEE_MVA            |   8488 |
...
```

## Merging files

After the preselection the data files are very small and there are many of them. The following line can be used to merge them:

```bash
merge_samples -p rx -s DATA_24_MagDown_24c3 -t Hlt2RD_BuToKpMuMu_MVA -v v7
```

where the command will merge all the files associated to a given sample and trigger.

## Copying files

If the original files are downloaded to a cluster and the user needs the files in e.g. a laptop one could:

- Use SSHFS to mount the cluster's file system in the laptop.
- Copy the files through

```bash
copy_samples -k all -c rk
```

where:

`-k` Kind of files to be copied, i.e. friend tree like `mva`, `main`, `hop` etc. For everything use `all`.   
`-c` Name of config specifying what to copy, e.g. `rk`   
`-p` Projecte.g. `rx` (default), `nopid`, etc. These correspond to the subdirectory in `$ANADIR/Data`

The config files live in `src/rx_data_data/copy_files` and can be adapted for new samples or different source paths.

## Checking for corrupted files

For this run:

```bash
check_corrupted -p /path/to/directory/with/files -x "data_*_MVA_*.root"
```

Which will check for corrupted files and will remove them.
`-x` can be used to pass wildcards, in the case above, it would target only data.
After removal, the download can be tried again, which would run only on the missing samples.
This might allow for these files to be fixed, assuming that they were broken due to network issues. 

## Calculating extra branches (no MVA)

Given the files produced by `post_ap`, new branches can be attached. These branches can be calculated using
`branch_calculator` and can be placed in small files. These latter files would be made into friends of the main files.

In order to do this we assume that all the ntuples live in `$DATADIR/main/vx`, where `DATADIR` needs to be exported
such that the code will pick it up. `vx` represents a version of the ntuples (e.g. `v1`, `v2`, etc), the code will 
pick up the latest. Then run:

```bash
branch_calculator -k swp_jpsi_misid -p  0 40 -b -v v1 -s 10000
```

which will:

- Create a new set of files in `$ANADIR/Data/swp_jpsi_misid/v1` with each input file, corresponding to an output file.
- Split the input files into 40 groups, with roughly the same file size.
- Process files by chunks of 10 thousands entries at a time. This is needed to prevent the cluster (HTCondor, etc) to run out of memory.
- Process the zeroth group.

Thus, this can be parallelized by running the line above 40 times in 40 jobs.

Currently the command can add:

`swp_jpsi_misid`: Branches corresponding to lepton kaon swaps that make the resonant mode leak into rare modes. Where the swap is inverted and the $J/\psi$ mass provided

`swp_cascade`: Branches corresponding to $D\to K\pi$ with $\pi\to\ell$ swaps, where the swap is inverted and the $D$ mass provided.

`hop`: With the $\alpha$ and mass branches calculated

## Calculating MVA branches

The code needed to calculate the trees with the MVA branches is different. For this do:

```bash
apply_classifier -v VERSION -s SAMPLE -t HLTTRIGGER
```

for a given sample. The code will 

- Search each file
- Create another file with the friend tree

To run this as a job, all the commands can be dumped in a text file with:

```bash
make_mva_joblist -v VERSION
```

where `VERSION` is the version of the MVA classifier 

## Missing files

Once the extra branches have been calculated, one will have a file structure like:

```
├── brem_track_2
│   └── v1
├── ecalo_bias
│   ├── v0
│   └── v1
├── hop
│   ├── v1
│   └── v2
├── main
│   ├── v1
│   ├── v5
│   └── v7
├── mva
│   ├── v1
│   ├── v2
│   ├── v3
│   └── v4
├── swp_cascade
│   ├── v1
│   └── v2
└── swp_jpsi_misid
    ├── v1
    └── v2
```

Typically (not always), a sample in `main` should be also in all the other friend trees.
If samples are missing, one could do:

```bash
check_missing -d /path/to/directory/with/samples
```

where the argument is optional, if not passed, the script will take the value of the 
`DATADIR` environment variable.

Other options are:

```
options:
  -h, --help            show this help message and exit
  -d DATA_DIR, --data_dir DATA_DIR
                        Path to directory with main and friend samples, if not passed, will pick
                        DATADIR from environment
  -s SKIP_SAM [SKIP_SAM ...], --skip_sam SKIP_SAM [SKIP_SAM ...]
                        Samples to skip
  -l {10,20,30}, --log_level {10,20,30}
                        Logging level
```

the output will be `missing.yaml`. This file will tell you:

- Which friend trees have which samples missing.
- Which are the files missing

In this case `missing` is relative to the samples and files in the `main` sample.

## Calculating luminosity

For this one would have to use the `get_lumi` script in this project, together
with the one in [this](https://gitlab.cern.ch/lhcb-luminosity/lumi_calib/-/blob/master/get_lumi.py?ref_type=heads)
project. This should, at some point, be cleaned up.
