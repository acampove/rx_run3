'''
This file contains tests for MisIDFitter
'''
import os
import pytest
import pandas            as pnd
import matplotlib.pyplot as plt

from dmu.stats.zfit            import zfit
from dmu.generic               import utilities  as gut
from dmu.stats.zfit_plotter    import ZFitPlotter
from zfit.core.interfaces      import ZfitData   as zdata
from zfit.core.interfaces      import ZfitPDF    as zpdf
from rx_misid.misid_fitter     import MisIDFitter
from rx_misid.misid_calculator import MisIDCalculator

# ---------------------------------------------------
class Data:
    '''
    Used to store attributes
    '''
    obs = zfit.Space('mass', limits=(4500, 7000))
# ---------------------------------------------------
def _get_toy_data() -> zdata:
    pdf = sut.get_model(kind='s+b')
    sam = pdf.create_sampler(n=1000)

    return sam
# ---------------------------------------------------
def test_simple():
    '''
    Simplest test
    '''
    q2bin = 'low'
    data  = _get_toy_data()

    ftr   = MisIDFitter(data=data, q2bin=q2bin)
    pdf   = ftr.get_pdf()
