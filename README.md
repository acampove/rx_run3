# Classifier

This project is meant to be used to make classifiers for the RX analyses

## Training

For that run:

```bash
train_classifier -v v2 -c train_turbo_mva_b12_prc -q low
train_classifier -v v2 -c train_turbo_mva_b12_prc -q central
train_classifier -v v2 -c train_turbo_mva_b12_prc -q high

train_classifier -v v2 -c train_turbo_mva_b12_cmb -q low
train_classifier -v v2 -c train_turbo_mva_b12_cmb -q central
train_classifier -v v2 -c train_turbo_mva_b12_cmb -q high
```

Which will train the classifiers for different $q^2$ bins and with different
settings. The settings are part of `YAML` files and are stored in:

```bash
rx_classifier/src/rx_classifier_data/v2
```

The underlying tool doing all the training can be found in 
[dmu](https://github.com/acampove/dmu?tab=readme-ov-file#machine-learning)

## Output

The output of this training are, for each fold:

- Pickle file with the classifier
- ROC curve
- Importance table
- Classifier score
- Hyper-parameters table
- Plots of the features
- Covariance matrix

## TODO

Some things that can be tried are:

- The hyper-parameters need to be optimized
- The working point needs to be optimized
- One could train with alternative datasets, like the muon channel, given the low statistics.
- One could also merge new datasets to improve the training.
- One could try MLPs or other type of classifiers.

