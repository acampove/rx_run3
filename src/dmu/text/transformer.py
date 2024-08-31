
import os
import toml
import numpy
import pprint
import logging

log=logging.getLogger('dmu:text:transformer')
#-------------------------------------------------------------------------------------------
class transformer:
    '''
    Class used to apply transformations to text files
    '''
    #-----------------------------------------
    def __init__(self, txt_path=None, cfg_path=None):
        '''
        txt_path (str): Path to text file to be transformed, can have any extension, py, txt, log, etc
        cfg_path (str): Path to TOML file holding configuration needed for transformations 
        '''
        self._txt_path = txt_path
        self._cfg_path = cfg_path
        self._suffix   = 'trf'

        self._l_line   = None
        self._cfg      = None

        self._initialized=False
    #-----------------------------------------
    def _initialize(self):
        if self._initialized:
            return

        self._check_file(self._txt_path)
        self._check_file(self._cfg_path)
        self._load_input()
        self._cfg = toml.load(self._cfg_path)

        self._initialized=True
    #-----------------------------------------
    def _check_file(self, file_path):
        '''
        Will raise exception if path not found

        file_path (str): path to file
        '''
        if not os.path.isfile(file_path):
            raise FileNotFoundError(f'File not found: {file_path}')

        log.debug(f'Found: {file_path}')
    #-----------------------------------------
    def _load_input(self):
        '''
        Will open  self._txt_path and put the lines in self._l_line
        '''
        with open(self._txt_path) as ifile:
            self._l_line = ifile.read().splitlines()

            nline = len(self._l_line)
            log.info(f'Found {nline} lines in {self._txt_path}')
    #-----------------------------------------
    def _get_out_path(self, out_path):
        '''
        Will return name of output file
        If arg is not None, will make directory (in case it does not exist) and return arg
        If arg is None, will rename input path using suffix  and return
        '''
        if out_path is not None:
            dir_name = os.path.dirname(out_path)
            os.makedirs(dir_name, exist_ok=True)

            return out_path

        file_name = os.path.basename(self._txt_path)
        if '.' not in file_name:
            return f'{file_name}_{self._suffix}'

        l_part     = file_name.split('.')
        bef_ext    = l_part[-2]
        l_part[-2] = f'{bef_ext}_{self._suffix}'

        file_name  = '.'.join(l_part)
        file_dir   = os.path.dirname(self._txt_path)

        return f'{file_dir}/{file_name}'    
    #-----------------------------------------
    def _transform(self, l_line, trf):
        log.info(f'{"":<4}{trf}')

        if trf == 'append':
            return self._apply_append(l_line)
        else:
            raise ValueError(f'Invalid transformation: {trf}')

        return l_line
    #-----------------------------------------
    def _apply_append(self, l_line):
        d_append = self._cfg['append']
        for target, l_to_be_added in d_append.items():
            arr_line   = numpy.array(self._l_line)
            arr_index, = numpy.where(arr_line == target)

            if arr_index.size  == 0:
                pprint.pprint(self._l_line)
                raise RuntimeError(f'No instance of {target} found in {self._txt_path}')

            for index in arr_index:
                log.debug(f'Inserting at {index}')
                l_line[index+1:index+1] = l_to_be_added

        return l_line
    #-----------------------------------------
    def save_as(self, out_path=None):
        '''
        Saves text file after transformation to `out_path`
        If no path is passed, will name as:

        /some/dir/file.txt -> /some/dir/file_trf.txt
        '''
        self._initialize()

        log.info(20 * '-')
        log.info('Applying transformations')
        log.info(20 * '-')
        for trf in  self._cfg:
            self._l_line = self._transform(self._l_line, trf)

        out_path = self._get_out_path(out_path)
        log.info(f'Saving to: {out_path}')
        with open(out_path, 'w') as ofile:
            text = '\n'.join(self._l_line)
            ofile.write(text)
#-------------------------------------------------------------------------------------------
