PID correction of simulation
===

### Introduction

The following set of scripts aims to do two main things:
- Create the histograms that parametrize the efficiency of specific PID cuts for muons, kaons and pions both in Data and MC
- Attach these efficiencies as weights to the MC samples, to then use their ratio to correct the simulation.

### Creating the histograms

A preliminary version of the histograms can be produce by running:

```bash
# for tests run over one file only with -M 1
create_pid_maps -c v1 -b v1 -s b1 -p Pi -o /output/directory
```

where:

```
options:
  -h, --help            show this help message and exit
  -c CFG_VERS, --cfg_vers CFG_VERS
                        Version of configuration file
  -b BIN_VERS, --bin_vers BIN_VERS
                        Version of binning file
  -p {e,Pi,K,Mu,P}, --particle {e,Pi,K,Mu,P}
                        Particle name
  -s SAMPLE, --sample SAMPLE
                        Sample
  -o OUT_DIR, --out_dir OUT_DIR
                        Directory where pkl files will go
  -d, --dry-run         Enable dry-run mode (default: False)
  -M MAXFILES, --maxfiles MAXFILES
                        Limit number of files to this value
  -v, --verbose         Will print debug messages
```

the config and binning files are in:

```
rx_pid_data/config
rx_pid_data/binning
```

in versioned `yaml` and `json` files respectively.
In the config files the samples are nicknamed as:

```yaml
b1  : 2024_WithUT_block1_v1
b2  : 2024_WithUT_block2
b3  : 2024_WithUT_block3
b5  : 2024_WithUT_block5
b6  : 2024_WithUT_block6
b7  : 2024_WithUT_block7
b8  : 2024_WithUT_block8
```

and the `b*` key is what the `-s` flag should get.

### Cluster jobs

In order to make these maps with jobs sent to the cluster do:

- Move to LXPLUS
- Copy to an empty directory both submission and running scripts in 
`src/rx_pid_data/jobs`
and
`src/rx_pid_data_scripts`
respectively.

```bash
chmod +x make_pid_maps

# Modify the line OPATH to point to the user's directory where the outputs will go

# The log files will be placed here, the directory has to exist or else the jobs will be held
mkdir logs/

condor_submit job.sub
```

the jobs can be monitored with `watch condor_q`

### Plotting

To make plots do:

```bash
plot_histograms -d /directory/where/pickle/files/are
```

and the utility will loop over all the `pkl` files plotting the histograms inside.
