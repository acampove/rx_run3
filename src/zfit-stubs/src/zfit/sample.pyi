import tensorflow as tf
from collections.abc import Iterable
from zfit._interfaces import ZfitPDF
from zfit.util import ztyping
from zfit.z.random import counts_multinomial as counts_multinomial, sample_with_replacement as sample_with_replacement

__all__ = ['counts_multinomial', 'poisson', 'sample_with_replacement']

def poisson(n: ztyping.NumericalScalarType | None = None, pdfs: Iterable[ZfitPDF] | None = None) -> tf.Tensor: ...
