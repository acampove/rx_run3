
#-------------------------------------------------------------------------------------------
class transformer:
    '''
    Class used to apply transformations to text files
    '''
    def __init__(self, txt_path=None, cfg_path=None):
        '''
        txt_path (str): Path to text file to be transformed
        cfg_path (str): Path to TOML file holding configuration needed for transformations 
        '''
        self._txt_path = txt_path
        self._cfg_path = cfg_path
    #-----------------------------------------
    def save_as(self, out_path=None):
        '''
        Saves text file after transformation to `out_path`
        If no path is passed, will name as:

        /some/dir/file.txt -> /some/dir/file_trf.txt
        '''

        return
#-------------------------------------------------------------------------------------------
