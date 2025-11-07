'''
Module with tests for classes in specification.py module
'''

from pathlib               import Path

from ROOT import RDataFrame
from rx_data.specification import Sample

# ----------------------
def _make_file(index : int, tmp_path : Path) -> Path:
    '''
    Parameters
    -------------
    index: Index of the file that needs to be created
    tmp_path: Directory where temporary file will go

    Returns
    -------------
    Path to test file
    '''
    rdf = RDataFrame(10)
    rdf = rdf.Define('a', '1')

    path = tmp_path / f'file_{index:03}.root'
    rdf.Snapshot('tree', str(path))

    return path
# ----------------------
def _get_sample(tmp_path : Path) -> Sample:
    '''
    Returns
    -------------
    Instance of Sample
    '''
    paths = [ _make_file(index=index, tmp_path=tmp_path) for index in range(4) ]
    data  = {'files' : paths, 'trees' : ['tree']}

    return Sample(**data)
# ----------------------
def test_sample(tmp_path : Path):
    sample = _get_sample(tmp_path = tmp_path)

    assert sample.size == 40
