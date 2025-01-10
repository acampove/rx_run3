'''
Script used to interface with PIDCalib2 to get
the PID maps needed by RX analyses
'''
import argparse

# ----------------------------
class Data:
    '''
    Class used to share attributes
    '''

    cfg_path : str
# ----------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-c', '--conf' , type=str, help='Name of config file, e.g. config.yaml')
    args = parser.parse_args()

    Data.cfg_path = args.conf
# ----------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    _run()
# ----------------------------
if __name__ == '__main__':
    main()
