# Description

This is a set of scripts and config files meant to be used to make plots of several
kinematic distributions.

## generic

This utility will make generic plots.

### cleanup

By using `cleanup` as the config one will get plots of variables used to
remove background, e.g.:

```bash
generic -t Hlt2RD_BuToKpMuMu_MVA -s "DATA*" -c cleanup -q high
```

Which will use the `cleanup.yaml` config to make plots in the high q2 region
of samples containing "DATA" in the name and belonging to the
`Hlt2RD_BuToKpMuMu_MVA` trigger. The config lives in `rx_plotter_data`

for different MC samples one can do:

```bash
generic -t Hlt2RD_BuToKpEE_MVA -s Bu_Kee_eq_btosllball05_DPC -c cleanup -q central
```

### Block quality

One can use the same command with `block_quality`, e.g:

```bash
generic -t Hlt2RD_BuToKpMuMu_MVA -s "DATA*" -c block_quality -q jpsi 
```

which will make plots showing distributions per block. These distributions pass
a tight MVA requirement and are taken from the core of the DTF B mass. Thus can be
considered background subtracted.
