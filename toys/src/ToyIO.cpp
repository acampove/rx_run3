#ifndef TOYIO_CPP
#define TOYIO_CPP

#include "ToyIO.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "SettingDef.hpp"

const TString ToyIO::ToyFit             = "/ToyFit";
const TString ToyIO::ToyFitComponent    = "/FitComponents";
const TString ToyIO::ToyFitRooFitResult = "/RooFitResult";
const TString ToyIO::ToyFitResult       = "/ToyFitResult";
const TString ToyIO::ToyFitLog          = "/Logs";
const TString ToyIO::ToyFitPlot         = "/Plots";
const TString ToyIO::ToyPull            = "/Pulls";
const TString ToyIO::ToyUncertainty     = "/Uncertainties";
const TString ToyIO::FailedToy          = "/FailedToys";
const TString ToyIO::ToyToken           = "/.ToyToken";
const char *  ToyIO::Backslash          = "/";

TString ToyIO::GetToyExecuteDir(TString _options) { return IOSvc::GetToyDir(_options, ConfigHolder()); }

TString ToyIO::GetToyStudyDir(){
    TString toyDir = IOSvc::GetToyDir("-study", ConfigHolder());
    if (SettingDef::IO::useEOS){
        TString _localDir = toyDir.Remove(0, toyDir.Last(*ToyIO::Backslash) + 1);
        toyDir = _localDir;
    }
    return toyDir;
};

TString ToyIO::GetToyFitDir(int _jobIndex){
    TString toyDir    = GetToyStudyDir();
    TString toyFitDir = toyDir + ToyFit + "/Job" + to_string(_jobIndex);
    return toyFitDir;
}


TString ToyIO::GetToyFitComponentDir(int _jobIndex) {
    TString toyDir          = GetToyStudyDir();
    TString fitComponentDir = toyDir + ToyFitComponent + "/Job" + to_string(_jobIndex);
    return fitComponentDir;
}

TString ToyIO::GetToyFitRooFitResultDir(int _toyIndex) {
    TString toyDir          = GetToyStudyDir();
    TString RooFitResultDir = toyDir + ToyFitRooFitResult + "/Toy" + to_string(_toyIndex);
    return RooFitResultDir;
}

TString ToyIO::GetToyFitResultDir() {
    TString toyDir          = GetToyStudyDir();
    TString toyFitResultDir = toyDir + ToyFitResult;
    return toyFitResultDir;
}

TString ToyIO::GetToyFitLogDir(int _toyIndex) {
    TString toyDir    = GetToyStudyDir();
    TString toyLogDir = toyDir + ToyFitLog + "/Toy" + to_string(_toyIndex);
    return toyLogDir;
}

TString ToyIO::GetToyFitPlotsDir(int _toyIndex) {
    TString toyDir     = GetToyStudyDir();
    TString toyPlotDir = toyDir + ToyFitPlot + "/Toy" + to_string(_toyIndex);
    return toyPlotDir;
}

TString ToyIO::GetToyPullDir() {
    TString toyDir     = GetToyStudyDir();
    TString toyPullDir = toyDir + ToyPull;
    return toyPullDir;
}

TString ToyIO::GetToyUncertaintyDir() {
    TString toyDir     = GetToyStudyDir();
    TString toyPullDir = toyDir + ToyUncertainty;
    return toyPullDir;
}

TString ToyIO::GetFailedToyDir() {
    TString toyDir     = GetToyStudyDir();
    TString toyPullDir = toyDir + FailedToy;
    return toyPullDir;
}

TString ToyIO::GetToyTokenDir() {
    TString toyDir          = GetToyStudyDir();
    TString toyFitResultDir = toyDir + ToyToken;
    return toyFitResultDir;
}

#endif
