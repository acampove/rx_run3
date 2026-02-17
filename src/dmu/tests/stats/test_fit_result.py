'''
File containing tests for FitResult class
'''

import pytest

from pathlib     import Path
from dmu.stats   import FitResult
from dmu.stats   import zfit
from dmu.testing import get_nll

# -----------------------------
def test_from_zfit():
    '''
    Test conversion from zfit object
    '''
    nll = get_nll(kind = 's+b')
    min = zfit.minimize.Minuit()

    res = min.minimize(loss = nll)
    res.hesse(name = 'minuit_hesse')

    val = FitResult.from_zfit(res = res)
    print(val)
# -----------------------------
def test_serialize(tmp_path : Path):
    '''
    Test conversion from zfit object
    '''
    nll = get_nll(kind = 's+b')
    min = zfit.minimize.Minuit()

    res = min.minimize(loss = nll)
    res.hesse(name = 'minuit_hesse')

    val = FitResult.from_zfit(res = res)

    fpath = tmp_path / 'data.json'
    val.to_json(path = fpath)

    obj = FitResult.from_json(path = fpath)

    assert obj == val
# -----------------------------
def test_getitem():
    '''
    Test conversion from zfit object
    '''
    nll = get_nll(kind = 's+b')
    min = zfit.minimize.Minuit()

    res = min.minimize(loss = nll)
    res.hesse(name = 'minuit_hesse')

    val = FitResult.from_zfit(res = res)

    x, y = val['mu']

    print(val)
    print(x,y)
# -----------------------------
def test_get():
    '''
    Test conversion from zfit object
    '''
    nll = get_nll(kind = 's+b')
    min = zfit.minimize.Minuit()

    res = min.minimize(loss = nll)
    res.hesse(name = 'minuit_hesse')

    val = FitResult.from_zfit(res = res)

    x, y = val['mu']
    a, b = val.get('mu', 3)

    assert a == x
    assert b == y

    a, b = val.get('none', 3)

    assert a == 3
    assert b == 3
# -----------------------------
def test_no_errors_ok():
    '''
    Test no_errors_ok
    '''
    nll = get_nll(kind = 's+b')
    min = zfit.minimize.Minuit()

    res = min.minimize(loss = nll)
    FitResult.from_zfit(res = res, no_errors_ok=True)

    with pytest.raises(KeyError):
        FitResult.from_zfit(res = res)
# -----------------------------
def test_hash():
    '''
    Test conversion from zfit object
    '''
    nll = get_nll(kind = 's+b')
    min = zfit.minimize.Minuit()

    res = min.minimize(loss = nll)
    res.hesse(name = 'minuit_hesse')

    val = FitResult.from_zfit(res = res)
    hash(val)
