[TOC]

- For instructions on how to install this project, check [this](doc/installation.md)   
- For documentation specific to MVA lines of the RD group, check [this](doc/mva_lines.md)   
- For tools to deal with nicknames check [this](doc/nicknames.md)
- For instructions to mount EOS in your laptop check [this](doc/mounting_eos.md)
- For instructions on how to make the decay descriptor fields check [this](doc/descriptors.md)

# Environment

The following variables _can_ be defined:

`ANADIR`: Where outputs will be saved, if not specified, outputs will go to `/tmp`

# Add samples

In order to add new samples to the analysis do:

- Open `src/ap_utilities_data/analyses/by_priority.yaml`
- Add the event type to the `all` section
- Run

```bash
analyze_samples -c by_priority
```

this should create: 

**summary.yaml**: which will show the samples not found already in
`by_priority.yaml`. Add the lines to the sections according to the priority.
Samples in `all` but not `priority` section     will go in `missing`   
Samples in `priority` section, but not in `all` will go in `new`.

**missing.yaml**: If the missing sections were filled with lists of event types, `missing.yaml`
will contain the mappings with the decay string.

## Add the list of samples 

The list goes in `rd_ap_2024/info.yaml`. For this, the [installation](doc/installation.md#with-access-to-dirac) that allows access to DIRAC is needed. 
Given a set of settings specified in `ap_utilities_data/samples/2024.yaml` file like:

```yaml
settings_common: &common
    year      : 2024
    block_id  : 2024.W31.34
    hlt_conf  : 2024.W31.34
    polarity  : MagUp
    nu_path   : Nu6.3
    sim_vers  : Sim10d
    generator : Pythia8
    ctags     : sim10-2024.Q3.4-v1.3-mu100
    dtags     : dddb-20240427
# -------------------------------------------
sections:
    one:
      settings:
        <<: *common
    two:
      settings:
        <<        : *common
        sim_vers  : Sim10d-SplitSim02
    three:
      settings:
        <<        : *common
        generator : BcVegPyPythia8
```

and a set of samples specified in `ap_utilities_data/analysis/by_priority.yaml` like:

```yaml
high_priority:
  12123003  : $B_u \to K^+ e^+ e^-$
  11114002  : $B_d \to K^{*0} \mu^+ \mu^-$
medium_priority:
  12153020  : $B_u \to J/\psi(e^+ e^-) \pi^+$
  12143010  : $B_u \to J/\psi(\mu^+ \mu^-) \pi^+$
```

## Check existing samples 

run:

```bash
check_samples -c 2024 -s by_priority -n 6
```

to check if the samples exist using 6 threads (default is 1). The script will produce:

- `info_SECTION_NAME.yaml`: Where `SECTION_NAME` corresponds to each section above, i.e. `one`, `two`, `three`
- `validation_SECTION_NAME.yaml`: Which will be needed for validation later.

Once this has been done, the lines needed for the `info.yaml` can be obtained by concatenating the partial outputs with:

## Pick only samples that do not exist as ntuples

Currently there are 241 AP jobs associated to the MC. Only 35 are missing, in order to find out
what ntupling jobs need to be send do:

```bash
find_in_ap
```

which will:

- Read the `info.yaml` files created above
- Check, using `apd`, what samples are missing
- Create a `info.yaml` in the current directory, only with those samples.

# How to add a decay 

## Add the decay matching lines 
This is done in `rd_ap_2024/tupling/config/mcfuntuple.yaml`. Each section in this file looks like:

```yaml
# This is a nickname for the sample
Bd_Denu_Kstenu_eq_VisibleInAcceptance_HighVisMass_EGDWC:
# This is the decay descriptor and how to match particles to branch names
  Bd   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  D    : '[B0  ==>  ^(  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  Em   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )  ^e-  anti-nu_e  )   e+  nu_e  ]CC'
  Ep   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )  ^e+  nu_e  ]CC'
  Kp   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==> ^K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  Kst  : '[B0  ==>   (  D-  ==>  ^(  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
  nu   : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+  pi-  )   e-  anti-nu_e  )   e+ ^nu_e  ]CC'
  pim  : '[B0  ==>   (  D-  ==>   (  K*(892)0  ==>  K+ ^pi-  )   e-  anti-nu_e  )   e+  nu_e  ]CC'
```

- To get the sample nickname follow [this](doc/nicknames.md)
- To get the descriptors one can either write them down in the case of a few samples or run follow [these](doc/descriptors.md) instructions in case of multiple.


## Updating `tupling/config/samples_turbo_lines_mapping.yaml`

This file lists the samples together with the _analyses_ like in:

```yaml
Bc_Dsst2573mumu_KKpi_eq_BcVegPy_DPC: # This is a sample
- Bc_lines                           # This is an analysis
Bc_Jpsipi_mm_eq_WeightedBcVegPy_DPC:
- Bc_lines
Bc_pimumu_eq_PHSP_BcVegPy_DPC:
- Bc_lines
```

Where the _analyses_ are sets of HLT2 lines described in `tupling/config/analyses.yaml`.

# Checks 

## Before pipelines 

There are four files relevant to the production:

## tupling/config/samples_turbo_lines_mapping.yaml

This is where the sample $\to$ HLT line category is specified. I.e. each sample
will be processed under a certain group of lines, `rx_muon` lines, `rx_electron` lines etc.

## tupling/config/mcfuntuple.yaml

This is where the decay destriptors used for the `MCDecayTree` building are specified

## info.yaml 

This is where the samples that are made into ntuples will be specified.

## Reference event types

Which are stored in this project, in:

```bash
ap_utilities/src/ap_utilities_data/analyses/analyses.yaml
```

and which describes, for each analysis (currently `rk` and `rx`), what event types should be used.

One needs to make sure that samples that will be made: 
- Have a descriptor for the `MCDecayTree`.
- Are in the reference `analyses.yaml` file

Etc

In order to do these checks run:

```bash
check_production -p /home/acampove/Packages/AnalysisProductions/rd_ap_2024 -a rx
```

This script will produce `report.yaml`, which looks like:

```yaml
# Print nicknames of samples going above 100 characters
long_nicknames: ['105', 'some_long_sample_name']
missing:
  info_mcfuntuple:
    only info:        # Samples specified for production, but without MCDecayTree
      - ...
    only mcfuntuple:  # Samples with MCDecayTree but not been made
      - ...
  info_rx:
    only info:        # Samples been made, but not in the reference file
      - ...
    only rx:          # Samples in the reference file but not been made
      - ...
  info_samples:
    only info:        # Samples been made, but without trigger association (DANGER: this wil break the pipeline)
      - ...
    only samples:     # Samples with trigger association and not been ntupled
      - ...
  mcfuntuple_samples:
    only mcfuntuple:  # Samples with MCDecayTree but not been ntupled
      - ...
    only samples:     # Samples with HLT trigger association but no MCDecayTree.
      - ...
```

## After pipelines

This is done in an environment with access to EOS. To gain EOS access from outside of LXPLUS (e.g. a laptop) follow 
[these](doc/mounting_eos.md) instructions. After that do:

```bash
# This project is in pip
pip install ap_utilities

validate_ap_tuples -p 18455 -c 05_09_2024 -t 4
```

Where:   
`-l`: Logging level, by default 20 (info), but it can be 10 (debug) or 30 (warning)   
`-t`: Is the number of threads to use, if not passed, it will use one.   
`-p`: Is the pipeline number, needed to find the ROOT files in EOS   
`-c`: Name of config file written as shown below 

```yaml
# -----------------------------------------
# Needed to find where files are in EOS
# -----------------------------------------
paths:
  pipeline_dir : /eos/lhcb/wg/dpa/wp2/ci
  analysis_dir : rd_ap_2024
# -----------------------------------------
# Each key corresponds to a MC sample, the value is a list of lines that must be found
# as a tree in the file. If any, then the sample is not signal for any of the HLT2 lines
# therefore no tree (equivalent to a line) is required to be made
# -----------------------------------------
samples:
  # These is a sample without a dedicated trigger
  Bu_K1ee_eq_DPC:
    - any 
  # This is a sample with two triggers targetting it
  Bd_Kpiee_eq_DPC:
    - Hlt2RD_B0ToKpPimEE
    - Hlt2RD_B0ToKpPimEE_MVA
```

The configs will be already in the package in `ap_utilities_data/validation/`

# Utilities

## Listing ntupled samples

In order to list the samples ntupled and belonging to `rd_ap_2024` (can be made more flexible in the future) do:

```bash
list_ap_samples -v v1r3788
```

where the version is associated to the AP MR.
