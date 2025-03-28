# Description

This is a set of scripts and config files meant to be used to make plots of several
kinematic distributions.

## 1D plots 

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

## 2D plots

For this run:

```bash
plot_2d -C mm -q central -c mass_q2
```

where:

```
options:
  -h, --help            show this help message and exit
  -C {ee,mm}, --chanel {ee,mm}
                        Channel
  -q {low,central,jpsi,psi2,high}, --q2bin {low,central,jpsi,psi2,high}
                        q2 bin, optional
  -c {mass_q2,cmb_prc}, --config {mass_q2,cmb_prc}
                        Settings, i.e. mass_q2
  -l {10,20,30}, --loglvl {10,20,30}
                        Log level
```

This will provide plots showing 

- Correlations
- Mass vs q2

