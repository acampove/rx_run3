# q2 systematics

This project is used to assess the systematics associated to q2 smearing. 

# Installation

Make a virtual environment like:

```bash
mamba create -n q2_systematics root=6.28 python=3.9
```

# Usage

## Nominal and nSPD binning systematics

In order to get the smearing factors run:

```bash
get_tables -v vx -t [ETOS,GTIS] -y [2011,2012...] -b [0,1,2] -x [nom, nspd]
```

with `vx` as the name of the directory where output will go. Other options for running over specific years can be checked with:

```bash
get_tables -h
```
The code will:

1. Load trees and apply a selection both to MC and data.
2. Fit MC.
3. Fit data with shapes fixed to simulation, except for mean and width.
4. Extract the parameters and store them.

## Momentum dependent systematics

Run:

```bash
get_resolutions -d 2017 -t GTIS -b 0 -v v8 -s
```

to run the fits to the simulation, for a given dataset, trigger and brem category, for data drop `-s`. 
The version is only used to name the output directory.

## Jobs

However, to speed things up, one can send the jobs to the cluster to run in parallel for each dataset with:

```bash
./job.sh vx q2sys [nom, nspd]
```
for nominal and `nSPD` systematics and:

```bash
./job.sh vx q2mom [0,1]
```

for the momentum dependent $q^2$ smearing, for simulation or data (1, 0).

## Getting the calibration files for nom and nSPD

the outputs will need to be merged with:

```bash
python python/merge.py -i vx -o vy
```

where `vx` is the version described before and `vy` is the version of the calibration files. Comparisons between versions can be got with:

## Getting the calibration files for momentum dependence systematics

Run:

```bash
python python/dump_ratios.py -v vx
```

to produce JSON files and ROOT files with the $\sigma_{data}/\sigma_{MC}$ ratios. 

## Plot resolution maps

```bash
python python/plot_resolutions.py -v vx
```

## Making comparison plots

```bash
python python/compare.py -v v1 v2 v3...
```

where the versions are the versions of the calibration files (e.g. v1.nom).
