'''
Script used to print statistics on files stored in cluster
'''
import os
import argparse
import pandas   as pnd

import yaml
# --------------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    samples_path : str
# --------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to print statistics on files store in cluster')
    parser.add_argument('-p', '--path' , type=str, help='Path to file storing lists of samples', required=True)
    args = parser.parse_args()

    Data.samples_path = args.path
# --------------------------------------
def _get_samples() -> dict:
    with open(Data.samples_path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    return d_data
# --------------------------------------
def _size_from_paths(l_path : str) -> int:
    l_size = [ os.path.getsize(path) / (1024 ** 2) for path in l_path ]
    size   = sum(l_size)

    return int(size)
# --------------------------------------
def _data_to_size(d_data : dict) -> pnd.DataFrame:
    d_size = {'Sample' : [], 'Trigger' : [], 'Size' : []}
    for sample in d_data:
        for trigger in d_data[sample]:
            l_path = d_data[sample][trigger]
            size   = _size_from_paths(l_path)

            d_size['Sample' ].append(sample)
            d_size['Trigger'].append(trigger)
            d_size['Size'   ].append(size)

    df = pnd.DataFrame(d_size)
    df = df.sort_values(by='Size', ascending=False)

    return df
# --------------------------------------
def _add_total(df : pnd.DataFrame) -> pnd.DataFrame:
    total_size      = df.Size.sum()
    df.loc[len(df)] = ['Total', 'Any', total_size]

    return df
# --------------------------------------
def _save_table(df : pnd.DataFrame, file_name : str) -> None:
    df = _add_total(df)
    with open(file_name, 'w', encoding='utf-8') as ofile:
        text = df.to_markdown(index=False)
        ofile.write(text)
# --------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
    d_data = _get_samples()
    df     = _data_to_size(d_data)
    df_dt  = df[ df.Sample.str.startswith('DATA_')]
    df_mc  = df[~df.Sample.str.startswith('DATA_')]

    _save_table(df_dt, 'data.md')
    _save_table(df_mc,   'mc.md')
# --------------------------------------
if __name__ == '__main__':
    main()
