# Overview

This package provides a tool to determine if a secondary vertex, given
its position and uncertainty, is consistent with having been produced
within the VELO material. The package grew out of the dark photon
analysis of [LHCb-ANA-2017-027](https://twiki.cern.ch/twiki/bin/viewauth/LHCbPhysics/DarkPhoton). The tool requires ROOT 6 and can be
used in Python, compiled C++ code, and within the ROOT C++
interpreter. The following data files are included in this package:
* `CL12.0_pars.root`: parameter file fit using a line + cubic-spline +
                    semi-circle parameterization of the transverse
                    foil shape and 2011 and 2012 beam-gas data.
* `HE16.0_pars.root`: parameter file fit using a line + cubic-spline +
                    semi-circle parameterization of the transverse
                    foil shape and 2016 proton-helium SMOG data.
* `CL12.1_pars.root`: same data as CL12.0_pars.root but using a line +
                    semi-circle + semi-circle parameterization which
                    provides a marginally better fit.
* `HE16.1_pars.root`: same data as HE16.0_pars.root but using a line +
                    semi-circle + semi-circle parameterization which
                    provides a marginally better fit.

# Classes

Two main classes are provided:
* `ModuleMaterial`: stores a parameterization of the VELO modules.
* `FoilMaterial`:   stores a parameterization of the VELO RF-foil.

In both of these classes three main methods are provided:
* `integral`:  integrates the probability density function of a secondary 
             vertex position and uncertainty over the material.
* `intersect`: provides the first intersection point with material, given a 
             secondary vertex position and flight direction.
* `distance`:  returns the minimum distance in uncertainty space to the 
             material, as well as the point on the material.

There are also `config*` methods which allow the tool to be configured,
as well as additional methods which provide access to the material
parameterization.

The `VeloMaterial` class combines the Module and Foil tools and implements the VELO distance definition and
cuts as used in LHCb-ANA-2017-027.
* `tip`:       returns true if a secondary vertex position and flight direction
             intersect the tip of the RF-foil.
* `miss`:      returns true if the first expected hit, given a secondary vertex 
             position and flight direction, is missed.
* `distance`:  returns the harmonic mean of six distances - upper module,
             lower module, forward upper foil, forward lower foil, backward 
	         upper foil, backward lower foil. 

Note that the distance methods for both the module and foil provide a
4-vector, where the first three values are the x,y,z-position on the
module or foil of the minimum distance in uncertainty space, and the
4th component is the distance in uncertainty space. For the combined
tool, only the distance in uncertainty space is returned.

# Parameterization

The parameterization files contain function strings (fnc), vectors of
function parameters (par), and vectors of fit uncertainty on function
parameters (err). The object prefixes correspond to the following:
* `module_<unit>_a_z_[fnc,par,err]`: defines the function used to fit
             the z-positions of the sensors for a given module
             unit. This can be either a single sensor (i.e. pile-up
             units) or a double sensor.
* `module_<unit>_[0,1]_[l,u]_[fnc,par,err]`: provides the function which
             defines the sensor in the transverse plane. Here 0 or 1
             indicates the first or second sensor (if it exists) and l
             and u indicate if the sensor is in the lower or upper
             half of the VELO (in x).
* `foil_[l,u]_[b,c,t,f]_fnc`: provides the function which defines the
             foil in the transverse plane for the lower (l) or upper
             (u) half and the backwards (b), central (c), transition
             (t), and forward (f) segments of the foil.
* `foil_[l,u]_[b,c,t,f]_lim`: defines the z-limits for the segment.
* `foil_[l,u]_[b,c,t,f]_[0-9]_[fnc,par,err]`: defines the functions
             which provide the parameters for the transverse foil
             function.
* `foil_[l,u]_[b,c,t,f]_[0-9]_[spl]`: provides splines instead of
             functions for the parameters for the transverse foil
             function.

For both the module and foil function definitions in the transverse
plane, the first parameter must always be the y-position of the
function and the second parameter the x-position.

# Documentation

The documentation can be found in this [note](https://twiki.cern.ch/twiki/pub/LHCbPhysics/DarkPhoton/Material.INT.v1.pdf)
