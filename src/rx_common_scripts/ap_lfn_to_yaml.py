'''
Module that will transform lists of LFNs from AP jobs 
into a yaml file ready to be plugged into the RX c++ framework
'''
# pylint: disable=line-too-long, import-error
# pylint: disable=invalid-name

from dmu.logging.log_store  import LogStore
log = LogStore.add_logger('rx_common:pap_lfn_to_yaml')

# ---------------------------------
def main():
    '''
    Script starts here
    '''
    args = _get_args()
    _initialize(args)

    d_data = _get_data_dict()

    with open('samples.yaml', 'w', encoding='utf-8') as ofile:
        yaml.safe_dump(d_data, ofile)
# ---------------------------------
if __name__ == '__main__':
    main()
