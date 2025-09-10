# Description

These files are used to configure the slimming and filtering of ntuples

## v1

First version to filter rd_2024_ap data

## v2

- Adding PT and P definitions for all particles 

## v3

- Adding versions field instead of version. This is now a dictionary mapping sample with version
- Adding MC from v1r2041

## v4

Fixes:

- Adding q2 and angle between tracks definitions
- Adding definitions for Hp branches, which were missing and are using in sprucing lines

## v5

Adding isolation branches needed for PRec BDT training

## v6

Used to send all blocks of data and MC

## v7

Adding preselection and branches needed for HOP variables

## v8

Dropping cut on lepton kinematics

## v9

Doing only RK lines and not removing branches

## v10

- Dropping PIDe cut on electrons, processes only `Hlt2RD_BuToKpEE_MVA` line. 
- Can be used for applying properly PIDe cut, with brem corrected electrons.

## v11

Using all electron RK lines, i.e.:

- Hlt2RD_BuToKpEE_MVA
- Hlt2RD_BuToKpEE_SameSign_MVA
- Hlt2RD_BuToKpEE_MVA_cal
- Hlt2RD_BuToKpEE_MVA_misid

Can be additionally used to stdy mis-ID backgrounds and PIDe calibration.

## v12

Same as v11, but removing altogether PID selection for electrons and Kaon. Needed to do pass-fail

## v13

This should be the last set of samples for RK, includes

- Signal samples for blocks 7 and 8, not processed before
- Inclusive muon samples 
