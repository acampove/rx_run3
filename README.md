# $R_X$ orchestration

This project is meant to chain all the subtasks in the $R_X$ project.
The dependencies are listed in the `pyproject.toml` and are meant to
be taken from gitlab's master branch, where the latest valid version should be.

# Installation

Run

```bash
git clone ssh://git@gitlab.cern.ch:7999/rx_run3/rx_orchestration.git
pip install rx_orchestration
```

which should install all the `RX` projects

# Usage

## Snakemake

In the main folder there is a `Snakemake` file and run:

```bash
Snakemake -c 8 all
```

## Prefect

To run the comparisons between original and brem corrected line-shape do:

```bash
cutflow_wf
```

## L(uigi) A(nalysis) W(orkflow)

All the remaining workfows are implemented with LAW due to its support for HTCondor.

### Plotting

These are workflows associated to the `rx_plots` project.

| Command       | Description                                                                 |
|---------------|-----------------------------------------------------------------------------|
| `cutflow_law` | Creates plots for the effect of cuts on different variables                 |
| `compare_law` | Compares the effect of:<br>• Brem correction on resolution<br>• Smearing MC due to scale and resolution differences with data |
| `plot_2d`     | Plots: <br>• Mass vs $q^2$ for data with and without the brem correction|

### Training of classifiers

These workflows are associated to the `rx_classifier` project.

```bash
train_law
```

trains the classifiers, the config is in:

`configs/rx_classifier/train.yaml`

