// Definitions for CINT and PYTHON interface of tuples
// For info have a look at
// https://root.cern.ch/selecting-dictionary-entries-linkdefh
// https://root.cern.ch/root/htmldoc/guides/users-guide/AddingaClass.html#other-useful-pragma-statements

#ifdef __ROOTCLING__

#pragma link off all class;
//#pragma link off all enum;
#pragma link C++ all function;
//#pragma link off all global;
#pragma link off all namespace;
//#pragma link off all struct;
//#pragma link off all typedef;
//#pragma link off all union;

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedef;

// See https://root-forum.cern.ch/t/streamerinfo-build-when-reading-custom-classes-warning-on-std-tuple-and-std-pair/43178
#pragma link C++ class std::tuple<bool,double,double>+;
#pragma link C++ class std::pair<TString,std::tuple<bool,double,double>>+;

#pragma link C++ class FitComponent+;
#pragma link C++ defined_in "FitComponent.hpp";

#pragma link C++ class FitComponentAndYield+;
#pragma link C++ defined_in "FitHolder.hpp";

#pragma link C++ class FitHolder+;
#pragma link C++ defined_in "FitHolder.hpp";

#pragma link C++ class FitManager+;
#pragma link C++ defined_in "FitManager.hpp";

#pragma link C++ class FitGenerator+;
#pragma link C++ defined_in "FitGenerator.hpp";

#pragma link C++ class FitterTool+;
#pragma link C++ defined_in "FitterTool.hpp";

#pragma link C++ class FitParameterPool-;
#pragma link C++ class FitParameterSnapshot+;
#pragma link C++ class ParameterWrapper+;
#pragma link C++ namespace RXFitter;

#pragma link C++ class std::pair<FitParameterConfig,pyRatioType::RatioType>+;
#pragma link C++ class std::pair<std::pair<FitParameterConfig,pyRatioType::RatioType>,ParameterWrapper>+;

#pragma link C++ defined_in "FitParameterPool.hpp";

#pragma link C++ class FitParameterConfig+;
#pragma link C++ defined_in "FitParameterConfig.hpp";

// Warning in <TStreamerInfo::Build>: pair<pair<FitParameterConfig,pyRatioType::RatioType>,ParameterWrapper>: pair<FitParameterConfig,pyRatioType::RatioType> has no streamer or dictionary, data member "first" will not be saved

#pragma link C++ class ConstraintsGenerator+;
#pragma link C++ defined_in "ConstraintsGenerator.hpp";

#pragma link C++ class UnblindParameter+;
#pragma link C++ class UnblindOffsetScale+;
#pragma link C++ defined_in "UnblindParameter.hpp";

#pragma link C++ defined_in "FitterSvc.hpp";

#pragma link C++ class SPlot2+;
#pragma link C++ defined_in "SPlot2.hpp";

#pragma link C++ class FitResultLogger+;
#pragma link C++ defined_in "FitResultLogger.hpp";

#pragma link C++ class AdaptiveNLL+;
#pragma link C++ defined_in "BinnedLikelihood.h";

#pragma link C++ class RombergNLL+;
#pragma link C++ defined_in "BinnedLikelihood.h";

#pragma link C++ class AnalyticalNLL+;
#pragma link C++ defined_in "BinnedLikelihood.h";

#pragma link C++ class RooProfileLLRX+;
#pragma link C++ defined_in "RooProfileLLRX.hpp";
#endif
