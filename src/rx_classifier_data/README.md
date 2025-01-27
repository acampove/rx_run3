# Description

Here is where the configuration files used to train MVA classifiers for the RX analysis are placed

- **v1**: Version using only cut based HLT lines, presented in September 2024
- **v2**: Version trained with rare modes, both MVA turbo and sprucing lines
- **v2px**: Each of these versions trains the partially reconstructed MVA with a different cone size, represented by `x`.
These MVAs were trained with:

```bash
#!/usr/bin/env bash

train_cone()
{
    ANGLE=$1

    train_classifier -v $ANGLE -c train_turbo_mva_b12_prc -q low
    train_classifier -v $ANGLE -c train_turbo_mva_b12_prc -q central
    train_classifier -v $ANGLE -c train_turbo_mva_b12_prc -q high
}

train_cone v2p10
train_cone v2p20
train_cone v2p25
train_cone v2p30
```
