'''
File with GPU related functionality
'''

from dmu        import LogStore
from .imports   import tf_config

log=LogStore.add_logger('dmu:stats:gpu')
# ----------------------
def check_for_gpu() -> None:
    '''
    Check if we are using the GPU, this assumes Tensorflow is installed
    '''
    gpus = tf_config.list_physical_devices('GPU')

    if gpus:
        log.info(f"GPU is available: {gpus}")
    else:
        log.info("GPU is not available, using CPU.")
# ----------------------
