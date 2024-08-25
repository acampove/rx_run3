from data_checks.ntuple_filter import ntuple_filter

#---------------------------------------
def test_simple():
    obj=ntuple_filter(cfg='dt_2014_turbo', index=1)
    obj.filter()
#---------------------------------------
def  main():
    test_simple()
#---------------------------------------
if __name__  == '__main__':
    main()
