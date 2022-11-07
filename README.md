# q2_systematics

This project is used to assess the systematics associated to q2 smearing. In order to get the smearing factors run:

```bash
python python/get_tables.py -v vx
```
with `vx` as the version of the weights wanted. Other options for running over specific years can be checked with:

```bash
python python/get_tables.py -h
```
The code will:

1. Load trees and apply a selection both to MC and data.
2. Fit MC.
3. Fit data with shapes fixed to simulation, except for mean and width.
4. Extract the parameters and store them.

