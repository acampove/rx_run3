'''
Module with test for MatrixPlotter class
'''

import numpy
import matplotlib.pyplot as plt

from dmu.plotting.matrix   import MatrixPlotter
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:plotting:test_matrix')
# -------------------------------------------
def test_simple():
    '''
    Basic matrix plotting test
    '''
    LogStore.set_level('dmu:plotting:matrix', 10)

    cfg = {
        'labels'     : ['x', 'y', 'z'],
        'title'      : 'Some title',
        'label_angle': 45,
        'upper'      : True,
        'zrange'     : [0, 10],
        'format'     : '{:.3f}',
        'size'       : [7, 7],
        'fontsize'   : 12,
        'mask_value' : 0}

    mat = [
        [1, 2, 3],
        [2, 0, 4],
        [3, 4, numpy.nan]
        ]

    mat = numpy.array(mat)

    obj = MatrixPlotter(mat=mat, cfg=cfg)
    obj.plot()
# -------------------------------------------
def test_correlations():
    '''
    Basic matrix plotting test
    '''
    LogStore.set_level('dmu:plotting:matrix', 10)

    cfg = {
        'labels'     : ['x', 'y', 'z'],
        'title'      : 'Some title',
        'label_angle': 45,
        'upper'      : True,
        'zrange'     : [0, 1],
        'format'     : '{:.3f}',
        'size'       : [7, 7],
        'fontsize'   : 12,
        'mask_value' : 0}

    nentries = 1000
    data     = [
        numpy.random.normal(loc=0, scale=1, size=nentries),
        numpy.random.normal(loc=0, scale=1, size=nentries),
        numpy.random.normal(loc=0, scale=1, size=nentries),
    ]

    mat = numpy.corrcoef(data, rowvar=True)
    obj = MatrixPlotter(mat=mat, cfg=cfg)
    obj.plot()
    plt.show()
