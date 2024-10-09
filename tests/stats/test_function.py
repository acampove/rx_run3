'''
Module with tests for Function class
'''
import os
import numpy
import pytest
import matplotlib.pyplot as plt

from dmu.stats.function    import Function
from dmu.logging.log_store import LogStore

log_store = LogStore.add_logger('dmu:tests:test_function')
#----------------------------------------------------
class Data:
    '''
    Class used to hold shared data
    '''
    l_arg_eval = [
            0.2,
            [0.1, 0.2, 0.3],
            numpy.linspace(0, 5, num=200)]
#----------------------------------------------------
@pytest.fixture
def _initialize():
    LogStore.set_level('dmu:stats:function', 10)
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
@pytest.mark.parametrize('xval', Data.l_arg_eval)
def test_eval(xval):
    '''
    Will test () operator
    '''

    x = numpy.linspace(0, 5, num=10)
    y = numpy.sin(x)

    fun  = Function(x=x, y=y)
    yval = fun(xval)

    if yval.size < 20:
        return

    out_dir_path = _make_out_dir('eval')
    plt.scatter(xval, yval, label='Function')
    plt.scatter(   x,    y, label=    'Data')
    plt.legend()
    plt.savefig(f'{out_dir_path}/function.png')
    plt.close()
#----------------------------------------------------
