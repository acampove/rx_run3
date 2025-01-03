# $R_X$ selection

This project is meant to apply an offline selection to ntuples produced by
[post_ap](https://github.com/acampove/post_ap/tree/main/src/post_ap_scripts)
and downloaded with
[rx_data](https://github.com/acampove/rx_data).
the selection is done with jobs sent to an HTCondor cluster.

## Usage

For local tests one can use `apply_selection` as in:

```bash
apply_selection -d /home/acampove/Data/rx_samples/v1/post_ap -s data_24_magdown_24c2 -r q2 bdt -q central -t Hlt2RD_B0ToKpPimMuMu -p RK -i 0 -n 900
```

where:

```bash
usage: apply_selection [-h] -i IPART -n NPART -d IPATH -s SAMPLE -p PROJECT -q Q2BIN -t HLT2 [-c CUTVER] [-r REMOVE [REMOVE ...]]

Script used to apply selection and produce samples for Run 3

options:
  -h, --help            show this help message and exit
  -i IPART, --ipart IPART
                        Part to process
  -n NPART, --npart NPART
                        Total number of parts
  -d IPATH, --ipath IPATH
                        Path to directory with subdirectories with samples
  -s SAMPLE, --sample SAMPLE
                        Name of sample to process, e.g. data_24_magdown_24c2
  -p PROJECT, --project PROJECT
                        Name of project, e.g RK, RKstr
  -q Q2BIN, --q2bin Q2BIN
                        q2 bin, e.g. central
  -t HLT2, --hlt2 HLT2  Name of HLT2 trigger, e.g. Hlt2RD_B0ToKpPimMuMu
  -c CUTVER, --cutver CUTVER
                        Version of selection, by default, latest
  -r REMOVE [REMOVE ...], --remove REMOVE [REMOVE ...]
                        List of cuts to remove from the full selection
```
