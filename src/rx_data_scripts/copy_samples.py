'''
Script used to copy ntuples from mounted filesystem
'''
import argparse

# -----------------------------------------
class Data:
    '''
    Class holding attributes meant to be shared
    '''
    path : str
    samp : str
# -----------------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Script used to copy files from remote server to laptop')
    parser.add_argument('-p', '--path', type=str, help='Path to YAML file with paths samples existing in source file system')
    parser.add_argument('-s', '--samp', type=str, help='Path to YAML files with samples to be copied')
    args = parser.parse_args()

    Data.path = args.path
    Data.samp = args.samp
# -----------------------------------------
def main():
    '''
    Starts here
    '''
    _parse_args()
# -----------------------------------------
if __name__ == '__main__':
    main()
