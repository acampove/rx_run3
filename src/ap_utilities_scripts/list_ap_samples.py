'''
This script is meant to list information from AP productions
'''

import argparse

# ----------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Script needed to parse samples from AP productions')
    parser.add_argument('-v', '--version' , type=str, help='Version of production', required=True)
    args = parser.parse_args()

# ----------------------
def main():
    '''
    Entry point
    '''
    cfg = _parse_args()

    _list_samples(cfg=cfg)
# ----------------------
if __name__ == '__main__':
    main()
