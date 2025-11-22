'''
Script that will be used to update the mapping between event type and
formatted sample names
'''
from importlib.resources import files

import yaml

from ap_utilities.decays import utilities as aput

# --------------------------------
def _add_data_low_org(d_low_org : dict[str,str]) -> dict[str,str]:
    d_low_org['data_24_magdown_24c1'] = 'DATA_24_MagDown_24c1'
    d_low_org['data_24_magdown_24c2'] = 'DATA_24_MagDown_24c2'
    d_low_org['data_24_magdown_24c3'] = 'DATA_24_MagDown_24c3'
    d_low_org['data_24_magdown_24c4'] = 'DATA_24_MagDown_24c4'
    d_low_org['data_24_magup_24c1'  ] = 'DATA_24_MagUp_24c1'
    d_low_org['data_24_magup_24c2'  ] = 'DATA_24_MagUp_24c2'
    d_low_org['data_24_magup_24c3'  ] = 'DATA_24_MagUp_24c3'
    d_low_org['data_24_magup_24c4'  ] = 'DATA_24_MagUp_24c4'

    return d_low_org
# --------------------------------
def _load_file(file_name : str) -> dict[str,str]:
    file_path = files('ap_utilities_data').joinpath(f'naming/{file_name}')
    file_path = str(file_path)

    with open(file_path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# --------------------------------
def _reformat(d_data : dict[str,str], keys : bool) -> dict[str,str]:
    if keys:
        d_data_form = { aput.format_nickname(key) : value for key, value in d_data.items() }
    else:
        d_data_form = { key : aput.format_nickname(value) for key, value in d_data.items() }

    return d_data_form
# --------------------------------
def _save(d_data : dict[str,str], file_name : str) -> None:
    file_path = files('ap_utilities_data').joinpath(file_name)
    file_path = str(file_path)

    with open(file_path, 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_data, ofile)
# --------------------------------
def main():
    '''
    Starts here
    '''

    d_name_evt = _load_file('name_evt.yaml')
    d_evt_name = _load_file('evt_name.yaml')

    d_form_evt = _reformat(d_name_evt, keys=True)
    d_evt_form = _reformat(d_evt_name, keys=False)
    d_low_org  = { sample.lower() : sample for sample in d_form_evt }
    d_low_org  = _add_data_low_org(d_low_org)

    _save(d_low_org , 'lower_original.yaml')
    _save(d_form_evt,       'form_evt.yaml')
    _save(d_evt_form,       'evt_form.yaml')
# --------------------------------
if __name__ == '__main__':
    main()
