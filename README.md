# Classifier

This project is meant to be used to make classifiers for the RX analyses

## Training

For that run:

```bash
train_classifier -v v2 -c train_turbo_mva_b12  -q high
train_classifier -v v2 -c train_spruce_mva_b12 -q high
```

Which will train a classifier using the settings in the `train_cut_bukee_2024.yaml` file.
To limit the number of training entries to e.g. 10000 do `-m 10000`. Both files are expected to
be in the `rx_classifier_data/v1` directory. The `-q` argument controls the q2 bin, which can be
`low`, `central`, `jpsi`, `psi2S` and `high`.
