'''
This module implements tests for the KDEOptimizer class
'''

import numpy
import pytest

from dmu.stats.zfit          import zfit
from dmu.stats.kde_optimizer import KDEOptimizer

@pytest.mark.parametrize('size', [100, 1_000, 10_000])
def test_simple(size : int):
    '''
    Simplest test
    '''
    arr  = numpy.random.normal(loc=0.0, scale=1.0, size=size)
    obs  = zfit.Space('x', limits=(-3,+3))
    dat  = zfit.data.from_numpy(obs=obs, array=arr)

    opt  = KDEOptimizer(data=dat, obs=obs)
    pdf  = opt.get_pdf()
