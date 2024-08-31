#!/usr/bin/env python3

from dmu.text import transformer as txt_trf

#---------------------------------
class data:
    txt = None
    cfg = None
    out = None
#---------------------------------
def get_args():
    parser=argparse.ArgumentParser(description='Will transform a text file following a set of rules')
    parser.add_argument('-i', '--input' , help='Path to input file' , required=True) 
    parser.add_argument('-o', '--output', help='Path to output file, if not passed, it will be same as input, but with trf before extension')
    parser.add_argument('-c', '--config', help='Path to config file', required=True) 
    args = parser.parse_args()

    data.txt = args.input
    data.out = args.output
    data.cft = args.config
#---------------------------------
def main():
    get_args()

    #trf = txt_trf(txt_path=data.txt, cfg_path=data.cfg)
    #trf.save_as(data.out)
#---------------------------------
if __main__ == '__main__':
    main()
