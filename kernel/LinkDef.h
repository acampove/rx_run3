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
//simple functions in HelperSvc.hpp would not be available at the prompt

#include "MessageSvc.hpp"

#pragma link C++ class ConfigHolder+;
#pragma link C++ defined_in "ConfigHolder.hpp";

#pragma link C++ namespace PDG;
#pragma link C++ namespace PDG::Mass;
#pragma link C++ namespace PDG::ID;
#pragma link C++ namespace PDG::BF;
#pragma link C++ defined_in "ConstDef.hpp";

#pragma link C++ defined_in "TruthMatchingSvc.hpp";

#pragma link C++ class CutHolder+;
#pragma link C++ defined_in "CutHolder.hpp";
#pragma link C++ class CutHolderRKst+;
#pragma link C++ defined_in "CutHolderRKst.hpp";
#pragma link C++ class CutHolderRK+;
#pragma link C++ defined_in "CutHolderRK.hpp";
#pragma link C++ class CutHolderRPhi+;
#pragma link C++ defined_in "CutHolderRPhi.hpp";
#pragma link C++ class CutHolderRL+;
#pragma link C++ defined_in "CutHolderRL.hpp";
#pragma link C++ class CutHolderRKS+;
#pragma link C++ defined_in "CutHolderRKS.hpp";

#pragma link C++ class EventType+;
#pragma link C++ defined_in "EventType.hpp";

#pragma link C++ namespace CutDefRX;
#pragma link C++ defined_in "CutDefRX.hpp";
#pragma link C++ namespace CutDefRKst;
#pragma link C++ defined_in "CutDefRKst.hpp";
#pragma link C++ namespace CutDefRK;
#pragma link C++ defined_in "CutDefRK.hpp";
#pragma link C++ namespace CutDefRPhi;
#pragma link C++ defined_in "CutDefRPhi.hpp";
#pragma link C++ namespace CutDefRL;
#pragma link C++ defined_in "CutDefRL.hpp";
#pragma link C++ namespace CutDefRKS;
#pragma link C++ defined_in "CutDefRKS.hpp";

#pragma link C++ namespace WeightDefRX;
#pragma link C++ defined_in "WeightDefRX.hpp";

#pragma link C++ namespace pyPrj;
#pragma link C++ enum pyPrj::Prj;
#pragma link C++ namespace pyAnalysis;
#pragma link C++ enum pyAnalysis::Analysis;
#pragma link C++ namespace pySample;
#pragma link C++ enum pySample::Sample;
#pragma link C++ namespace pyQ2Bin;
#pragma link C++ enum pyQ2Bin::Q2Bin;
#pragma link C++ namespace pyYear;
#pragma link C++ enum pyYear::Year;
#pragma link C++ namespace pyPolarity;
#pragma link C++ enum pyPolarity::Polarity;
#pragma link C++ namespace pyTrigger;
#pragma link C++ enum pyTrigger::Trigger;
#pragma link C++ namespace pyTriggerConf;
#pragma link C++ enum pyTriggerConf::TriggerConf;
#pragma link C++ namespace pyBrem;
#pragma link C++ enum pyBrem::Brem;
#pragma link C++ namespace pyPdfType;
#pragma link C++ enum pyPdfType::PdfType;
#pragma link C++ namespace pyOpenMode;
#pragma link C++ enum pyOpenMode::OpenMode;
#pragma link C++ namespace pyBlindMode;
#pragma link C++ enum pyBlindMode::BlindMode;
#pragma link C++ defined_in "EnumeratorSvc.hpp";


/*
This when saving fits to disk...
Warning in <TStreamerInfo::Build>: pair<TString,tuple<bool,double,double> >: tuple<bool,double,double> has no streamer or dictionary, data member "second" will not be saved
Warning in <TStreamerInfo::Build>: pair<TString,tuple<EventType,TString,TString> >: tuple<EventType,TString,TString> has no streamer or dictionary, data member "second" will not be saved
Warning in <TStreamerInfo::Build>: pair<pySample::Sample,tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> >: tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> has no streamer or dictionary, data member "second" will not be saved
Warning in <TStreamerInfo::Build>: pair<pyBrem::Brem,tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> >: tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> has no streamer or dictionary, data member "second" will not be saved
Warning in <TStreamerInfo::Build>: pair<pair<FitParameterConfig,pyRatioType::RatioType>,ParameterWrapper>: pair<FitParameterConfig,pyRatioType::RatioType> has no streamer or dictionary, data member "first" will not be saved
*/
#pragma link C++ class std::tuple<EventType,TString,TString>+;
#pragma link C++ class std::pair<pySample::Sample,std::tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> >+;
#pragma link C++ class std::pair<pySample::Sample,std::tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> >+;
#pragma link C++ class std::pair<pyBrem::Brem,std::tuple<pyPdfType::PdfType,TString,TString,TString,TString,double> >+;
#pragma link C++ class std::tuple<pyPdfType::PdfType,TString,TString,TString,TString,double>+; //needed as private in FitConfiguration.hpp


#pragma link C++ class FitConfiguration+;
#pragma link C++ defined_in "FitConfiguration.hpp";

#pragma link C++ class EfficiencyForFitHandler+;
#pragma link C++ class InfoEff+;
#pragma link C++ defined_in "EfficiencyForFitHandler.hpp";

#pragma link C++ defined_in "HistogramSvc.hpp";



#pragma link C++ class IdChains+;
#pragma link C++ class RescalerRXSamples+;
#pragma link C++ defined_in "XJPsRescaler.hpp";

#pragma link C++ struct SplitInfo+;
#pragma link C++ function Reweight2XJPs+;
#pragma link C++ defined_in "HelperSvc.hpp";

#pragma link C++ class IOSvc+;
#pragma link C++ defined_in "IOSvc.hpp";

#pragma link C++ enum Color;
#pragma link C++ class MessageSvc+;
#pragma link C++ defined_in "MessageSvc.hpp";

#pragma link C++ class ParserSvc+;
#pragma link C++ defined_in "ParserSvc.hpp";

#pragma link C++ defined_in "PhysicsSvc.hpp";

#pragma link C++ defined_in "LaTable.hpp";

#pragma link C++ namespace SettingDef;
#pragma link C++ class SettingDef::IO+;
#pragma link C++ class SettingDef::Config+;
#pragma link C++ class SettingDef::Cut+;
#pragma link C++ class SettingDef::Weight+;
#pragma link C++ class SettingDef::Tuple+;
#pragma link C++ class SettingDef::Events+;
#pragma link C++ class SettingDef::Efficiency+;
#pragma link C++ class SettingDef::Fit+;
#pragma link C++ class SettingDef::AllowedConf+;
#pragma link C++ namespace gRX+;
#pragma link C++ defined_in "SettingDef.hpp";

#pragma link C++ namespace Tex;
#pragma link C++ defined_in "StyleSvc.hpp";

#pragma link C++ class TupleHolder+;
#pragma link C++ defined_in "TupleHolder.hpp";

#pragma link C++ class TupleReader+;
#pragma link C++ defined_in "TupleReader.hpp";

#pragma link C++ class WeightHolder+;
#pragma link C++ defined_in "WeightHolder.hpp";
#pragma link C++ class WeightHolderRX+;
#pragma link C++ defined_in "WeightHolderRX.hpp";
#pragma link C++ class WeightHolderRL+;
#pragma link C++ defined_in "WeightHolderRL.hpp";

#pragma link C++ namespace pdgrounding;
#pragma link C++ defined_in "PdgRounding.hpp";



#endif
