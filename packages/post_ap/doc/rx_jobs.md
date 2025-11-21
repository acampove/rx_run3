# $R_{X}$ filtering jobs

```bash
mkdir tmp
touch tmp/jobs.sh
```

put the following in `job.sh`:

```bash
#!/usr/bin/env bash

# ---------------------------------------
# Tests section: These should be run before main jobs
# ---------------------------------------
send_test_job()
{
    SAMPLE=$1
    job_filter_ganga -n $SAMPLE"_"$TEST_INDEX -p rd_ap_2024 -s $SAMPLE -f $CFG -b Dirac -v $VENV -t
}

send_tests_ee_mc()
{
    send_test_job test_w31_34_v1r2266_ee
    send_test_job test_w35_37_v1r2266_ee
    send_test_job test_w37_39_v1r2266_ee
    send_test_job test_w40_42_v1r2266_ee

    send_test_job test_w31_34_v1r2437_ee
    send_test_job test_w35_37_v1r2437_ee
    send_test_job test_w37_39_v1r2437_ee
    send_test_job test_w40_42_v1r2437_ee
}

send_tests_mm_mc()
{
    send_test_job test_w31_34_v1r2266_mm
    send_test_job test_w35_37_v1r2266_mm
    send_test_job test_w37_39_v1r2266_mm
    send_test_job test_w40_42_v1r2266_mm

    send_test_job test_w31_34_v1r2437_mm
    send_test_job test_w35_37_v1r2437_mm
    send_test_job test_w37_39_v1r2437_mm
    send_test_job test_w40_42_v1r2437_mm
}

send_tests_data()
{
    send_test_job test_dt_c1
    send_test_job test_dt_c2
    send_test_job test_dt_c3
    send_test_job test_dt_c4
}
# ---------------------------------------
# Actual jobs
# ---------------------------------------
send_job()
{
    SAMPLE=$1
    job_filter_ganga -n $SAMPLE -p rd_ap_2024 -s $SAMPLE -f $CFG -b Dirac -v $VENV 
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
    # ----
    send_job w31_34_v1r2437
    send_job w35_37_v1r2437
    send_job w37_39_v1r2437
    send_job w40_42_v1r2437
}
# ---------------------------------------
VENV=037
CFG=/home/acampove/Packages/post_ap/src/post_ap_data/post_ap/rx/v11.yaml
TEST_INDEX=001

send_tests_ee_mc
send_tests_mm_mc
send_tests_data

#send_dt
#send_mc
```

where only the first two lines need to be updated according to the user's virtual environment version and the location of their config.
Comment the actual submissions and send the test jobs first.

Submit the jobs by `source tmp/job.sh`
