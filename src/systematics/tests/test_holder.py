'''
Module containing tests for Holder class
'''

import numpy
import pytest

from rpk_tools     import Component
from rpk_tools     import Holder
from rpk_log_store import log_store as LogStore

# ----------------------
@pytest.fixture(scope='session', autouse=True)
def initialize():
    '''
    This will run before any test
    '''
    LogStore.set_level('rx_generic:holder', 10)
# ----------------------
def test_simple():
    '''
    Simplest test
    '''
    a = numpy.random.normal(size=10)
    b = numpy.random.normal(size=20)
    c = numpy.random.normal(size=30)    

    holder   = Holder[numpy.ndarray]()
    holder[Component.jpsi_ee] = a
    holder[Component.pKKK   ] = b 
    holder[Component.pKKpi  ] = c 

    assert Component.jpsi_ee in holder
    assert Component.pKKK    in holder
    assert Component.pKKpi   in holder

    assert numpy.equal(a, holder[Component.jpsi_ee]).all()
    assert numpy.equal(b, holder[Component.pKKK   ]).all()
    assert numpy.equal(c, holder[Component.pKKpi  ]).all()
# ----------------------
def test_copy():
    '''
    Tests copying holder
    '''
    a = numpy.random.normal(size=10)
    b = numpy.random.normal(size=20)
    c = numpy.random.normal(size=30)    

    holder   = Holder[numpy.ndarray]()
    holder[Component.jpsi_ee] = a
    holder[Component.pKKK   ] = b 
    holder[Component.pKKpi  ] = c 

    new_holder = holder()

    assert numpy.equal(new_holder[Component.jpsi_ee], holder[Component.jpsi_ee]).all()
    assert numpy.equal(new_holder[Component.pKKK   ], holder[Component.pKKK   ]).all()
    assert numpy.equal(new_holder[Component.pKKpi  ], holder[Component.pKKpi  ]).all()
# ----------------------
def test_iterable():
    '''
    Test that holder is iterable
    '''
    a = numpy.random.normal(size=10)
    b = numpy.random.normal(size=20)
    c = numpy.random.normal(size=30)    

    holder   = Holder[numpy.ndarray]()
    holder[Component.jpsi_ee] = a
    holder[Component.pKKK   ] = b 
    holder[Component.pKKpi  ] = c 

    for comp in holder:
        print(comp)

    comps = list(holder)

    assert comps == [Component.jpsi_ee, Component.pKKK, Component.pKKpi]
# ----------------------
def test_items():
    '''
    Test retrieval of items 
    '''
    a = numpy.random.normal(size=10)
    b = numpy.random.normal(size=20)
    c = numpy.random.normal(size=30)    

    holder   = Holder[numpy.ndarray]()
    holder[Component.jpsi_ee] = a
    holder[Component.pKKK   ] = b 
    holder[Component.pKKpi  ] = c 

    comps = []
    arrays= []
    for comp, array in holder.items():
        comps.append(comp)
        arrays.append(array)

    assert comps  == [Component.jpsi_ee, Component.pKKK, Component.pKKpi]
    assert arrays == [a, b, c]
# ----------------------
