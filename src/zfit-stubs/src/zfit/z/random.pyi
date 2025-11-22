import tensorflow as tf
from collections.abc import Iterable

__all__ = ['counts_multinomial', 'sample_with_replacement']

def sample_with_replacement(a: tf.Tensor, axis: int, sample_shape: tuple[int]) -> tf.Tensor: ...
def counts_multinomial(total_count: int | tf.Tensor, probs: Iterable[float | tf.Tensor] | None = None, logits: Iterable[float | tf.Tensor] | None = None, dtype: tf.DType = ...) -> tf.Tensor: ...
