from data_checks.filter_file import FilterFile
from log_store               import log_store

#--------------------------------------
class data:
    file_path='/home/acampove/data/aprod/00231366_00000013_1.ftuple.root'
#--------------------------------------
def set_log():
    log_store.set_level('data_checks:FilterFile', 10)
#--------------------------------------
def test_simple():
    obj=FilterFile(file_path=data.file_path, cfg_nam='dt_2014_turbo')
    obj.run()
#--------------------------------------
def main():
    set_log()
    test_simple()
#--------------------------------------
if __name__ == '__main__':
    main()

