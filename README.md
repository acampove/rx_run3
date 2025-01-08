# Classifier

This project is meant to be used to make classifiers for the RX analyses

## Training

For that run:

```bash
train_classifier -v v1 -c train_cut_bukee_2024
train_classifier -v v1 -c train_cut_bdksee_2024
```

Which will train a classifier using the settings in the `train_cut_bukee_2024.yaml` file.
To limit the number of training entries to e.g. 10000 do `-m 10000`. Both files are expected to
be in the `rx_classifier_data/v1` directory.

