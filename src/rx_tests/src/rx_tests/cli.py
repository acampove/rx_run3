'''
Entry point for testing CLI utilities
'''

import typer
from ihep_utilities import JobSubmitter
from rx_tests       import TestConfig
from rx_tests       import TestJobs
from dmu            import LogStore

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_tests:cli')
# ----------------------
@app.command()
def dummy():
    pass
# ----------------------
@app.command()
def test_all(dry_run : bool = typer.Option(False, '--dry_run', '-d')) -> None:
    '''
    Runs all tests as configured in rx_tests_data/config.yaml
    '''
    cfg = TestConfig.from_package(
        file_path = 'config.yaml',
        package   = 'rx_tests_data')
    
    tjb = TestJobs(cfg = cfg)
    jobs= tjb.get_jobs()
    
    sbt = JobSubmitter(
        jobs        = jobs, 
        environment = 'rx_run3')
    sbt.run(skip_submit=dry_run)
# ----------------------
if __name__ == '__main__':
    app()
