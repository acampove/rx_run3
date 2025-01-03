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

In the cluster, one will need a dedicated directory for the log files, this is specified by exporting the variable `$JOBDIR`, i.e.:

```bash
export JOBDIR=/path/to/place/here/log/files/are
```

then run:

```bash
job_sel -d /home/acampove/Data/rx_samples/v1/post_ap -s data_24_magdown_24c2 -q central -t Hlt2RD_B0ToKpPimMuMu -p RK -n 100 -r q2-bdt
```

which will send 100 jobs to do the selection. The options are:

```bash
Script used to setup mamba environment and run apply_selection script

-d: Path to directory containing directory samples
-s: Name of the sample (subdirectory) to run over
-r: String with dash separated cuts to remove, e.g. q2-bdt, optional
-q: q2 bin, e.g. central
-t: HLT2 trigger, e.g Hlt2RD_B0ToKpPimEE
-p: Project, e.g. RK
-n: Number of parts into which selection is split, e.g. 100
-T: Will run a test with the first job (default) or will submit all the jobs
-Q: Queue, e.g. test (5m), short (30m), mid (10h), by default short
```

