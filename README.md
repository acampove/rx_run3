# ap_utilities

This project holds code needed to transform the AP used by the RD group into something that makes ntuples with MVA HLT triggers.

## Add MVA lines to `Config.py`

To do that run:

```bash
transform_text -i Config.py -o output.py hlt_rename.toml -l 10
```

which will create an `output.py` file with the replacements specified in `hlt_rename.toml`.

