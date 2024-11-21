# ap_utilities

This project holds code needed to transform the AP used by the RD group into something that makes ntuples with MVA HLT triggers.

## Check for samples existence 

Given a set of MC samples specified in a YAML file like:

```YAML
settings:
  year      : 2024
  mc_path   : 2024.W31.34
  polarity  : MagUp
  nu_path   : Nu6.3
  sim_vers  : Sim10d
  generator : Pythia8
  ctags     : sim10-2024.Q3.4-v1.3-mu100
  dtags     : dddb-20240427
event_type :
  B2K1          : 12425000  
  B2K2          : 12425011  
  Bu2Kstee      : 12123445  
  Bd2Kpiee      : 11124037  
```

run:

```bash
check_samples -i samples.yaml -n 6
```

to check if the samples exist using 6 threads (default is 1)  and store them in `samples_found.yaml`

To run this one has to be in an environment with:

1. Access to DIRAC.
1. A valid grid token.

## Specific to MVA lines in the RD group

### Add lines to `Config.py`

To do that run:

```bash
transform_text -i Config.py -c conf_rename.toml
```

which will create an `output.py` file with the replacements specified in `hlt_rename.toml`.

### Add lines to `main.py`

To do that run:

```bash
transform_text -i main.py  -c main_rename.toml
```

