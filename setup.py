from setuptools import setup, find_packages

import glob

setup(
    name                ='q2_syst',
    version             ='0.0.1',
    description         ='Project used to calculate q2 smearing factors and the systematics',
    long_description    ='',
    scripts             = glob.glob('scripts/*'),
    package_dir         = {'' : 'src'},
    install_requires    = open('requirements.txt').read().splitlines(),
)

