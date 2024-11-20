'''
Module with testing functions for functions in rfile/utilities.py
'''

import dmu.rfile.utilities   as rfut
import dmu.testing.utilities as tsut

def test_get_trees_from_files():
    '''
    Test function meant to provide trees from file handle
    '''
    ifile = tsut.get_file_with_trees('/tmp/file.root')

    d_tree = rfut.get_trees_from_file(ifile)
    l_tree = list(d_tree.values())
    l_name = [ tree.GetName() for tree in l_tree]

    assert l_name == ['a', 'b', 'c']
