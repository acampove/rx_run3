#ifndef MESSAGESVC_HPP
#define MESSAGESVC_HPP

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <typeinfo>

#include "TCut.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TString.h"

#include "RooAbsPdf.h"
#include "RooAbsReal.h"
#include "RooDataSet.h"
#include "RooFormulaVar.h"
#include "RooRealVar.h"
using namespace std;

#define RESET "\033[0m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"

enum class Color { Reset = 0, Black = 1, Red = 2, Green = 3, Yellow = 4, Blue = 5, Magenta = 6, Cyan = 7, White = 8 };

/**
 * \class MessageSvc
 * \brief Message service
 */
class MessageSvc {        
  public:
    /**
     * \brief Constructor
     */
    MessageSvc() = default;
    static bool SILENCED;

    static void Debug(TString _func, TString _string1 = "", TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "");
    template < typename T > static void Debug(TString _func, T * _t, TString _option = ""){
        if( !MessageSvc::SILENCED){
            cout << YELLOW;
            cout << setw(m_setw1) << left << Message("DEBUG");
            cout << setw(m_setw2) << left << _func;
            if (_t != nullptr) {
                _t->Print("");
                if (_option == "v") _t->Print("v");
            } else
                Error(_func, (TString) "Object is nullptr");
            cout << RESET;
        }
    };
    static void Error(TString _func, TString _string1 = "", TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "");
    static void Error(int _expression, TString _func, TString _string1 = "", TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "");

