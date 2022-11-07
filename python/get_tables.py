import utils_noroot as utnr
import os
import argparse

#-------------------
class data:
    log     = utnr.getLogger(__name__)
    cal_dir = os.environ['CALDIR']

    l_all_trig = ['ETOS', 'GTIS']
    l_all_year = ['2011', '2012', '2015', '2016', '2017', '2018']
    l_all_brem = [0, 1, 2]

    version    = None
    l_trig     = None 
    l_year     = None 
    l_brem     = None 
#-------------------
def get_table(trig=None, year=None, brem=None):
    return {'a' : 1}
#-------------------
def save_table(trig=None, year=None, brem=None):
    d_scale=get_table(trig=trig, year=year, brem=brem)
    map_path = f'{data.cal_dir}/qsq/{data.version}/{year}.json'
    data.log.visible(f'Saving to: {map_path}')
    utnr.dump_json(d_scale, map_path)
#-------------------
def get_args():
    parser = argparse.ArgumentParser(description='Used to produce q2 smearing factors systematic tables')
    parser.add_argument('-v', '--vers' , type =str, help='Version for output maps', required=True)
    parser.add_argument('-t', '--trig' , nargs='+', help='Triggers'       , choices=data.l_all_trig, default=data.l_all_trig)
    parser.add_argument('-y', '--year' , nargs='+', help='Years'          , choices=data.l_all_year, default=data.l_all_year)
    parser.add_argument('-b', '--brem' , nargs='+', help='Brem categories', choices=data.l_all_brem, default=data.l_all_brem)
    args = parser.parse_args()

    data.version= args.vers
    data.l_trig = args.trig
    data.l_year = args.year
    data.l_brem = args.brem
#-------------------
if __name__ == '__main__':
    get_args()
    for trig in data.l_trig:
        for year in data.l_year:
            for brem in data.l_brem:
                save_table(trig=trig, year=year, brem=brem)

