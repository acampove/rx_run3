'''
Entry point for testing CLI utilities
'''

import typer
import subprocess

from pathlib        import Path
from ihep_utilities import JobSubmitter
from rx_tests       import TestConfig
from rx_tests       import TestJobs
from dmu            import LogStore

app = typer.Typer(help=__doc__)
log = LogStore.add_logger('rx_tests:cli')
# ----------------------
@app.command()
def show_report(
    browser: str= typer.Option('firefox', '--browser', '-b'),
    path : Path = typer.Option(..., '--path', '-p')) -> None:
    '''
    Picks path to XML files, merges them, converts them to HTML
    and shows report in browser
    '''
    xml_files = list(path.glob('*.xml'))
    if not xml_files:
        typer.echo(f"No XML files found in {path}")
        raise typer.Exit(1)

    merged = Path('/tmp/merged.xml')
    report = Path('/tmp/report.html')

    subprocess.run(['junitparser', 'merge', *xml_files, str(merged)], check=True)
    subprocess.run(['junit2html', str(merged), str(report)], check=True)
    subprocess.run([browser, str(report)], check=True)
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
