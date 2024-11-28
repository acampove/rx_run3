# ap_utilities

For documentation specific to MVA lines of the RD group, check [this](doc/mva_lines.md)

## Environment and installation

### With access to grid

This is needed to run the scripts that check the bookkeeping path for samples existence.
To run this one has to be in an environment with:

1. Access to DIRAC.
- Setup the LHCb environment with
```bash
. /cvmfs/lhcb.cern.ch/lib/LbEnv 

# Token valid fro 100 hours
lhcb-proxy-init -v 100:00
```
- Access a shell with dirac:
```bash
lb-dirac bash
```
- Install this project:
```bash
pip install ap-utilities
```
### Without access to grid

Otherwise, one can just:

- Install this project:
```bash
pip install ap-utilities
````

and use the project

## Decay nicknames

### Accessing table with DecFiles sample nicknames

These nicknames can be accessed from python scripts with:

```python
import ap_utilities.physics.utilities as aput

# To get exactly what was saved
literal = aput.read_decay_name(event_type=event_type, style='literal')

# To get representation with special symbols like "," or "-" replaced
safe_1  = aput.read_decay_name(event_type=event_type, style= 'safe_1')
```

### Update table with nicknames and event types

This is most likely not needed, unless a new sample has been created and a new nickname needs to be added. The following lines:

```bash
export DECPATH=/home/acampove/Packages/DecFiles

update_decinfo
```

will:

1. Set the path to the [DecFiles](https://gitlab.cern.ch/lhcb-datapkg/Gen/DecFiles)
root directory such that `update_decinfo` can use it.
1. Read the event types and nicknames and save them to a YAML file

## Check for samples existence

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

to check if the samples exist using 6 threads (default is 1). The script will produce 
`info_SECTION_NAME.yaml` and `validation_SECTION_NAME.yaml`, which will correspond to each sections up there
i.e. `one`, `two` and `three`. 

Once this has been done, the lines needed for the `info.yaml` can be obtained by concatenating the partial outputs with:

```bash
cat info_*.yaml > samples.yaml
```

**Important**: Given that most settings are the same between sections, one can use anchors and aliases to override
only what is different between them.


## Validate outputs of pipelines

In order to do this:

### Mount EOS in laptop

```bash
# install SSHF
...
# Check that it's installed
which sshfs

# Make directory to mount EOS

APDIR=/eos/lhcb/wg/dpa/wp2/ci/
sudo mkdir -p $APDIR
sudo chown $USER:$USER $APDIR 

# Mount EOS
sshfs -o idmap=user USERNAME@lxplus.cern.ch:$MNT_DIR $MNT_DIR
```

### Checks prior to submit MR or run pipelines

In order to do these checks run:

```bash
check_production -p /home/acampove/Packages/AnalysisProductions/rd_ap_2024
```

Where the path is the path to the production directory. This script will check:

1. If the samples (by nickname) in `info.yaml` are different. Same nicknames are not expected.
1. If the entries in `mcfuntuple.yaml` are different.
1. If there are samples in `info.yaml` are not found in `mcfuntuple.yaml`. In which case `MCDecayTree` will not be
made.

### Run Validation

```bash
# This project is in pip
pip install ap_utilities

validate_ap_tuples -p PIPELINE -f ntuple_scheme.yaml -t 5
```

where:   
-l: Logging level, by default 20 (info), but it can be 10 (debug) or 30 (warning)   
-t: Is the number of threads to use, if not passed, it will use one.   
-p: Is the pipeline number, needed to find the ROOT files in EOS   
-f: passes the file with the configuration   

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
