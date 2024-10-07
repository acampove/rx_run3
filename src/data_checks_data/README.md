[[_TOC_]]

# Description

These are YAML and TOML config files used for different reasons:

## AP

**dt_2024_turbo_comb.toml**: Configs used to filter data ntuples coming from AP.
**mc_2024_turbo_comb.toml**: Configs used to filter mc ntuples coming from AP.

## Linking and merging of files

**link_confg.yaml**: By the `link_merge` utility to assign readable directory name from event type.

## Plotting before selections

**bukkee_opt.toml** Used to make basic data/MC comparison plots from files before selections
Should be used with:

```bash
plot_vars -y 2024 -v v2 -c bukee_opt -d data_ana_cut_bp_ee:Data ctrl_BuToKpEE_ana_ee:Simulation
```

## Plotting after selections


