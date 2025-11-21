# $L_{b}\to pK\mu\mu$ jobs

```bash
mkdir tmp
touch tmp/jobs.sh
```

put the following in `job.sh`:

```bash
#!/usr/bin/env bash


send_test_job()
{
    SAMPLE=$1
    JBNAME=$SAMPLE"_"$TEST_INDEX
    job_filter_ganga -n $JBNAME -p rd_ap_2024 -s $SAMPLE -c $CFG_VER -b Dirac -v $ENV_VER -t
}

send_job()
{
    SAMPLE=$1
    JBNAME=$SAMPLE
    job_filter_ganga -n $JBNAME -p rd_ap_2024 -s $SAMPLE -c $CFG_VER -b Dirac -v $ENV_VER
}

send_tests_mc()
{
    send_test_job test_w35_37_v1r2266_mm
    send_test_job test_w31_34_v1r2266_mm
    send_test_job test_w37_39_v1r2266_mm
}

send_tests_dt()
{
    send_test_job test_dt_c1
    send_test_job test_dt_c2
    send_test_job test_dt_c3
    send_test_job test_dt_c4
}

send_dt()
{
    send_job data_turbo_24c1
    send_job data_turbo_24c2
    send_job data_turbo_24c3
    send_job data_turbo_24c4
}

send_mc()
{
    send_job w31_34_v1r2266
    send_job w35_37_v1r2266
    send_job w37_39_v1r2266
    send_job w40_42_v1r2266
}

ENV_VER=037
CFG_VER=v1
TEST_INDEX=001

send_tests_dt
send_tests_mc

send_dt
send_mc
```

where only the first two lines need to be updated according to the user's virtual environment version and the location of their config.
Comment the actual submissions and send the test jobs first.

Submit the jobs by `source tmp/job.sh`
