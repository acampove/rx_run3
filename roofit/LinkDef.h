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

#pragma link C++ class RooExpTurnOn+;
#pragma link C++ defined_in "RooExpTurnOn.h";

#pragma link C++ class RooExpTurnOnNumerical+;
#pragma link C++ defined_in "RooExpTurnOnNumerical.h";

#pragma link C++ class RooDoubleCB+;
#pragma link C++ defined_in "RooDoubleCB.h";

#pragma link C++ class RooExpAndGauss+;
#pragma link C++ defined_in "RooExpAndGauss.h";

#pragma link C++ class RooIpatia+;
#pragma link C++ defined_in "RooIpatia.h";

#pragma link C++ class RooIpatia2+;
#pragma link C++ defined_in "RooIpatia2.h";

#pragma link C++ class RooExpGaussExp+;
#pragma link C++ defined_in "RooExpGaussExp.h";

#pragma link C++ class RooInverseArgus+;
#pragma link C++ defined_in "RooInverseArgus.h";

#pragma link C++ class RooInverseGumbel+;
#pragma link C++ defined_in "RooInverseGumbel.h";

#pragma link C++ class RooDoubleSidedCBShape+;
#pragma link C++ defined_in "RooDoubleSidedCBShape.h";

#pragma link C++ class RooBifurDSCBShape+;
#pragma link C++ defined_in "RooBifurDSCBShape.h";

#pragma link C++ class RooExpTurnOn+;
#pragma link C++ defined_in "RooExpTurnOn.h";

#pragma link C++ class RooMultiVarGaussianSuppressWarning+;
#pragma link C++ defined_in "RooMultiVarGaussianSuppressWarning.h";


#pragma link C++ class RooMultiVarGaussianNoNorm+;
#pragma link C++ defined_in "RooMultiVarGaussianNoNorm.h";

#pragma link C++ class RooDSCBShape+;
#pragma link C++ defined_in "RooDSCBShape.h";

#pragma link C++ class RooLandauAnalytical+;
#pragma link C++ defined_in "RooLandauAnalytical.h";

#pragma link C++ class RooChangeTracker2+;
#pragma link C++ defined_in "RooChangeTracker2.h";

#pragma link C++ class RooNDKeysPdf2+ ;
#pragma link C++ class RooNDKeysPdf2::BoxInfo2+;
#pragma link C++ defined_in "RooNDKeysPdf2.h";

#endif
