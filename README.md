# Description

This is a set of scripts and config files meant to be used to make plots of several
kinematic distributions.

## 1D plots 

### cutflow 

The `cutflow` utility will make plots of a dataset under multiple cuts, one after the other.
Use it with:

```bash
cutflow -q jpsi -s DATA* -t Hlt2RD_BuToKpEE_MVA -c cleanup
```

where `cleanup` refers to `cleanup.yaml`, a config file that shows the effect of applying cleanup
requirements (HOP, Cascade, $J/\psi$ mis ID veto). Below each config description:

| Kind    | Description                                            |
| ------- | ------------------------------------------------------ |
| vetoes  | Check effect on `B_M` original and corrected of vetoes |
| bdt     | Check effect on `B_M` original and correcte of BDT     |
| cleanup | Same as vetoes but checks effect on multiple variables |

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

