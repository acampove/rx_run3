# q2 systematics

This project is used to assess the systematics associated to q2 smearing. In order to get the smearing factors run:

## Nominal and nSPD binning systematics

```bash
python python/get_tables.py -v vx -t [ETOS,GTIS] -y [2011,2012...] -b [0,1,2] -x [nom, nspd]
```

with `vx` as the name of the directory where output will go. Other options for running over specific years can be checked with:

```bash
python python/get_tables.py -h
```
The code will:

1. Load trees and apply a selection both to MC and data.
2. Fit MC.
3. Fit data with shapes fixed to simulation, except for mean and width.
4. Extract the parameters and store them.

## Momentum dependent systematics

Run:

```bash
python python/get_resolutions.py -d 2017 -t GTIS -b 0 -v v8 -s
```

## Jobs

However, to speed things up, one can send the jobs to the cluster to run in parallel for each dataset with:

```bash
./job.sh vx
```

## Getting the calibration files

the outputs will need to be merged with:

```bash
python python/merge.py -i vx -o vy
```

where `vx` is the version described before and `vy` is the version of the calibration files. Comparisons between versions can be got with:

## Making comparison plots

```bash
python python/compare.py -v v1 v2 v3...
```

where the versions are the versions of the calibration files (e.g. v1.nom).
