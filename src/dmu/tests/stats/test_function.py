'''
Module with tests for Function class
'''
import numpy
import pytest
import matplotlib.pyplot as plt

from pathlib               import Path
from dmu.stats.function    import FunOutOfBounds, Function
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
            numpy.linspace(0, 9, num=200)]
#----------------------------------------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This runs before tests
    '''
    LogStore.set_level('dmu:stats:function', 10)
#----------------------------------------------------
def test_simple():
    '''
    Will test creation of function
    '''

    x = [0, 1, 2, 3]
    y = [0, 1, 2, 3]

    fun = Function(x=x, y=y)
    print(fun)
#----------------------------------------------------
def test_tag(tmp_path : Path):
    '''
    Will test adding tag to function
    '''
    x = [0, 1, 2, 3]
    y = [0, 1, 2, 3]

    tag       = 'this_is_a_tag'
    path      = tmp_path / 'function.json'

    fun_1     = Function(x=x, y=y)
    fun_1.tag = tag
    fun_1.save(path = path)

    fun_2 = Function.load(path)

    assert tag == fun_1.tag
    assert tag == fun_2.tag
#----------------------------------------------------
def test_save_plot(tmp_path : Path):
    '''
    Test saving plot feature
    '''
    x = [0, 1, 2, 3]
    y = [0, 1, 2, 3]

    fun=Function(x=x, y=y)
    fun.save(path = tmp_path / 'function.json', plot = True)
#----------------------------------------------------
def test_repeated():
    '''
    Will test creation of function
    '''

    x = [0, 1, 2, 2, 3]
    y = [0, 1, 2, 2, 3]

    fun = Function(x=x, y=y)
    print(fun)
#----------------------------------------------------
def test_equal():
    '''
    Will test == overiding
    '''
    x = [0, 1, 2, 3]
    y = [0, 1, 2, 3]

    fun_1 = Function(x=x, y=y)

    w = [0, 1, 2, 3]
    z = [0, 1, 2, 3]

    fun_2 = Function(x=w, y=z)

    fun_3 = 23

    assert fun_1 == fun_2
    assert fun_1 != fun_3
#----------------------------------------------------
def test_save(tmp_path : Path):
    '''
    Test saving feature
    '''
    x = [0, 1, 2, 3]
    y = [0, 1, 2, 3]

    fun=Function(x=x, y=y)
    fun.save(path = tmp_path / 'function_1.json')
    fun.save(path = tmp_path / 'function_2.json')
#----------------------------------------------------
def test_load(tmp_path : Path):
    '''
    Test loading function
    '''
    x = [0, 1, 2, 3]
    y = [0, 1, 2, 3]

    path = tmp_path / 'function.json'

    fun_1=Function(x=x, y=y)
    fun_1.save(path = path)

    fun_2=Function.load(path)

    assert fun_1 == fun_2
#----------------------------------------------------
@pytest.mark.parametrize('xval', Data.l_arg_eval)
def test_eval(xval : float, tmp_path : Path):
    '''
    Will test () operator
    '''

    x = numpy.linspace(0, 9, num=10)
    y = numpy.sin(x)

    fun  = Function(x=x, y=y)
    yval = fun(xval)

    if yval.size < 20:
        return

    plt.scatter(xval, yval, label='Function')
    plt.scatter(   x,    y, label=    'Data')
    plt.legend()
    plt.savefig(tmp_path / 'function.png')
    plt.close()
#----------------------------------------------------
@pytest.mark.parametrize('xval', Data.l_arg_eval)
def test_load_eval(xval : float, tmp_path : Path):
    '''
    Will test () operator on loaded function
    '''
    x    = numpy.linspace(0, 9, num=10)
    y    = numpy.sin(x)

    path = tmp_path / 'function.json'

    fun  = Function(x=x, y=y)
    fun.save(path = path)

    fun  = Function.load(path)
    yval = fun(xval)
    if yval.size < 20:
        return

    plt.scatter(xval, yval, label='Function')
    plt.scatter(   x,    y, label=    'Data')
    plt.legend()
    plt.savefig(tmp_path / 'function.png')
    plt.close()
#----------------------------------------------------
def _shuffle(x, y):
    '''
    Will randomly shuffle (x, y) points and return shuffled x and y vectors
    '''
    x_y  = numpy.array([x , y]).T
    numpy.random.shuffle(x_y)
    x_y  = x_y.T
    x    = x_y[0]
    y    = x_y[1]

    return x, y
#----------------------------------------------------
@pytest.mark.parametrize('xval', Data.l_arg_eval)
def test_unsorted(xval : float, tmp_path : Path):
    '''
    Will test () operator on loaded function
    '''
    x    = numpy.linspace(0, 9, num=10)
    y    = numpy.sin(x)
    x, y = _shuffle(x, y)

    path = tmp_path / 'function.json'

    fun  = Function(x=x, y=y)
    fun.save(path = path)

    fun  = Function.load(path)
    yval = fun(xval)
    if yval.size < 20:
        return

    plt.scatter(xval, yval, label='Function')
    plt.scatter(   x,    y, label=    'Data')
    plt.legend()
    plt.savefig(tmp_path / 'function.png')
    plt.close()
#----------------------------------------------------
def test_large_load(tmp_path : Path):
    '''
    Test loading function
    '''
    x = numpy.linspace(0, 1, 100_000)
    y = numpy.linspace(0, 1, 100_000)

    path = tmp_path / 'function.json'

    fun_1=Function(x=x, y=y)
    fun_1.save(path = path)

    fun_2=Function.load(path)

    assert fun_1 == fun_2
#----------------------------------------------------
def test_eval_off():
    '''
    Will test () operator with data outside bounds
    '''
    x   = numpy.linspace(0, 9, num=10)
    y   = numpy.sin(x)
    fun = Function(x=x, y=y)

    z = numpy.linspace(-1, 10, num=100)
    with pytest.raises(FunOutOfBounds):
        _ = fun(z, off_bounds_raise=True)

    _ = fun(z, off_bounds_raise=False)
#----------------------------------------------------
