'''
Module holding PChain class
'''
# pylint: disable=invalid-name
# pylint: disable=missing-function-docstring

from dmu.logging.log_store import LogStore

log=LogStore.add_logger('fitter:pchain')
#----------------------------------
class PChain:
    '''
    Class meant to represent a decay chain
    '''
    #----------------------------------
    def __init__(self, pid, mid, gmid, ggmid):
        self._TID          = pid
        self._MOTHER_TID   = mid
        self._GMOTHER_TID  = gmid
        self._GGMOTHER_TID = ggmid
    #----------------------------------
    @property
    def id(self) -> int:
        return self._TID

    @property
    def mother_id(self) -> int:
        return self._MOTHER_TID

    @property
    def gmother_id(self) -> int:
        return self._GMOTHER_TID

    @property
    def ggmother_id(self) -> int:
        return self._GGMOTHER_TID
    #----------------------------------------------------
    def match_decay(self, l_dec_id):
        if len(l_dec_id) == 1:
            return self._TID ==      l_dec_id[0]
        if len(l_dec_id) == 2:
            return  self._TID == abs(l_dec_id[0]) and self._MOTHER_TID == abs(l_dec_id[1])
        if len(l_dec_id) == 3:
            return  self._TID == abs(l_dec_id[0]) and self._MOTHER_TID == abs(l_dec_id[1]) and self._GMOTHER_TID == abs(l_dec_id[2])
        if len(l_dec_id) == 4:
            return  self._TID == abs(l_dec_id[0]) and self._MOTHER_TID == abs(l_dec_id[1]) and self._GMOTHER_TID == abs(l_dec_id[2]) and self._GGMOTHER_TID == abs(l_dec_id[3])

        return False
    #----------------------------------------------------
    def has_in_chain(self, ID):
        _return = False

        if self._MOTHER_TID   == ID:
            _return = True
        if self._GMOTHER_TID  == ID:
            _return = True
        if self._GGMOTHER_TID == ID:
            _return = True

        return _return
    #----------------------------------------------------
    def match_upstream(self, IDFirstDau, HeadPart):
        _return = False
        if  self._MOTHER_TID   == IDFirstDau and self._GMOTHER_TID  == HeadPart:
            _return = True
        if  self._GMOTHER_TID  == IDFirstDau and self._GGMOTHER_TID == HeadPart:
            _return = True
        if  self._GGMOTHER_TID == IDFirstDau:
            _return = True

        return _return
    #----------------------------------------------------
    def match_id(self, iD):
        return self._TID == abs(iD)

    def match_mother(self, iD):
        return self._MOTHER_TID == abs(iD)

    def match_gmother(self, iD):
        return self._GMOTHER_TID == abs(iD)

    def match_ggmother(self, iD):
        return self._GGMOTHER_TID == abs(iD)
#----------------------------------------------------
