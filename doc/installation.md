# Installation

For this, install micromamba, following the instructions [here](https://mamba.readthedocs.io/en/latest/user_guide/micromamba.html)
Then create a micromamba environment and install the code inside it.
```bash
# create an environment with ROOT and python
micromamba create -n rx_run3 root==6.32.10 python==3.12.11

# Activate it
micromamba activate rx_run3

git clone git@github.com:acampove/rx_run3.git

# go to the directory with the code
cd rx_run3

# And install it
pip install -r requirements.txt
```
For development use `requirements.txt`, which

- Will install stub files to hide type annotation errors
- Will install in editable mode
