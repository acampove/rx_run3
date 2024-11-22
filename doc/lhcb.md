# DecayFiles

## Decay nicknames

### Table with nicknames and event types

The following lines:

```bash
export DECPATH=/home/acampove/Packages/DecFiles

update_decinfo
```

will:

1. Set the path to the [DecFiles](https://gitlab.cern.ch/lhcb-datapkg/Gen/DecFiles)
root directory such that `update_decinfo` can use it.
1. Read the event types and nicknames and save them to a YAML file

### Accessing table

These nicknames can be accessed from python scripts with:

```python
import dmu.physics.utilities as phut

# To get exactly what was saved
literal = phut.read_decay_name(event_type=event_type, style='literal')

# To get representation with special symbols like "," or "-" replaced
safe_1  = phut.read_decay_name(event_type=event_type, style= 'safe_1')
```
