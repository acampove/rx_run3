'''
This script is used to make the text file with the list of jobs needed
to create the friend trees with the MVA scores
'''

import os
import argparse
import yaml

# --------------------------------
class Data:
    '''
    Data class
    '''
    main_yaml : str
    version   : str
    out_name  = 'mva_jobs.txt'
# --------------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script used to create list of MVA application jobs')
    parser.add_argument('-v', '--version' , type=str, help='Version of MVA files, e.g. 7p6', required=True)
    args = parser.parse_args()

    Data.version = args.version
# --------------------------------
def _save_file() -> None:
    with open(Data.main_yaml, encoding='utf-8') as ifile:
        data = yaml.safe_load(ifile)

    l_line = []
    for sample in data:
        for trigger in data[sample]:
            line = f'apply_classifier -v {Data.version} -s {sample:50} -t {trigger}'
            l_line.append(line)

    with open(Data.out_name, 'w', encoding='utf-8') as ofile:
        text = '\n'.join(l_line)
        ofile.write(text)
# --------------------------------
def _initialize() -> None:
    ana_dir = os.environ['ANADIR']
    yaml_path = f'{ana_dir}/Data/samples/main.yaml'

    if not os.path.isfile(yaml_path):
        raise FileNotFoundError(f'Cannot find {yaml_path}')

    Data.main_yaml = yaml_path
# --------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _initialize()

    _save_file()
# --------------------------------
if __name__ == '__main__':
    main()
