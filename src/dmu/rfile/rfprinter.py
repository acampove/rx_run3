'''
Module containing RFPrinter
'''
import os

from ROOT import TFile

from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:rfprinter')
#--------------------------------------------------
class RFPrinter:
    '''
    Class meant to print summary of ROOT file
    '''
    #-----------------------------------------
    def __init__(self, path : str):
        '''
        Takes path to root file
        '''
        if not os.path.isfile(path):
            raise FileNotFoundError(f'Cannot find {path}')

        self._root_path = path
        self._text_path = path.replace('.root', '.txt')
    #-----------------------------------------
    def _get_trees(self, ifile):
        '''
        Takes TFile object, returns list of TTree objects
        '''
        l_key=ifile.GetListOfKeys()

        l_tree=[]
        for key in l_key:
            obj=key.ReadObj()
            if obj.InheritsFrom("TTree"):
                fname=ifile.GetName()
                tname=obj.GetName()

                title=f'{fname}/{tname}'
                obj.SetTitle(title)
                l_tree.append(obj)
            elif obj.InheritsFrom("TDirectory"):
                l_tree+=self._get_trees(obj)

        return l_tree
    #---------------------------------
    def _get_tree_info(self, tree):
        '''
        Takes ROOT tree, returns list of strings with information about tree
        '''
        l_branch= tree.GetListOfBranches()
        l_line  = []
        for branch in l_branch:
            bname = branch.GetName()
            leaf  = branch.GetLeaf(bname)
            btype = leaf.GetTypeName()

            l_line.append(f'{"":4}{bname:<100}{btype:<40}')

        return l_line
    #-----------------------------------------
    def _save_info(self, l_info):
        '''
        Takes list of strings, saves it to text file
        '''

        with open(self._text_path, 'w', encoding='utf-8') as ofile:
            for info in l_info:
                ofile.write(f'{info}\n')

        log.info(f'Saved to: {self._text_path}')
    #-----------------------------------------
    def save(self, to_screen=False):
        '''
        Will save a text file with the summary of the ROOT file contents

        to_screen (bool) : If true, will print to screen, default=False
        '''
        l_info = []
        log.info(f'Reading from : {self._root_path}')
        with TFile.Open(self._root_path) as ifile:
            l_tree = self._get_trees(ifile)
            for tree in l_tree:
                l_info+= self._get_tree_info(tree)

        self._save_info(l_info)
        if to_screen:
            for info in l_info:
                log.info(info)
#-----------------------------------------
