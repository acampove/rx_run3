'''
Module with test for MatrixPlotter class
'''

import numpy
import matplotlib.pyplot as plt

from dmu.plotting.matrix import MatrixPlotter

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
            'mask_value' : 0,
            }

    cov = [
            [1, 2, 3],
            [2, 0, 4],
            [3, 4, numpy.nan]
            ]

    cov = numpy.array(cov)

    obj = MatrixPlotter(cov=cov, cfg=cfg)
    obj.plot()

    #'path'       : '/tmp/dmu/tests/plotting/matrix/simple.png',
    plt.show()
# -------------------------------------------
