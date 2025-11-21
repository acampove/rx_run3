# Pick only samples that do not exist as ntuples

Currently there are 241 AP jobs associated to the MC. Only 35 are missing, in order to find out
what ntupling jobs need to be send do:

```bash
find_in_ap
```

which will:

- Read the `info.yaml` files created above
- Check, using `apd`, what samples are missing
- Create a `info.yaml` in the current directory, only with those samples.

