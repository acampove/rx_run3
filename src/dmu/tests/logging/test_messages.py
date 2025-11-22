'''
Module with tests for utilities in messages.py module
'''
import os
from io                    import StringIO
from dmu.logging           import messages as mes
from dmu.logging.log_store import LogStore

log = LogStore.add_logger('dmu:test_messages')
# -------------------------
def test_filter_stderr():
    '''
    Will test context manager meant to filter messages going to stderr
    '''
    banned = ['ONE', 'TWO']
    buffer = StringIO()

    with mes.filter_stderr(
        banned_substrings=banned,
        capture_stream   =buffer):

        os.write(2, b'MSG ONE\n')
        os.write(2, b'MSG TWO\n')
        os.write(2, b'MSG THREE\n')

    data = buffer.getvalue()

    assert 'MSG ONE'   not in data
    assert 'MSG TWO'   not in data
    assert 'MSG THREE'     in data
# -------------------------
