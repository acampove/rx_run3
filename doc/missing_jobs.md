In case problems happen with Ganga, the jobs can still be retrieved
by using the Dirac job ID:

- In the Dirac webpage get the list of job IDs in a CSV file.
- Run:

```bash
lfns_from_csv -f jobid.csv
```

in a system with access to EOS, e.g. LXPLUS. This will search the job 
output and provide the LFNs in a JSON file.
