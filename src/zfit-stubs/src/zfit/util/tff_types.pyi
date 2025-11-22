from typing import TypeVar

__all__ = ['BoolTensor', 'ComplexTensor', 'IntTensor', 'RealTensor', 'StringTensor']

BoolTensor = TypeVar('BoolTensor', *tensor_like)
IntTensor = TypeVar('IntTensor', *tensor_like)
RealTensor = TypeVar('RealTensor', *tensor_like)
FloatTensor = TypeVar('FloatTensor', *tensor_like)
DoubleTensor = TypeVar('DoubleTensor', *tensor_like)
ComplexTensor = TypeVar('ComplexTensor', *tensor_like)
StringTensor = TypeVar('StringTensor', *tensor_like)
