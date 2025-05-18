
import uproot
import dask
import dask.dataframe as dd
import pandas as pd

# List of ROOT files
root_files = ['/path/to/file1.root', '/path/to/file2.root', ...]

# Function to load one ROOT file into pandas DataFrame (lazy)
@dask.delayed
def load_root_file(file_path):
    with uproot.open(file_path) as file:
        tree = file['myTree']
        df = tree.arrays(library='pd')  # convert to pandas DataFrame
    return df

# Create a list of delayed DataFrames
delayed_dfs = [load_root_file(f) for f in root_files]

# Convert delayed objects to a Dask DataFrame
ddf = dd.from_delayed(delayed_dfs)

# Now you can use `ddf` as a normal Dask DataFrame,
# then convert batches to PyTorch tensors as before
