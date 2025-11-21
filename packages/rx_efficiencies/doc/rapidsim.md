# Installation

The steps are shown in the [rapidsim](https://github.com/gcowan/RapidSim)
page. However, a modified version that makes work easier is:

```bash
# Activate the environmetn
micromamba activate myenv

git clone git@github.com:gcowan/RapidSim.git

# Edit the CMakeLists such that the second line reads
# CMAKE_MINIMUM_REQUIRED( VERSION 3.5 FATAL_ERROR )
cd RapidSim
mkdir build
cd build 
cmake ../ -DCMAKE_INSTALL_PREFIX=$HOME/.local
make -j4
make -j4 install
```

make sure that `$HOME/.local` is in the `PATH`.

```bash
cd $HOME/.local/bin

ln -s $PWD/RapidSim.exe rapidsim
chmod +x rapidsim

# Add this to the .bashrc
export RAPIDSIM_ROOT=$HOME/.local

Now you can use this program by using `rapidsim`.
