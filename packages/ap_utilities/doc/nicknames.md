## Decay nicknames

### Accessing table with DecFiles sample nicknames

These nicknames can be accessed from python scripts with:

```python
import ap_utilities.decays.utilities as aput

# TO get a formatted nickname for the sample from the event type
nickname = aput.read_decay_name(event_type=event_type)

# To get event type back from nickname
event_type = aput.read_event_type(nickname=nickname)

# To get old nickname from new one. The old one is the one used by RX in Run1/2
old_nickname = aput.old_from_new_nick(nickname=nickname)

# To get new nickname from old one
new_nickname = aput.new_from_old_nick(nickname=nickname)

# To get original nickname from lowercase nickname
original_nickname = aput.name_from_lower_case(lower_case)
```

the reason for `name_from_lower_case` to exist is that the `AnalysisProductions` currently make the
names of the samples lower-case, which alters the samples' names when these are parsed from the job name.
The function allows to retrieve back the old naming.

### Update table with nicknames and event types

This is most likely not needed, unless a new sample has been created and a new nickname needs to be added. The following lines:

```bash
export DECPATH=/home/acampove/Packages/DecFiles

update_decinfo
```

will:

1. Set the path to the [DecFiles](https://gitlab.cern.ch/lhcb-datapkg/Gen/DecFiles)
root directory such that `update_decinfo` can use it.
1. Read the event types and nicknames and save them to a YAML file
1. Read the event types and decay strings and save them to a YAML file

### Update YAML files with formatted sample names

When the formatting rules change, the yaml files:

```bash
evt_form.yaml
form_evt.yaml
lower_original.yaml
```
will all have to be updated with:

```bash
update_sample_naming
```

