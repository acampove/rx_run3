# Checks 

## Before pipelines 

There are four files relevant to the production:

### tupling/config/samples_turbo_lines_mapping.yaml

This is where the sample $\to$ HLT line category is specified. I.e. each sample
will be processed under a certain group of lines, `rx_muon` lines, `rx_electron` lines etc.

### tupling/config/mcfuntuple.yaml

This is where the decay destriptors used for the `MCDecayTree` building are specified

### info.yaml 

This is where the samples that are made into ntuples will be specified.

### Reference event types

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
