from .serializer import BaseRepr as BaseRepr, Serializer as Serializer
from typing import Literal

class BasePDFRepr(BaseRepr):
    hs3_type: Literal['BasePDF']
    extended: bool | None | Serializer.types.ParamTypeDiscriminated
    name: str | None
    def convert_params(cls, values): ...
