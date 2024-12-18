// Definitions for CINT and PYTHON interface of tuples
// For info have a look at
// https://root.cern.ch/selecting-dictionary-entries-linkdefh
// https://root.cern.ch/root/htmldoc/guides/users-guide/AddingaClass.html#other-useful-pragma-statements

#ifdef __ROOTCLING__

#include "itertools.hpp"
#include <boost/lexical_cast.hpp>

#pragma link off all class;
//#pragma link off all enum;
#pragma link C++ all function;
//#pragma link off all global;
#pragma link off all namespace;
//#pragma link off all struct;
//#pragma link off all typedef;
//#pragma link off all union;
#pragma link C++ all function;
#pragma link C++ all functions; //necessary to have functions available!

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ class EfficiencyHolder+;
#pragma link C++ defined_in "EfficiencyHolder.hpp";

#pragma link C++ class EfficiencyCalculator+;
#pragma link C++ struct PlotInfo;
#pragma link C++ defined_in "EfficiencyCalculator.hpp";

#pragma link C++ class VariableBinning+;
#pragma link C++ defined_in "VariableBinning.hpp";

// #pragma link C++ all namespace; //necessary to have functions available!
// #pragma link C++ all class; //necessary to have functions available!
#pragma link C++ class BinnedEffs+;
#pragma link C++ struct EfficiencyInfos+;

#pragma link C++ defined_in "EfficiencySvc.hpp";

#pragma link C++ class RXWeight+;
#pragma link C++ class RXSelection+;
#pragma link C++ class RXVarPlot+;
#pragma link C++ class RXDataPlayer+;
#pragma link C++ defined_in "RXDataPlayer.hpp";

#pragma ling C++ class CounterHelpers+;
#pragma link C++ defined_in "CounterHelpers.hpp";

#pragma link C++ class FlatnessHelpers+;
#pragma link C++ defined_in "FlatnessHelpers.hpp";

#pragma link C++ struct IsoBinningInputs+;
#pragma link C++ defined_in "IsoHelper.hpp";

#endif
