import matplotlib.pyplot as plt
from . import ztyping as ztyping
from ..core.space import convert_to_space as convert_to_space
from .checks import RuntimeDependency as RuntimeDependency
from .warnings import warn_experimental_feature as warn_experimental_feature
from _typeshed import Incomplete
from collections.abc import Callable as Callable, Mapping
from zfit._interfaces import ZfitBinnedData as ZfitBinnedData, ZfitData as ZfitData, ZfitPDF as ZfitPDF, ZfitUnbinnedData as ZfitUnbinnedData

def plot_sumpdf_components_pdfV1(model, *, plotfunc: Callable | None = None, scale: int = 1, ax=None, linestyle=None, plotkwargs: Mapping[str, object] | None = None, extended: bool | None = None): ...
def plot_model_pdf(model: ZfitPDF, *, plotfunc: Callable | None = None, extended: bool | None = None, obs: ztyping.ObsTypeInput = None, scale: float | int | None = None, ax: plt.Axes | None = None, num: int | None = None, full: bool | None = None, linestyle=None, plotkwargs=None): ...
def assert_initialized(func): ...

class ZfitPDFPlotter:
    @warn_experimental_feature
    @assert_initialized
    def plotpdf(self, data: ZfitData | None = None, *, depth: int | None = None, density: bool | None = None, plotfunc: Callable | None = None, extended: bool | None = None, obs: ztyping.ObsTypeInput = None, scale: float | int | None = None, ax: plt.Axes | None = None, num: int | None = None, full: bool | None = None, linestyle=None, plotkwargs: Mapping[str, object] | None = None, histplotkwargs: Mapping[str, object] | None = None): ...
    @property
    def comp(self) -> None: ...
    def __call__(self, data=None, **kwargs): ...

class PDFPlotter(ZfitPDFPlotter):
    defaults: Incomplete
    pdf: Incomplete
    def __init__(self, pdf: ZfitPDF | None, pdfplotter: Callable | None = None, componentplotter: ZfitPDFPlotter = None, defaults: Mapping[str, object] | None = None) -> None: ...
    @property
    @assert_initialized
    def comp(self): ...

class SumCompPlotter(ZfitPDFPlotter):
    pdf: Incomplete
    def __init__(self, pdf: ZfitPDF | None, *args, **kwargs) -> None: ...
