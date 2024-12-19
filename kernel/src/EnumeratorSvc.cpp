#ifndef ENUMERATORSVC_CPP
#define ENUMERATORSVC_CPP

#include "EnumeratorSvc.hpp"
#include "MessageSvc.hpp"

#include "TString.h"
#include <stdexcept>

//======================== Projects
const Prj pyPrj::hash_project(const TString & _string) {
    if (_string == "") return Prj::All;
    if (_string == "RKst") return Prj::RKst;
    if (_string == "RK") return Prj::RK;
    if (_string == "RPhi") return Prj::RPhi;
    if (_string == "RL") return Prj::RL;
    if (_string == "RKS") return Prj::RKS;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_project(\"", _string, "\") failed", "EXIT_FAILURE");
    return Prj::Error;
}
TString pyPrj::to_string(const Prj & _enum) {
    switch (_enum) {
        case Prj::All: return ""; break;
        case Prj::RKst: return "RKst"; break;
        case Prj::RK: return "RK"; break;
        case Prj::RPhi: return "RPhi"; break;
        case Prj::RL: return "RL"; break;
        case Prj::RKS: return "RKS"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Prj failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Prj failed", "EXIT_FAILURE");
    return "Error";
}
TString pyPrj::to_tex(const Prj & _enum) {
    switch (_enum) {
        case Prj::All: return ""; break;
        case Prj::RKst: return TString("#it{R}(") + Tex::Kstar0 + ")"; break;
        case Prj::RK: return TString("#it{R}(") + Tex::Kaon + ")"; break;
        case Prj::RPhi: return TString("#it{R}(") + Tex::Phi + ")"; break;
        case Prj::RL: return TString("#it{R}(") + Tex::Lambda0 + ")"; break;
        case Prj::RKS: return TString("#it{R}(") + Tex::KShort + ")"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_tex Prj failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_tex Prj failed", "EXIT_FAILURE");
    return "Error";
}

//======================== Analysis
const Analysis pyAnalysis::hash_analysis(const TString & _string) {
    if (_string == "") return Analysis::All;
    if (_string.Contains("MM")) return Analysis::MM;
    if (_string.Contains("EE")) return Analysis::EE;
    if (_string.Contains("ME")) return Analysis::ME;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_analysis(\"", _string, "\") failed", "EXIT_FAILURE");
    return Analysis::Error;
}
TString pyAnalysis::to_string(const Analysis & _enum) {
    switch (_enum) {
        case Analysis::All: return ""; break;
        case Analysis::MM: return "MM"; break;
        case Analysis::EE: return "EE"; break;
        case Analysis::ME: return "ME"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Analysis failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Analysis failed", "EXIT_FAILURE");
    return "Error";
}
TString pyAnalysis::to_tex(const Analysis & _enum) {
    switch (_enum) {
        case Analysis::All: return ""; break;
        case Analysis::MM: return Tex::MM; break;
        case Analysis::EE: return Tex::EE; break;
        case Analysis::ME: return Tex::ME; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_tex Analyses failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_tex Analyses failed", "EXIT_FAILURE");
    return "Error";
}

//======================== Samples
const Sample pySample::hash_sample(const TString & _string) {
    if (_string == "") return Sample::Empty;
    if (_string == "LL") return Sample::LL;
    if (_string == "JPs") return Sample::JPsi;
    if (_string == "Psi") return Sample::Psi;
    if (_string == "Gamma") return Sample::Gamma;
    if (_string == "Bd") return Sample::Bd;
    if (_string == "Bs") return Sample::Bs;
    if (_string == "Bs2Phi") return Sample::Bs2Phi;
    if (_string == "HadSwap") return Sample::HadSwap;
    if (_string == "Psi2JPsX") return Sample::Psi2JPsX;
    if (_string == "Psi2JPsPiPi") return Sample::Psi2JPsPiPi;
    if (_string == "BdBu") return Sample::BdBu;
    if (_string == "Bd2Kst") return Sample::Bd2Kst;
    if (_string == "Bu2Kst") return Sample::Bu2Kst;
    if (_string == "Lb") return Sample::Lb;
    if (_string == "Comb") return Sample::Comb;
    if (_string == "CombSS") return Sample::CombSS;
    if (_string == "PartReco") return Sample::PartReco;
    if (_string == "PartRecoH") return Sample::PartRecoH;
    if (_string == "PartRecoHad") return Sample::PartRecoHad;
    if (_string == "DataDrivenEMisID") return Sample::DataDrivenEMisID;
    if (_string == "TemplateMisID_PIDe3") return Sample::TemplateMisID_PIDe3;
    if (_string == "DoubleMisID_PiPi") return Sample::DoubleMisID_PiPi;
    if (_string == "DoubleMisID_KK") return Sample::DoubleMisID_KK;
    if (_string == "PartRecoL") return Sample::PartRecoL;
    if (_string == "PartRecoK1") return Sample::PartRecoK1;
    if (_string == "PartRecoK2") return Sample::PartRecoK2;
    if (_string == "Leakage") return Sample::Leakage;
    if (_string == "MisID") return Sample::MisID;
    if (_string == "DSLC") return Sample::DSLC;
    if (_string == "KEtaPrime") return Sample::KEtaPrime;
    if (_string == "LPT") return Sample::Data;
    if (_string == "Custom") return Sample::Custom;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_sample(\"", _string, "\") failed", "EXIT_FAILURE");
    return Sample::Error;
}

TString pySample::to_string(const Sample & _enum) {
    switch (_enum) {
        case Sample::Empty: return ""; break;
        case Sample::LL: return "LL"; break;
        case Sample::JPsi: return "JPs"; break;
        case Sample::Psi: return "Psi"; break;
        case Sample::Gamma: return "Gamma"; break;
        case Sample::Bd: return "Bd"; break;
        case Sample::Bs: return "Bs"; break;
        case Sample::Bs2Phi: return "Bs2Phi"; break;
        case Sample::HadSwap: return "HadSwap"; break;
        case Sample::Psi2JPsX: return "Psi2JPsX"; break;
        case Sample::Psi2JPsPiPi: return "Psi2JPsPiPi"; break;
        case Sample::BdBu: return "BdBu"; break;
        case Sample::Bd2Kst: return "Bd2Kst"; break;
        case Sample::Bu2Kst: return "Bu2Kst"; break;
        case Sample::Lb: return "Lb"; break;
        case Sample::Comb: return "Comb"; break;
        case Sample::CombSS: return "CombSS"; break;
        case Sample::PartReco: return "PartReco"; break;
        case Sample::PartRecoH: return "PartRecoH"; break;
        case Sample::PartRecoHad: return "PartRecoHad"; break;
        case Sample::DataDrivenEMisID: return "DataDrivenEMisID"; break;
        case Sample::TemplateMisID_PIDe3: return "TemplateMisID_PIDe3"; break;
        case Sample::DoubleMisID_PiPi: return "DoubleMisID_PiPi"; break;
        case Sample::DoubleMisID_KK: return "DoubleMisID_KK"; break;
        case Sample::PartRecoL: return "PartRecoL"; break;
        case Sample::PartRecoK1: return "PartRecoK1"; break;
        case Sample::PartRecoK2: return "PartRecoK2"; break;
        case Sample::Leakage: return "Leakage"; break;
        case Sample::MisID: return "MisID"; break;
        case Sample::DSLC: return "DSLC"; break;
        case Sample::KEtaPrime : return "KEtaPrime"; break;
        case Sample::Data: return "LPT"; break;
        case Sample::Custom: return "Custom"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Sample failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string for Sample failed", "EXIT_FAILURE");
    return "Error";
}

const Sample pySample::GetSignalSample(const Q2Bin & _q2bin) 
{
    switch (_q2bin) 
    {
        case Q2Bin::All:     return Sample::Empty;
        case Q2Bin::Low:     return Sample::LL;   
        case Q2Bin::Central: return Sample::LL;   
        case Q2Bin::High:    return Sample::LL;   
        case Q2Bin::JPsi:    return Sample::JPsi; 
        case Q2Bin::Psi:     return Sample::Psi;  
        case Q2Bin::Gamma:   return Sample::Gamma;
        default:
            throw std::invalid_argument("EnumeratorSvc", (TString) "GetSignalSample failed", "EXIT_FAILURE"); 
    }
}

//======================== Q2 Regions
const Q2Bin pyQ2Bin::hash_q2bin(const TString & _string) {
    if (_string == "") return Q2Bin::All;
    if (_string == "low") return Q2Bin::Low;
    if (_string == "central") return Q2Bin::Central;
    if (_string == "high") return Q2Bin::High;
    if (_string == "jps") return Q2Bin::JPsi;
    if (_string == "psi") return Q2Bin::Psi;
    if (_string == "gamma") return Q2Bin::Gamma;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_q2bin(\"", _string, "\") failed", "EXIT_FAILURE");
    return Q2Bin::Error;
}
TString pyQ2Bin::to_string(const Q2Bin & _enum) {
    switch (_enum) {
        case Q2Bin::All: return ""; break;
        case Q2Bin::Low: return "low"; break;
        case Q2Bin::Central: return "central"; break;
        case Q2Bin::High: return "high"; break;
        case Q2Bin::JPsi: return "jps"; break;
        case Q2Bin::Psi: return "psi"; break;
        case Q2Bin::Gamma: return "gamma"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Q2Bin failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Q2Bin failed", "EXIT_FAILURE");
    return "Error";
}
TString pyQ2Bin::to_tex(const Q2Bin & _enum) {
    switch (_enum) {
        case Q2Bin::All: return ""; break;
        case Q2Bin::Low: return "#scale[0.5]{#it{q}^{2}_{#it{low}} }"; break;
        case Q2Bin::Central: return "#scale[0.5]{#it{q}^{2}_{#it{central}} }"; break;
        case Q2Bin::High: return "#scale[0.5]{#it{q}^{2}_{#it{high}} }"; break;
        case Q2Bin::JPsi: return "#scale[0.5]{#it{q}^{2}_{" + Tex::JPsi + "} }"; break;
        case Q2Bin::Psi: return "#scale[0.5]{#it{q}^{2}_{" + Tex::Psi + "} }"; break;
        case Q2Bin::Gamma: return "#scale[0.5]{#it{q}^{2}_{" + Tex::Photon + "} }"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_tex Q2Bin failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_tex Q2Bin failed", "EXIT_FAILURE");
    return "Error";
}

//======================== Years
const Year pyYear::hash_year(const TString & _string) {
    if (_string == "") return Year::All;
    if (_string == "11") return Year::Y2011;
    if (_string == "12") return Year::Y2012;
    if (_string == "R1") return Year::Run1;
    if (_string == "15") return Year::Y2015;
    if (_string == "16") return Year::Y2016;
    if (_string == "R2p1") return Year::Run2p1;
    if (_string == "17") return Year::Y2017;
    if (_string == "18") return Year::Y2018;
    if (_string == "R2p2") return Year::Run2p2;
    if (_string == "R2") return Year::Run2;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_year(\"", _string, "\") failed", "EXIT_FAILURE");
    return Year::Error;
}
TString pyYear::to_string(const Year & _enum) {
    switch (_enum) {
        case Year::All: return ""; break;
        case Year::Y2011: return "11"; break;
        case Year::Y2012: return "12"; break;
        case Year::Run1: return "R1"; break;
        case Year::Y2015: return "15"; break;
        case Year::Y2016: return "16"; break;
        case Year::Run2p1: return "R2p1"; break;
        case Year::Y2017: return "17"; break;
        case Year::Y2018: return "18"; break;
        case Year::Run2p2: return "R2p2"; break;
        case Year::Run2: return "R2"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Year failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Year failed", "EXIT_FAILURE");
    return "Error";
}
TString pyYear::to_tex(const Year & _enum) {
    auto _tex = to_string(_enum);
    if (_tex != "") return TString("#scale[0.5]{") + _tex + TString(" }");
    return "";
}
int pyYear::to_int( const Year & year){
    if(year == Year::Y2011) return 11; 
    if(year == Year::Y2012) return 12; 
    if(year == Year::Y2015) return 15; 
    if(year == Year::Y2016) return 16; 
    if(year == Year::Y2017) return 17; 
    if(year == Year::Y2018) return 18; 
    MessageSvc::Warning("to_int(year), not supported run merged periods, returning -1");
    return -1;
};
const Year pyYear::from_int( int year){
    if(year == 11) return Year::Y2011; 
    if(year == 12) return Year::Y2012; 
    if(year == 15) return Year::Y2015; 
    if(year == 16) return Year::Y2016; 
    if(year == 17) return Year::Y2017; 
    if(year == 18) return Year::Y2018; 
    MessageSvc::Warning("from_int(year), not supported run merged periods, returning ERROR");
    return Year::Error;
};

//======================== Magnet Polarities
const Polarity pyPolarity::hash_polarity(const TString & _string) {
    if (_string == "") return Polarity::All;
    if (_string == "MD") return Polarity::MD;
    if (_string == "MU") return Polarity::MU;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_polarity() failed", "EXIT_FAILURE");
    return Polarity::Error;
}
TString pyPolarity::to_string(const Polarity & _enum) {
    switch (_enum) {
        case Polarity::All: return ""; break;
        case Polarity::MD: return "MD"; break;
        case Polarity::MU: return "MU"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Polarity failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Polarity failed", "EXIT_FAILURE");
    return "Error";
}
TString pyPolarity::to_tex(const Polarity & _enum) {
    auto _tex = to_string(_enum);
    if (_tex != "") return TString("#scale[0.5]{") + _tex + TString(" }");
    return "";
}

//======================== Trigger Categories
const Trigger pyTrigger::hash_trigger(const TString & _string) {
    if (_string == "") return Trigger::All;
    if (_string == "L0I") return Trigger::L0I;
    if (_string == "L0L") return Trigger::L0L;
    if (_string == "L0H") return Trigger::L0H;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_trigger(\"", _string, "\") failed", "EXIT_FAILURE");
    return Trigger::Error;
}
TString pyTrigger::to_string(const Trigger & _enum) {
    switch (_enum) {
        case Trigger::All: return ""; break;
        case Trigger::L0I: return "L0I"; break;
        case Trigger::L0L: return "L0L"; break;
        case Trigger::L0H: return "L0H"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Trigger failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Trigger failed", "EXIT_FAILURE");
    return "Error";
}
TString pyTrigger::to_tex(const Trigger & _enum) {
    auto _tex = to_string(_enum);
    if (_tex != "") return TString("#scale[0.5]{") + _tex + TString(" }");
    return "";
}

//======================== Trigger Types
const TriggerConf pyTriggerConf::hash_triggerconf(const TString & _string) {
    if (_string == "inclusive") return TriggerConf::Inclusive;
    if (_string == "exclusive") return TriggerConf::Exclusive;
    if (_string == "exclusive2") return TriggerConf::Exclusive2;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_triggerconf(\"", _string, "\") failed", "EXIT_FAILURE");
    return TriggerConf::Error;
}
TString pyTriggerConf::to_string(const TriggerConf & _enum) {
    switch (_enum) {
        case TriggerConf::Inclusive: return "inclusive"; break;
        case TriggerConf::Exclusive: return "exclusive"; break;
        case TriggerConf::Exclusive2: return "exclusive2"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string TriggerConf failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string TriggerConf failed", "EXIT_FAILURE");
    return "Error";
}
TString pyTriggerConf::to_tex(const TriggerConf & _enum) {
    auto _tex = to_string(_enum);
    if (_tex != "") return TString("#scale[0.5]{") + _tex + TString(" }");
    return "";
}

//======================== Brem Categories
const Brem pyBrem::hash_brem(const TString & _string) {
    if (_string == "") return Brem::All;
    if (_string == "0G") return Brem::G0;
    if (_string == "1G") return Brem::G1;
    if (_string == "2G") return Brem::G2;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_brem(\"", _string, "\") failed", "EXIT_FAILURE");
    return Brem::Error;
}
TString pyBrem::to_string(const Brem & _enum) {
    switch (_enum) {
        case Brem::All: return ""; break;
        case Brem::G0: return "0G"; break;
        case Brem::G1: return "1G"; break;
        case Brem::G2: return "2G"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Brem failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Brem failed", "EXIT_FAILURE");
    return "Error";
}
TString pyBrem::to_tex(const Brem & _enum) {
    auto _tex = to_string(_enum);
    if (_tex != "") return TString("#scale[0.5]{") + _tex + TString(" }");
    return "";
}

//======================== Track Categories
const Track pyTrack::hash_track(const TString & _string) {
    if (_string == "") return Track::All;
    if (_string == "LL") return Track::LL;
    if (_string == "DD") return Track::DD;
    if (_string == "TAG") return Track::TAG;
    if (_string == "PRB") return Track::PRB;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_track(\"", _string, "\") failed", "EXIT_FAILURE");
    return Track::Error;
}
TString pyTrack::to_string(const Track & _enum) {
    switch (_enum) {
        case Track::All: return ""; break;
        case Track::LL: return "LL"; break;
        case Track::DD: return "DD"; break;
        case Track::TAG: return "TAG"; break;
        case Track::PRB: return "PRB"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string Track failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string Track failed", "EXIT_FAILURE");
    return "Error";
}
TString pyTrack::to_tex(const Track & _enum) {
    auto _tex = to_string(_enum);
    if (_tex != "") return TString("#scale[0.5]{") + _tex + TString(" }");
    return "";
}

//======================== PDF Types
const PdfType pyPdfType::hash_pdftype(const TString & _string) {
    if (_string == "") return PdfType::Empty;
    if (_string == "StringToPDF") return PdfType::StringToPDF;
    if (_string == "StringToFit") return PdfType::StringToFit;
    if (_string == "RooAbsPDF") return PdfType::RooAbsPDF;
    if (_string == "RooHistPDF") return PdfType::RooHistPDF;
    if (_string == "RooKeysPDF") return PdfType::RooKeysPDF;
    if (_string == "SignalCopy") return PdfType::SignalCopy;
    if (_string == "Template") return PdfType::Template;
    if (_string == "ToyPDF") return PdfType::ToyPDF;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_pdf(\"", _string, "\") failed", "EXIT_FAILURE");
    return PdfType::Error;
}
TString pyPdfType::to_string(const PdfType & _enum) {
    switch (_enum) {
        case PdfType::Empty: return ""; break;
        case PdfType::StringToPDF: return "StringToPDF"; break;
        case PdfType::StringToFit: return "StringToFit"; break;
        case PdfType::RooAbsPDF: return "RooAbsPDF"; break;
        case PdfType::RooHistPDF: return "RooHistPDF"; break;
        case PdfType::RooKeysPDF: return "RooKeysPDF"; break;
        case PdfType::SignalCopy: return "SignalCopy"; break;
        case PdfType::Template: return "Template"; break;
        case PdfType::ToyPDF: return "ToyPDF"; break;
        default: MessageSvc::Error("to_string PdfType failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string PdfType failed", "EXIT_FAILURE");
    return "Error";
}

//======================== Blinding Modes
const BlindMode pyBlindMode::hash_blindmode(const TString & _string) {
    if (_string == "") return BlindMode::Empty;
    if (_string == "OffsetScale") return BlindMode::OffsetScale;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_blindmode(\"", _string, "\") failed", "EXIT_FAILURE");
    return BlindMode::Error;
}
TString pyBlindMode::to_string(const BlindMode & _enum) {
    switch (_enum) {
        case BlindMode::Empty: return ""; break;
        case BlindMode::OffsetScale: return "OffsetScale"; break;
        default: MessageSvc::Error("to_string BlindMode failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string BlindMode failed", "EXIT_FAILURE");
    return "Error";
}

//======================== Opening Modes
const OpenMode pyOpenMode::hash_openmode(const TString & _string) {
    TString _check_string = _string;
    _check_string.ReplaceAll(" ", "");
    if (_check_string == "") return OpenMode::NONE;
    if (_check_string == "READ") return OpenMode::READ;
    if (_check_string == "RECREATE") return OpenMode::RECREATE;
    if (_check_string == "UPDATE") return OpenMode::UPDATE;
    if (_check_string == "WARNING") return OpenMode::WARNING;
    if (_check_string == "ERROR") return OpenMode::ERROR;
    MessageSvc::Error("EnumeratorSvc", (TString) "hash_openmode(\"", _string, "\") failed", "EXIT_FAILURE");
    return OpenMode::ERROR;
}
TString pyOpenMode::to_string(const OpenMode & _enum) {
    switch (_enum) {
        case OpenMode::NONE: return ""; break;
        case OpenMode::READ: return "READ"; break;
        case OpenMode::RECREATE: return "RECREATE"; break;
        case OpenMode::UPDATE: return "UPDATE"; break;
        case OpenMode::WARNING: return "WARNING"; break;
        case OpenMode::ERROR: return "ERROR"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string OpenMode failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string OpenMode failed", "EXIT_FAILURE");
    return "Error";
}

TString pyRatioType::to_string(const RatioType & _enum) {
    switch (_enum) {
        case RatioType::SingleRatio: return "SingleRatio"; break;
        case RatioType::DoubleRatio: return "DoubleRatio"; break;
        case RatioType::BranchingFraction: return "BranchingRatio"; break;
        case RatioType::FsOverFd: return "FsOverFd"; break;
        case RatioType::FLbOverFd: return "FLbOverFd"; break;
        case RatioType::YieldRatio: return "YieldRatio"; break;
        case RatioType::EfficiencyRatio: return "EfficiencyRatio"; break;
        case RatioType::EfficiencyRatioSystematic: return "EfficiencyRatioSystematic"; break;
        default: MessageSvc::Error("EnumeratorSvc", (TString) "to_string RatioType failed", "EXIT_FAILURE"); break;
    }
    MessageSvc::Error("EnumeratorSvc", (TString) "to_string RatioType failed", "EXIT_FAILURE");
    return "Error";
}
#endif
