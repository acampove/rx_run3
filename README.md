# ap_utilities

- For instructions on how to install this project, check [this](doc/installation.md)   
- For documentation specific to MVA lines of the RD group, check [this](doc/mva_lines.md)   
- For tools to deal with nicknames check [this](doc/nicknames.md)
- For instructions to mount EOS in your laptop check [this](doc/mounting_eos.md)
- For instructions on how to make the decay descriptor fields check [this](doc/descriptors.md)

## How to add a decay 

### Add the decay matching lines 
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

### Add the list of samples 

The list goes in `rd_ap_2024/info.yaml`. For this, the [installation](doc/installation.md#with-access-to-dirac) that allows access to DIRAC is needed. 
Given a set of MC samples specified in a YAML file like:

```YAML
settings_common: &common
  year      : 2024
  mc_path   : 2024.W31.34
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
    evt_type:
      - '11102211'
      - '11102202'
  two:
    settings:
      <<        : *common
      sim_vers  : Sim10d-SplitSim02
    evt_type:
      - '11102211'
      - '11102202'
  three:
    settings:
      <<        : *common
      generator : BcVegPyPythia8
    evt_type:
      - '14143013'
      - '14113032'
```

run:

```bash
check_samples -i samples.yaml -n 6
```

to check if the samples exist using 6 threads (default is 1). The script will produce:

- `info_SECTION_NAME.yaml`: Where `SECTION_NAME` corresponds to each section above, i.e. `one`, `two`, `three`
- `validation_SECTION_NAME.yaml`: Which will be needed for validation later.

Once this has been done, the lines needed for the `info.yaml` can be obtained by concatenating the partial outputs with:

```bash
cat info_*.yaml > samples.yaml
```

### Updating `tupling/config/samples.yaml`

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

## Checks 

### Before pipelines 

In order to do these checks run:

```bash
check_production -p /home/acampove/Packages/AnalysisProductions/rd_ap_2024
```

Where the path is the path to the production directory. This script will check:

1. If the samples (by nickname) in `info.yaml` are different. Same nicknames are not expected.
1. If the entries in `mcfuntuple.yaml` are different.
1. If there are samples in `info.yaml` are not found in `mcfuntuple.yaml`. In which case `MCDecayTree` will not be
made.

The second argument is a list of strings representing samples. Here they represent inclusive samples, which should
be skipped; this argument is optional. 

This script will produce `report.yaml`, which looks like:

```yaml
# Print nicknames of samples going above 100 characters
long_nicknames: ['105', 'some_long_sample_name']
missing:
  info.yaml_mcfuntuple.yaml:
    only info.yaml:
      - Bd_KstPi0gamma_Kpi_eq_DPC_SS
      - Bd_Ksteta_gg_eq_DPC_SS
    only mcfuntuple.yaml:
      - Bd_KplKmn_eq_DPC
      - Bd_Kplpimn_eq_CPV2017_DPC
  info.yaml_samples.yaml:
    only info.yaml:
      - Bd_Denu_Kstenu_eq_VIA_HVM_EGDWC
      - Bd_Dmunu_Kstmunu_eq_DPC
    only samples.yaml:
      - Bd_KplKmn_eq_DPC
      - Bd_Kplpimn_eq_CPV2017_DPC
  mcfuntuple.yaml_samples.yaml:
    only mcfuntuple.yaml:
      - Bd_Denu_Kstenu_eq_VIA_HVM_EGDWC
      - Bd_Dmunu_Kstmunu_eq_DPC
    only samples.yaml:
      - Dst_D0pi_KK_TightCut
      - Dst_D0pi_KPi_TightCut
```

### After pipelines

This is done in an environment with access to EOS. To gain EOS access from outside of LXPLUS (e.g. a laptop) follow 
[these](doc/mounting_eos.md) instructions. After that do:

```bash
# This project is in pip
pip install ap_utilities

validate_ap_tuples -p PIPELINE -f ntuple_scheme.yaml -t 5
```

Where:   
`-l`: Logging level, by default 20 (info), but it can be 10 (debug) or 30 (warning)   
`-t`: Is the number of threads to use, if not passed, it will use one.   
`-p`: Is the pipeline number, needed to find the ROOT files in EOS   
`-f`: passes the file with the configuration   

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

a few examples of config files can be found [here](https://github.com/acampove/config_files/tree/main/ap_utilities/validate_ap)
