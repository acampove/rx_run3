from prefect            import flow, task
from rx_plotter_scripts import compare

# ----------------------
@task
def _run_compare() -> None:
    compare.main(argv=[
        "-q", "jpsi",
        "-s", "Bu_JpsiK_ee_eq_DPC",
        "-t", "Hlt2RD_BuToKpEE_MVA",
        "-c", "resolution",
        "-b", "2",
        "-B", "2",
    ])
# ----------------------
@flow
def plotting_flow():
    '''
    Entry point
    '''
    _run_compare()
# ----------------------
if __name__ == '__main__':
    plotting_flow()
