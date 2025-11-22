import tensorflow as tf
from _typeshed import Incomplete

Number: Incomplete
TensorLike: Incomplete
FloatTensorLike: Incomplete
EPSILON: float

def interpolate_spline(train_points: TensorLike, train_values: TensorLike, query_points: TensorLike, order: int, regularization_weight: FloatTensorLike = 0.0, name: str = 'interpolate_spline') -> tf.Tensor: ...
