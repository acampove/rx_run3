'''
Script used to compare performance of classifiers
'''
import argparse

# ------------------------------
class Data:
    '''
    Data class
    '''
    cfg_path : str
# ------------------------------
def _parse_args():
    parser = argparse.ArgumentParser(description='Used to perform comparisons of classifier performances')
    parser.add_argument('-c', '--conf' , help='Path to configuration path', required=True)
    args = parser.parse_args()

    Data.cfg_path = args.conf
# ------------------------------
def _compare():
    pass
# ------------------------------
def main():
    '''
    Start here
    '''
    _parse_args()
    _compare()
# ------------------------------
if __name__ == '__main__':
    main()
