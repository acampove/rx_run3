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


#pragma link C++ class TupleProcess+;
#pragma link C++ defined_in "TupleProcess.hpp";

#pragma link C++ class TupleCreate+;
#pragma link C++ defined_in "TupleCreate.hpp";


#pragma link C++ class TH1DHistoAdder+;
#pragma link C++ class TH1FHistoAdder+;

#pragma link C++ class BSTH1DHistoAdder+;
#pragma link C++ class TH2DHistAdder+;
#pragma link C++ class BSTH2DHistAdder+;
#pragma link C++ class PIDHistoAdder+;
#pragma link C++ class PIDHistoAdderBSKDE+;
#pragma link C++ class PIDHistoAdderBSKDE+;
#pragma link C++ class PIDHistoAdderBS_SMEAR+;
#pragma link C++ class Q2SmearCorrection+;
#pragma link C++ class Q2SmearCorrection+;
#pragma line C++ class DecModelWeightsAdder+;
#pragma link C++ defined_in "HistoAdders.hpp";


#pragma link C++ class Functors+;
#pragma link C++ struct Functors+;

#pragma link C++ defined_in "Functors.hpp";

#pragma link C++ struct BranchPort+;
#pragma link C++ struct TrackHLT1Info+;
#pragma link C++ class TrackMVAHLT1Info+;
#pragma link C++ class HelperProcessing+;
#pragma link C++ defined_in "HelperProcessing.hpp";


#pragma link C++ namespace PQWeights++;
#pragma link C++ defined_in "PQWeights.hpp";

#endif
