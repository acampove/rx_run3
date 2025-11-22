'''
Script used to create a GUI to go through fits in function of
the MVA scores
'''

import os
import numpy
import argparse
import matplotlib.pyplot as plt
import matplotlib.image  as mpimg
from matplotlib.widgets    import Slider
from matplotlib.figure     import Figure
from matplotlib.image      import AxesImage
from matplotlib.axes._axes import Axes

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('rx_fitter:navigate_fits')
# ----------------------
class Data:
    '''
    Class meant to be used to share attributes
    '''
    q2bin    : str
    ax_x     : Axes
    ax_y     : Axes

    im       : AxesImage
    img      : numpy.ndarray
    fig      : Figure

    slider_x : Slider
    slider_y : Slider
# ------------------------------------------
def _get_filename(x : int, y : int) -> str:
    '''
    Parameters
    --------------
    x,y: Integer coordinates for plots, in percent

    Returns
    --------------
    Path to PNG file
    '''
    identifier = f'{x:03}_{y:03}'
    ana_dir    = os.environ['ANADIR']
    path       = f'{ana_dir}/fits/data/{identifier}/rare/electron/data/DATA_24_p/Hlt2RD_BuToKpEE_MVA_rx_{Data.q2bin}/fit.png'

    return path
# ------------------------------------------
def _update(_) -> None:
    '''
    Updates plot
    '''
    x     = int(Data.slider_x.val)
    y     = int(Data.slider_y.val)
    fname = _get_filename(x=x, y=y)

    if not os.path.isfile(fname):
        image = numpy.zeros_like(Data.img)
    else:
        image = mpimg.imread(fname)

    Data.im.set_data(image)
    Data.fig.canvas.draw_idle()
# ----------------------
def _make_figure() -> None:
    '''
    Makes figure where plots will go
    '''
    fig, ax = plt.subplots()
    plt.subplots_adjust(bottom=0.25)
    im      = ax.imshow(Data.img)
    ax.axis('off')

    Data.ax_x = plt.axes((0.15, 0.10, 0.7, 0.03))
    Data.ax_y = plt.axes((0.15, 0.05, 0.7, 0.03))
    Data.im   = im
    Data.fig  = fig
# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to create an interactive window to explore fits for different parameters')
    parser.add_argument('-q', '--q2bin' , type=str, help='q2 bin', choices=['low', 'central', 'high'], required=True)
    args = parser.parse_args()

    Data.q2bin = args.q2bin
# ----------------------
def main():
    '''
    Entry point
    '''
    _parse_args()

    x_vals = list(range(60, 100, 4))
    y_vals = list(range(60, 100, 4))
    init_x = x_vals[0]
    init_y = y_vals[0]

    Data.img      = mpimg.imread(_get_filename(init_x, init_y))

    _make_figure()

    Data.slider_x = Slider(
        Data.ax_x, 'CMB', min(x_vals), max(x_vals),
        valinit     = init_x,
        valstep     = 4,
        handle_style= {'size' : 20})

    Data.slider_y = Slider(
        Data.ax_y, 'PRC', min(x_vals), max(x_vals),
        valinit     = init_y,
        valstep     = 4,
        handle_style= {'size' : 20})

    Data.slider_x.on_changed(_update)
    Data.slider_y.on_changed(_update)
    plt.show()
# ----------------------
if __name__ == '__main__':
    main()
