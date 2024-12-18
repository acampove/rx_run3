// Definitions for CINT and PYTHON interface of tuples
// For info have a look at
// https://root.cern.ch/selecting-dictionary-entries-linkdefh
// https://root.cern.ch/root/htmldoc/guides/users-guide/AddingaClass.html#other-useful-pragma-statements

#ifdef __ROOTCLING__

#pragma link off all class;
//#pragma link off all enum;
//#pragma link off all function;
//#pragma link off all global;
#pragma link off all namespace;
//#pragma link off all struct;
//#pragma link off all typedef;
//#pragma link off all union;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ class FoilMaterial+;
#pragma link C++ class ModuleMaterial+;
#pragma link C++ class VeloMaterial+;
#pragma link C++ defined_in "VeloMaterial.hpp";

#endif
