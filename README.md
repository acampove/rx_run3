# $R_X$ run3

This is the repository for the combined measurement of $R_K$ and $R_{K^*}$
in run 3

## Installation

For this do:

```bash
# Install UV
curl -LsSf https://astral.sh/uv/install.sh | sh

git clone git@github.com:acampove/rx_run3.git

cd rx_run3

# Install project in virtual environment
uv sync

# Or install with development dependencies
uv sync --extra dev

# Activate the virtual environment
source .venv/bin/activate
```

One can also use `uv run command` to run commands inside the environment.
However it is more practical to be fully inside the environment.
To make this process less cumbersome one can add:

```bash
uvshell()
{
    if [ ! -f "$PWD/.venv/bin/activate" ];then
        echo "No UV environment found"
        return
    fi

    source "$PWD/.venv/bin/activate"
}
```

to `$HOME/.bashrc` such that one would only call `uvshell` to enter the evironment
