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
    CONF=$2

    train_classifier -v $ANGLE -c $CONF -q low
    train_classifier -v $ANGLE -c $CONF -q central
    train_classifier -v $ANGLE -c $CONF -q high
}

train_cone v4 train_turbo_mva_cmb
train_cone v4 train_turbo_mva_prc
```

- **v5**: Dropped Iso mass variables
- **v6**: Adding back ISO mass variables in PRec MVA and using new version of code that
    1. Makes better plots
    2. Adds the ROC curve with the full testing dataset

The following versions use brem corrected variables and the following changes
for high q2.

| Version | $q^2$ Cut Type | Background Proxy                |
| ------- | -------------- | ------------------------------- |
| v7p0    | $q^2$          | OS data (default)               |
| v7p1    | $q^2_{track}$  | OS data (default)               |
| v7p2    | $q^2_{dtf}$    | OS data (default)               |
| v7p3    | $q^2_{dtf}$    | SS data                         |
| v7p4    | $q^2$          | SS data                         |
| v7p5    | $q^2$          | SS data no upper SB             |
| v7p6    | $q^2$          | v7p0 better hyper parameters    |
| v7p7    | $q^2$          | Trained on electron MC for Prec |
