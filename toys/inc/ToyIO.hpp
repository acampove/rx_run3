#ifndef TOYIO_HPP
#define TOYIO_HPP

#include "ConfigHolder.hpp"
#include "TString.h"

// This class is intended as the interface for both C++ executable and snakemake usage.
class ToyIO {
  public:
    ToyIO() = default;

    static TString GetToyStudyDir();

    static TString GetToyFitDir(int _jobIndex);

    static TString GetToyExecuteDir(TString _options);

    static TString GetToyFitComponentDir(int _jobIndex);

    static TString GetToyFitRooFitResultDir(int _toyIndex);

    static TString GetToyFitResultDir();

    static TString GetToyFitLogDir(int _toyIndex);

    static TString GetToyFitPlotsDir(int _toyIndex);

    static TString GetToyPullDir();

    static TString GetToyUncertaintyDir();

    static TString GetFailedToyDir();

    static TString GetToyTokenDir();

    // These are static public variables to be used by snakemake

    const static TString ToyFit;

    const static TString ToyFitComponent;

    const static TString ToyFitRooFitResult;

    const static TString ToyFitResult;

    const static TString ToyFitLog;

    const static TString ToyFitPlot;

    const static TString ToyPull;

    const static TString ToyUncertainty;

    const static TString FailedToy;

    const static TString ToyToken;

    const static char * Backslash;
};

#endif
