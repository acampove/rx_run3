import argparse
import utils_noroot as utnr

#-----------------------------
class data:
    log  = utnr.getLogger(__name__)
    ijob = None

    l_trig = ['ETOS', 'GTIS']
    l_year = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_brem = ['0', '1', '2']
#-----------------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to print settings used to run q2 systematics jobs')
    parser.add_argument('-i', '--index', type=int, help='Job index', required=True)
    args = parser.parse_args()

    data.ijob = args.index
#-----------------------------
def main():
    index = 0
    njob  = len(data.l_trig) * len(data.l_year) * len(data.l_brem)
    for trig in data.l_trig:
        for year in data.l_year:
            for brem in data.l_brem:
                if index != data.ijob:
                    index+= 1
                    continue

                data.log.info(f'TRIG={trig}')
                data.log.info(f'YEAR={year}')
                data.log.info(f'BREM={brem}')
                data.log.info(f'NJOB={njob}')

                return
#-----------------------------
if __name__ == '__main__':
    get_args()
    main()

