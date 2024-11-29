'''
Script used to build decay fields from YAML file storing event type -> decay correspondence 
'''
import argparse
import pprint

# ---------------------------
class Data:
    '''
    Class storing shared data
    '''

    l_event_type : list[str]
# ---------------------------
def _parse_args() -> None:
    parser = argparse.ArgumentParser(description='Used to perform several operations on TCKs')
    parser.add_argument('-i', '--input' , type=str, help='Path to textfile with event types')
    args = parser.parse_args()

    input_path = args.input
    with open(input_path, encoding='utf-8') as ifile:
        Data.l_event_type = ifile.read().splitlines()
# ---------------------------
def _get_decays(event_type : str) -> dict[str,str]:
    return {}
# ---------------------------
def main():
    '''
    Script starts here
    '''
    _parse_args()
    for event_type in Data.l_event_type:
        d_dec = _get_decays(event_type)
        pprint.pprint(d_dec)
        return
# ---------------------------
if __name__ == '__main__':
    main()
