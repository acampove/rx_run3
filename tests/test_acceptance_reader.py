from acceptance_reader import reader as acreader

#------------------------------------
def test_simple():
    obj=acreader(year='2011', proc='bsph')
    acc=obj.read()
#------------------------------------
def main():
    test_simple()
#------------------------------------
if __name__ == '__main__':
    main()

