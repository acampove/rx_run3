'''
Module with tests for Function class
'''
import os

from dmu.stats.function import Function

#----------------------------------------------------
def _make_out_dir(name : str):
    '''
    Will take test name and make output directory

    returns path to directory
    '''
    path = f'tests/stats/{name}'
    os.makedirs(path, exist_ok=True)

    return path
#----------------------------------------------------
def test_simple():
    '''
    Will test creation of function
    '''

    x = [0, 1, 2]
    y = [0, 1, 2]

    fun = Function(x=x, y=y)
    print(fun)
#----------------------------------------------------
def test_equal():
    '''
    Will test == overiding
    '''
    x = [0, 1, 2]
    y = [0, 1, 2]
    fun_1 = Function(x=x, y=y)

    w = [0, 1, 2]
    z = [0, 1, 2]
    fun_2 = Function(x=w, y=z)

    fun_3 = 23

    assert fun_1 == fun_2
    assert fun_1 != fun_3
#----------------------------------------------------
def test_save():
    '''
    Test saving feature
    '''
    out_dir_path = _make_out_dir('save')
    x = [0, 1, 2]
    y = [0, 1, 2]

    fun=Function(x=x, y=y)
    fun.save(path = f'{out_dir_path}/function.json')
#----------------------------------------------------
def test_load():
    '''
    Test loading function
    '''
    out_dir_path = _make_out_dir('load')
    x = [0, 1, 2]
    y = [0, 1, 2]

    path = f'{out_dir_path}/function.json'

    fun_1=Function(x=x, y=y)
    fun_1.save(path = path)

    fun_2=Function.load(path)

    assert fun_1 == fun_2
#----------------------------------------------------
