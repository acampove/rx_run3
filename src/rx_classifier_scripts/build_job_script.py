'''
Script used to build jobs.sh, which is used to send application jobs
'''
import argparse
import yaml

# --------------------------------
class Data:
    '''
    Class used to share attributes
    '''
    path : str
    conf : str
# --------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to create list of commands to be run to apply MVA scores')
    parser.add_argument('-p','--path', type=str, help='Path to YAML file with list of samples', required=True)
    parser.add_argument('-c','--conf', type=str, help='Path to YAML file with configuration for application of MVA', required=True)
    args = parser.parse_args()

    Data.path = args.path
    Data.conf = args.conf
# --------------------------------
def _get_sample_trigger() -> list[tuple[str,str]]:
    with open(Data.path, encoding='utf-8') as ifile:
        d_data = yaml.safe_load(ifile)

    l_samp_trig = []
    for sample in d_data:
        for trigger in d_data[sample]:
            l_samp_trig.append((sample, trigger))

    return l_samp_trig
# --------------------------------
def _get_command(sample : str, trigger : str) -> list[str]:
    if not sample.startswith('DATA_'):
        return [f'apply_classifier -c {Data.conf} -s {sample} -t {trigger}']

    return [
            f'apply_classifier -c {Data.conf} -s {sample} -t {trigger} -p 0 4',
            f'apply_classifier -c {Data.conf} -s {sample} -t {trigger} -p 1 4',
            f'apply_classifier -c {Data.conf} -s {sample} -t {trigger} -p 2 4',
            f'apply_classifier -c {Data.conf} -s {sample} -t {trigger} -p 3 4']
# --------------------------------
def _save_commands(l_samp_trig : list[tuple[str,str]]) -> None:
    l_command = []
    for samp, trig in l_samp_trig:
        l_command += _get_command(samp, trig)

    text = '\n'.join(l_command)
    with open('commands.sh', 'w', encoding='utf-8') as ofile:
        ofile.write(text)
# --------------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()

    l_samp_trig = _get_sample_trigger()

    _save_commands(l_samp_trig)
# --------------------------------
if __name__ == '__main__':
    main()
