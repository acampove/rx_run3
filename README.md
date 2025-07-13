# Fitter

This project is meant to automate:

- Building of models
- Fits to MC 
- Fits to data
- Creation of validation plots, tables, etc
- Application of constraints
- Obtention of fitting information, specially:
    - Mass scales, resolutions
    - Signal yields for resonant and rare modes

And be used as an input for:

- Systematics evaluation
- Extraction of $R_K$ and $R_K^*$
- Calibrations

Ideally with minimal changes for new observables.
The class is organized as in the diagram below:

[](doc/images/fitter.png)

This project makes heavy use of caching through the [Cache](https://github.com/acampove/dmu?tab=readme-ov-file#caching-with-a-base-class)
class and thus:

- It allows for fast reruns
- It allows for the fitters to also be used to retrieve parameters

