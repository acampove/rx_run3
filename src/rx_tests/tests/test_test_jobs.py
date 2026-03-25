'''
Module meant to test TestJobs class
'''

from rx_tests import TestJobs
from rx_tests import TestConfig

# --------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    cfg = TestConfig.from_package(
        package  = 'rx_tests_data',
        file_path= 'config.yaml')

    obj = TestJobs(cfg = cfg)
    jobs = obj.get_jobs()

    assert isinstance(jobs, dict)
    assert jobs

    for job, commands in jobs.items():
        print(job)
        for command in commands:
            print(command)
# --------------------------------------
