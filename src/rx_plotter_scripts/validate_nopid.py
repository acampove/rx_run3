'''
Script used to plot PID distributions from:

    - Samples with PID removed and original ones
    - Check that the distributions agree when the former ones have had the PID added
'''

import argparse

# ----------------------------
class Data:
    '''
    Data class
    '''
    channel : str
# ----------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-c', '--channel', type=str, help='Channel where the validation will be ran', choices=['ee', 'mm'], required=True)
    args = parser.parse_args()

    Data.channel = args.channel
# ----------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
# ----------------------------
if __name__ == '__main__':
    main()
