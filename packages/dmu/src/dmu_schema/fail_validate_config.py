from dataclasses import dataclass
from typing import List

@dataclass
class ConfigSchema:
    wrong_key : List[str]
