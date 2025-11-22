from .base_tf import WrapOptimizer as WrapOptimizer

class Adam(WrapOptimizer):
    def __init__(self, tol=None, learning_rate: float = 0.2, beta1: float = 0.9, beta2: float = 0.999, epsilon: float = 1e-08, name: str = 'Adam', **kwargs) -> None: ...
