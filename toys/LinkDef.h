// Definitions for CINT and PYTHON interface of tuples
// For info have a look at
// https://root.cern.ch/selecting-dictionary-entries-linkdefh
// https://root.cern.ch/root/htmldoc/guides/users-guide/AddingaClass.html#other-useful-pragma-statements

#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace PoissonInverse;
#pragma link C++ defined_in "poissoninv.hpp";

#pragma link C++ class ToyIO+;
#pragma link C++ defined_in "ToyIO.hpp";

#pragma link C++ class ToyFileHandler+;
#pragma link C++ defined_in "ToyFileHandler.hpp";

#pragma link C++ class ToyGenerator+;
#pragma link C++ defined_in "ToyGenerator.hpp";

#pragma link C++ class ToyReader+;
#pragma link C++ defined_in "ToyReader.hpp";

#pragma link C++ class ToyStudy+;
#pragma link C++ defined_in "ToyStudy.hpp";

#pragma link C++ class ToyTupleComponentGenerator+;
#pragma link C++ defined_in "ToyTupleComponentGenerator.hpp";

#pragma link C++ class ToyTupleComponentHeader+;
#pragma link C++ defined_in "ToyTupleComponentHeader.hpp";

#pragma link C++ class ToyTupleComponentReader+;
#pragma link C++ defined_in "ToyTupleComponentReader.hpp";

#pragma link C++ class ToyTupleConfig+;
#pragma link C++ defined_in "ToyTupleConfig.hpp";

#pragma link C++ class ToyTupleConfigGenerator+;
#pragma link C++ defined_in "ToyTupleConfigGenerator.hpp";

#pragma link C++ class ToyTupleConfigLoader+;
#pragma link C++ defined_in "ToyTupleConfigLoader.hpp";

#pragma link C++ class ToyTupleConfigSaver+;
#pragma link C++ defined_in "ToyTupleConfigSaver.hpp";

#pragma link C++ class ToyTupleGenerator+;
#pragma link C++ defined_in "ToyTupleGenerator.hpp";

#pragma link C++ class ToyTupleHeaderHandler+;
#pragma link C++ defined_in "ToyTupleHeaderHandler.hpp";

#pragma link C++ class ToyTupleReader+;
#pragma link C++ defined_in "ToyTupleReader.hpp";

#pragma link C++ class ToyYieldConfig+;
#pragma link C++ defined_in "ToyYieldConfig.hpp";

#pragma link C++ class VariableResetter+;
#pragma link C++ defined_in "VariableResetter.hpp";

#pragma link C++ class VariableSmearer+;
#pragma link C++ defined_in "VariableSmearer.hpp";

#pragma link C++ class CorrelatedVariablesSmearer+;
#pragma link C++ defined_in "CorrelatedVariablesSmearer.hpp";

#endif