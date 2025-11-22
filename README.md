# $R_X$ run3

This is the repository for the combined measurement of $R_K$ and $R_{K^*}$
in run 3.

Find in the following links:

- [Installation instructions](doc/installation.md)

## Analysis packages

This project contains the following packages:

rx_data        : Allows access to the data and also has tools for further processing
rx_efficiencies: Used to calculate geometric acceptances and reconstruction efficiencies


## Development packages

In order to make developing the code easier it is advised to use type anotations
with an LSP server, e.g. ruff and pyright. These server won't work unless the upstream
projects have also type annotations. That is not the case often, but annotations can
be added through stub packages. This project includes, zfit-stubs, vector-stubs 
and root_stubs.

