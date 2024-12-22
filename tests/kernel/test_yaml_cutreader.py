'''
Module with functions needed to test YamlCutReader python interface
'''

from importlib.resources import files

from rx_kernel import YamlCutReader
# -----------------------------
def test_default():
    '''
    Tests default contructor
    '''
    rdr = YamlCutReader()
    rdr.PrintData()
# -----------------------------
def test_reading():
    '''
    Tests reading YAML file
    '''
    config_path = files('rx_tests').joinpath('kernel/yaml_cutreader.yaml')
    config_path = str(config_path)

    rdr = YamlCutReader(config_path)
    rdr.PrintData()
# -----------------------------
