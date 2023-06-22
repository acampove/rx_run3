# q2 systematics

This project is used to assess the systematics associated to q2 smearing. 

# Installation

Make a virtual environment like:

```bash
mamba create -n q2_systematics root=6.28 python=3.9
```

# Usage

The outputs will go to the directory pointed by the enviroment variable `QSQSYS` and the directory structure should look like:

```bash
├── get_q2_resolutions
│   ├── data
│   │   ├── json
│   │   │   └── v1.mom
│   │   └── plots
│   │       └── v1.mom
│   ├── mc
│   │   ├── json
│   │   │   └── v1.mom
│   │   └── plots
│   │       └── v1.mom
│   ├── plots
│   │   └── v1.mom
│   │       ├── ETOS_2017
│   │       ├── ETOS_2018
│   │       ├── ETOS_r1
│   │       ├── ETOS_r2p1
│   │       ├── GTIS_2017
│   │       ├── GTIS_2018
│   │       ├── GTIS_r1
│   │       └── GTIS_r2p1
│   └── ratio
│       └── v1.mom
│           ├── ETOS_2017
│           ├── ETOS_2018
│           ├── ETOS_r1
│           ├── ETOS_r2p1
│           ├── GTIS_2017
│           ├── GTIS_2018
│           ├── GTIS_r1
│           └── GTIS_r2p1
├── get_q2_tables
│   └── fits
│       ├── v1.nom
│       ├── v1.nspd
│       └── v2.nspd
└── version_comparison
    └── v2pnom_v3pnom
```

once the scripts have been ran.

## Nominal and nSPD binning systematics

In order to get the smearing factors run:

```bash
get_q2_tables -v vx -t [ETOS,GTIS] -y [2011,2012...] -b [0,1,2] -x [nom, nspd]
```

with `vx` as the name of the directory where output will go. Other options for running over specific years can be checked with:

```bash
get_q2_tables -h
```
The code will:

1. Load trees and apply a selection both to MC and data.
2. Fit MC.
3. Fit data with shapes fixed to simulation, except for mean and width.
4. Extract the parameters and store them.

## Momentum dependent systematics

Run:

```bash
get_q2_resolutions -d 2017 -t GTIS -b 0 -v v8 -s
```

to run the fits to the simulation, for a given dataset, trigger and brem category, for data drop `-s`. 
The version is only used to name the output directory.

## Jobs

However, to speed things up, one can send the jobs to the cluster to run in parallel for each dataset with:

```bash
job_q2 -v vx -k q2sys -t (nom, nspd) [-r 1]
```
for nominal and `nSPD` systematics and:

```bash
job_q2 -v vx -k q2mom -t (0,1)       [-r 1]
```

for the momentum dependent $q^2$ smearing, for simulation or data (1, 0). The simulation (`-t 1`) have to be submitted before the data
(`-t 0`) jobs.

## Failed fits

In case of failed fits do:

### Momentum dependence systematics

The fits are checked by placing the plots in beamer slides. You will know from them the brem category, year and trigger, as well as 
the bin for which the fit failed. The fits are redone using:

```bash
get_q2_resolutions -d r1 -t GTIS -b 0 -v v7 -s -i 17
```

where `-i 17` targets only the given bin.

## Getting the calibration files for nom and nSPD

The outputs will need to be merged with:

```bash
q2_merger -s nom -i vx -o vy -y 2018 -d $CALDIR
```

where `vx` is the version described before and `vy` is the version of the calibration files, if the year is not specified it will run over everything and send it to `$CALDIR`. Comparisons between versions can be got with:

## Getting the calibration files for momentum dependence systematics

Run:

```bash
dump_q2_ratios -v vx
```

to produce JSON files and ROOT files with the $\sigma_{data}/\sigma_{MC}$ ratios. 

## Plot resolution maps

```bash
plot_resolutions -v vx
```

## Making comparison plots

```bash
compare_q2 -v v1 v2 v3...
```

where the versions are the versions of the calibration files (e.g. v1.nom).

## Validation

To overlay the signal component in the fit to the data with the simulation before and after the smearing do:

```bash
validate_smear -v v3.nom
```
