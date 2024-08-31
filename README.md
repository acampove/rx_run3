# D(ata) M(anipulation) U(tilities)

These are tools that can be used for different data analysis tasks.

# Text manipulation

## Transformations

Run:

```bash
transform_text -i ./transform.txt -c ./transform.toml
```
to apply a transformation to `transform.txt` following the transformations in `transform.toml`.

The tool can be imported from another file like:

```python
from dmu.text.transformer import transformer as txt_trf

trf=txt_trf(txt_path=data.txt, cfg_path=data.cfg)
trf.save_as(out_path=data.out)
```

Currently the supported transformations are:

### append

Which will apppend to a given line a set of lines, the config lines could look like:

```toml
[append]
'primes are'=['2', '3', '5']
'days are'=['Monday', 'Tuesday', 'Wednesday']
```


