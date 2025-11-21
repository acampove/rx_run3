# Environment and installation

The following variables _can_ be defined:

`ANADIR`: Where outputs will be saved. If not specified, outputs will go to `/tmp`

## Without access to DIRAC 

One can just:

- Install this project:
```bash
pip install ap-utilities
````

But one would not be able to check for samples existence this way.

## With access to DIRAC 

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

