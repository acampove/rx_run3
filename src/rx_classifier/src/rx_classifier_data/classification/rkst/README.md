# Description

These are the config files used to train the classifiers for $R_{K^{*0}}$

# v1

- Uses all variables from RX analysis
- Hyperparameters taken as RK run3 v7.7 model
- Kinematics were not recalculated for brem correction

# v2

Combinatorial and PRec

- Kinematics were recalculated with brem correction

# v3

PRec:

- Hyperparameters were tuned to reduce overtraining
- Remove the least important variables
- Remove Kstar mass cut and BDT

# v4

PRec:

- Add HOP mass cut
