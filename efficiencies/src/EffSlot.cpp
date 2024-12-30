#include "EffSlot.h"

#include "TString.h"
#include <vector>

void EffSlot::Print() const 
{
    MessageSvc::Info("EffSlot");
    cout << "\t wOpt       :    " << wOpt()        << endl;
    cout << "\t cOpt       :    " << cOpt()        << endl;
    cout << "\t wOptMCDecay:    " << wOptMCDecay() << endl;
    cout << "\t wOptNormN  :    " << wOptNormN()   << endl;
    cout << "\t wOptNormD  :    " << wOptNormD()   << endl;
    cout << "\t cutSetNorm :    " << cutSetNorm()  << endl;
    cout << "\t weightConfs:   [";

    int i = 0;
    for (auto & cnorm : weightConfigs()) 
    {
        if (i == 0 && weightConfigs().size() !=1)
            cout << " " << cnorm << "," << flush;
        else if (i == 0 && weightConfigs().size() ==1 )
            cout << " " << cnorm << " ]" << flush;
        else if (i != m_weightConfigurations.size() - 1)
            cout << cnorm << " , " << flush;
        else
            cout << cnorm << " ] " << endl;

        i++;
    }
    cout << endl;
}

std::vector< TString > EffSlot::ListAndCutsNorm() const 
{
    std::vector< TString > _listCuts;

    TString _raw = m_cutSetNorm;
    if (_raw.Contains("&")) 
        _listCuts = TokenizeString(_raw.ReplaceAll(" ", ""), "&");
    else 
        _listCuts = {_raw.ReplaceAll(" ", "")};

    return _listCuts;
}

