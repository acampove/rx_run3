PID correction of simulation
===

### Introduction

The following set of scripts aims to do two main things:
- Create the histograms that parametrize the efficiency of specific PID cuts for muons, kaons and pions both in Data and MC
- Attach these efficiencies as weights to the MC samples, to then use their ratio to correct the simulation.

### Creating the histograms
A preliminary version of the histograms can be produce by running the `run.sh` bash script. This script relies on the python script `create_pidcalib2_histograms_scripts.py` and the configuration files written in `.yaml` format, stored in `./configurations/`. At the moment some examples are stored in there. 
Once these two ingredients are available, we can then execute `./run.sh`, that will produce:
* For simulation, after having created a set of mock sweight files with weights all equal to one (expected by the PIDCalib2 since Data has it), a new `.json` file is created.
* For data, a copy of the `.json` file stored in a local and updated version of pidcalib2 should be provided. A copy of that file, with additional modifications is produced.
* A series of bash scripts saved in `./scripts/bash_scripts/`, that can be executed locally to test them
* A series of condor scripts saved in  `./scripts/condr_scripts/`, that submit the scripts `./scripts/bash_scripts/` on HTCondor.
* The produced tables are saved in `.pkl` next to the `.json` files used for their production.
* If the flag `--plot` is used, the very simple script `plot_histograms.py` is used to produce a standard set of plots, that are saved next to the `./pkl`.

## Attaching the efficiency on simulation
To be understood exactly as this needs to be done. `pidcalib2.ref_calib` is available in PIDCalib2.