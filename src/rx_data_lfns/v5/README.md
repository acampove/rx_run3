# Description

This version of the ntuples contains data and MC for all the blocks:

- Added preselection to reduce size of rx trigger trees **only** for RK trees (__Hlt2RD_Bu*__).
The preselection used was:

- Added extra branches for HOP variables calculation

# Contents

Branches added are:

- **B_BPVX/Y/Z:** Position of the B primary vertex 
- **B_END_VX/Y/Z:** Position of the B vertex 

# On the YAML file

## rx_samples.yaml 

Contains the same as the JSON files but:

- Organized in a tree structure, such that the user can build dataframes more easily
- We dropped the non-rk/rk* trees, only the __Hlt2RD_Bu*__ and __Hlt2RD_B0*__ DecayTree triggers are present.

## rk_samples.yaml 

Same as above but only with  the triggers used for RK.

## rk_mva.yaml

These are small trees with the MVA scores and only for RK. They can be attached to the trees in `rk_samples.yaml`
in order to add the combinatorial and prec scores to the main trees through friend trees.