    static void Info(Color _COLOR, TString _func, TString _string1 = "", TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "");
    static void Info(TString _func, TString _string1 = "", TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "");
    static void Info(TString _func, bool _property);
    template < typename T > static void Info(TString _func, T * _t, TString _option = ""){
        if( !MessageSvc::SILENCED){
            cout << GREEN;
            cout << setw(m_setw1) << left << Message("INFO");
            cout << setw(m_setw2) << left << _func;
            if (_t != nullptr) {
                _t->Print("");
                if (_option == "v") _t->Print("v");
                // if (((TString) typeid(_t).name()).Contains("RooRealVar")) {
                //    if (_option == "sci") ((RooRealVar *) _t)->printScientific(1);
                //    Print(cout, " ", "RooRealVar", to_string(((RooRealVar *) _t)->getVal()), "+/-", to_string(((RooRealVar *) _t)->getError()), "( + " + to_string(((RooRealVar *) _t)->getErrorHi()) + ", - " + to_string(((RooRealVar *) _t)->getErrorLo()) + " )", ((RooRealVar *) _t)->getError() != 0 ? " (" + to_string(((RooRealVar *) _t)->getError() / ((RooRealVar *) _t)->getVal() * 100) + "%)" : "");
                //}
                // if (((TString) typeid(_t).name()).Contains("RooAbsReal")) { Print(cout, " ", "RooAbsReal", to_string(((RooAbsReal *) _t)->getVal())); }
                if (((TString) typeid(_t).name()).Contains("TH1")) {
                    Print(cout, " ", "Bins", to_string(((TH1 *) _t)->GetNbinsX()));
                    Print(cout, " ", "Cells", to_string(((TH1 *) _t)->GetNcells()));
                    Print(cout, " ", "Entries", to_string(((TH1 *) _t)->GetEntries()));
                    Print(cout, " ", "Integral", to_string(((TH1 *) _t)->Integral()));
                    Print(cout, " ", "Underflows", to_string(((TH1 *) _t)->GetBinContent(0)));
                    Print(cout, " ", "Overflows", to_string(((TH1 *) _t)->GetBinContent(((TH1 *) _t)->GetNbinsX() + 1)));
                    Print(cout, " ", "Mean", to_string(((TH1 *) _t)->GetMean()), "+/-", to_string(((TH1 *) _t)->GetMeanError()));
                    Print(cout, " ", "StdDev", to_string(((TH1 *) _t)->GetStdDev()), "+/-", to_string(((TH1 *) _t)->GetStdDevError()));
                }
                if (((TString) typeid(_t).name()).Contains("TH2")) {
                    Print(cout, " ", "Bins", to_string(((TH2 *) _t)->GetNbinsX()), "x", to_string(((TH2 *) _t)->GetNbinsY()));
                    Print(cout, " ", "Cells", to_string(((TH2 *) _t)->GetNcells()));
                    Print(cout, " ", "Entries", to_string(((TH2 *) _t)->GetEntries()));
                    Print(cout, " ", "Integral", to_string(((TH2 *) _t)->Integral()));
                    Print(cout, " ", "MeanX", to_string(((TH2 *) _t)->GetMean(1)), "+/-", to_string(((TH2 *) _t)->GetMeanError(1)));
                    Print(cout, " ", "StdDevX", to_string(((TH2 *) _t)->GetStdDev(1)), "+/-", to_string(((TH2 *) _t)->GetStdDevError(1)));
                    Print(cout, " ", "MeanY", to_string(((TH2 *) _t)->GetMean(2)), "+/-", to_string(((TH2 *) _t)->GetMeanError(2)));
                    Print(cout, " ", "StdDevY", to_string(((TH2 *) _t)->GetStdDev(2)), "+/-", to_string(((TH2 *) _t)->GetStdDevError(2)));
                }
                if (((TString) typeid(_t).name()).Contains("TH3")) {
                    Print(cout, " ", "Bins", to_string(((TH3 *) _t)->GetNbinsX()), "x", to_string(((TH3 *) _t)->GetNbinsY()), "x", to_string(((TH3 *) _t)->GetNbinsZ()));
                    Print(cout, " ", "Cells", to_string(((TH3 *) _t)->GetNcells()));
                    Print(cout, " ", "Entries", to_string(((TH3 *) _t)->GetEntries()));
                    Print(cout, " ", "Integral", to_string(((TH3 *) _t)->Integral()));
                    Print(cout, " ", "MeanX", to_string(((TH3 *) _t)->GetMean(1)), "+/-", to_string(((TH3 *) _t)->GetMeanError(1)));
                    Print(cout, " ", "StdDevX", to_string(((TH3 *) _t)->GetStdDev(1)), "+/-", to_string(((TH3 *) _t)->GetStdDevError(1)));
                    Print(cout, " ", "MeanY", to_string(((TH3 *) _t)->GetMean(2)), "+/-", to_string(((TH3 *) _t)->GetMeanError(2)));
                    Print(cout, " ", "StdDevY", to_string(((TH3 *) _t)->GetStdDev(2)), "+/-", to_string(((TH3 *) _t)->GetStdDevError(2)));
                    Print(cout, " ", "MeanZ", to_string(((TH3 *) _t)->GetMean(3)), "+/-", to_string(((TH3 *) _t)->GetMeanError(3)));
                    Print(cout, " ", "StdDevZ", to_string(((TH3 *) _t)->GetStdDev(3)), "+/-", to_string(((TH3 *) _t)->GetStdDevError(3)));
                }
            } else
                Error(_func, (TString) "Object is nullptr");
            cout << RESET;
        }
    };
    template < typename T > static void Info(TString _func, vector< T > _ts){        
        if( !MessageSvc::SILENCED){
            cout << GREEN;
            cout << setw(m_setw1) << left << Message("INFO");
            cout << setw(m_setw2) << left << _func;
            cout << "[" << _ts.size() << "] ";
            if (((TString) typeid(_ts).name()).Contains("TCut")) cout << endl;
            for (const auto & _t : _ts) {
                if (((TString) typeid(_t).name()).Contains("TCut"))
                    cout << setw(m_setw1 + m_setw2) << left << " " << _t << endl;
                else if (((TString) typeid(_t).name()).Contains("TString"))
                    cout << "\"" << _t << "\" ";
                else
                    cout << _t << " ";
            }
            if (!((TString) typeid(_ts).name()).Contains("TCut")) cout << endl;
            cout << RESET;
        }
    };

    static void Warning(TString _func, TString _string1 = "", TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "");
    template < typename T > static void Warning(TString _func, T * _t, TString _option = ""){    
        if( !MessageSvc::SILENCED){
            cout << MAGENTA;
            cout << setw(m_setw1) << left << Message("WARNING");
            cout << setw(m_setw2) << left << _func;
            if (_t != nullptr) {
                _t->Print("");
                if (_option == "v") _t->Print("v");
            } else
                Error(_func, (TString) "Object is nullptr");
            cout << RESET;
        }
    };



    static void Print(TString _string1, TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "", TString _string9 = "");
    static void Print(ostream & os, TString _string1, TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "", TString _string9 = "", TString _string10 = "");

    static void Line();
    static void Line(ostream & os);

    static TString Message(TString _message);

    static double ShowPercentage(int ientry, int nentries, time_t start = 0, int ntimes = 2000, bool dobar = true, bool doentry = true);

    static void Banner();

  private:
    static const int m_line = 170;
    static const int m_char = 1;

    static const int m_setw1 = 20;
    static const int m_setw2 = 35;

    static TString color_to_string(const Color & _enum);
};


#endif
