[TOC]

# Description

What is below is meant for the IHEP cluster and will not be used, because the slimming
is already done in grid jobs and the resulting ntuples are small enough to be used as they are.

## Applying selection to produce slimmed ntuples 

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

Different institutes will have different mechanisms to submit jobs (i.e. HTCondor, Slurm, Torque...). Thus, different submission scripts
are needed and each institute will have to write the corresponding script.

To support more sites, one should:

- Write a `job_sel_xxx` script.
- Add it to `pyproject.toml`
- Document its usage.
- Open a merge request.

## With jobs

### Submission

Run:

```bash
job_sel_ihep -d /publicfs/ucas/user/campoverde/Data/RX_run3/v1/post_ap -s Bu_JpsiK_mm_eq_DPC -q central -t Hlt2RD_BuToKpMuMu_MVA -p RK -n 1000 -r q2-bdt
```

which will apply the selection by:

- Picking up the data found in `/publicfs/ucas/user/campoverde/Data/RX_run3/v1/post_ap`
- Applying the selection on the sample `Bu_JpsiK_mm_eq_DPC`
- For the HLT2 trigger `Hlt2RD_BuToKpMuMu_MVA`
- For the project `RK`
- Using 1000 jobs
- Skipping the `q2` and `bdt` cuts.

The options are:

```bash
Script used to submit jobs to the IHEP's HTCondor cluster. The jobs should apply selection.

-d: Path to YAML file containing sample list
-s: Name of the sample (subdirectory) to run over
-r: String with dash separated cuts to remove, e.g. q2-bdt, optional
-q: q2 bin, e.g. central
-t: HLT2 trigger, e.g Hlt2RD_B0ToKpPimEE
-p: Project, e.g. RK
-n: Number of parts into which selection is split, e.g. 100
-m: Memory required by job, default 4000Mb
-T: Will run a test with the first job (default) or will submit all the jobs
-Q: Queue, e.g. test (5m), short (30m), mid (10h), by default short
```

by default, it will run a test job.

### Job list

Given the large number of samples to process a utility was created to provide the list of commands, i.e. a list like:

```
job_sel_ihep -T 0 -d /publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml -s Bu_Kee_eq_btosllball05_DPC   -q central -t Hlt2RD_BuToKpEE_MVA   -p RK -n 10 -r q2-bdt
job_sel_ihep -T 0 -d /publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml -s Bu_Kee_eq_btosllball05_DPC   -q central -t SpruceRD_BuToHpEE     -p RK -n 10 -r q2-bdt
job_sel_ihep -T 0 -d /publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml -s Bu_Kee_eq_btosllball_DPC     -q central -t Hlt2RD_BuToKpEE_MVA   -p RK -n 10 -r q2-bdt
job_sel_ihep -T 0 -d /publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml -s Bu_Kee_eq_btosllball_DPC     -q central -t SpruceRD_BuToHpEE     -p RK -n 10 -r q2-bdt
job_sel_ihep -T 0 -d /publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml -s Bu_Kmumu_eq_btosllball05_DPC -q central -t Hlt2RD_BuToKpMuMu_MVA -p RK -n 10 -r q2-bdt
job_sel_ihep -T 0 -d /publicfs/ucas/user/campoverde/Data/RX_run3/v4/rx_samples.yaml -s Bu_Kmumu_eq_btosllball05_DPC -q central -t SpruceRD_BuToHpMuMu   -p RK -n 20 -r q2-bdt
```

Run the following:

```bash
make_jobs_list -v v1
```

which for the version `v1` of the `post_ap` ntuples, will automatically:

- Find which samples exist and for which triggers
- Assign a number of jobs for each of these samples, based on the number of entries

Some samples, with too few entries, will be skipped altogether. 
This is expected to take time, given that, in order to calculate the number of events, each file needs
to be opened.

### Held jobs

To automate resubmission of HTCondor jobs that run out of memory do:

```bash
jmanager_ihep -m 8000 -t 0
```

which will resubmit held jobs with 8GB (`-m`) and not as a test (`-t`).
