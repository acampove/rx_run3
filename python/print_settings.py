import argparse
import utils_noroot as utnr

#-----------------------------
class data:
    log    = utnr.getLogger(__name__)
    ijob   = None

    l_trig = ['ETOS', 'GTIS']
    l_year = ['2011', '2012', '2015', '2016', '2017', '2018'] 
    l_dset = ['r1', 'r2p1', '2017', '2018']
    l_brem = ['0', '1', '2']
#-----------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to print settings used to run q2 systematics jobs')
    parser.add_argument('-i', '--index', type=int, help='Job index', required=True)
    parser.add_argument('-d', '--dset' ,           help='Use datasets (merged years) instead of years', action='store_true')
    args = parser.parse_args()

    data.ijob = args.index
    data.dset = args.dset
#-----------------------------
def main():
    index = 0

    l_sample = data.l_dset if data.dset else data.l_year

    njob  = len(data.l_trig) * len(l_sample) * len(data.l_brem)
    for trig in data.l_trig:
        for year in l_sample:
            for brem in data.l_brem:
                if index != data.ijob:
                    index+= 1
                    continue

                print(f'TRIG={trig}')
                print(f'YEAR={year}')
                print(f'BREM={brem}')
                print(f'NJOB={njob}')

                return
#-----------------------------
if __name__ == '__main__':
    get_args()
    main()

