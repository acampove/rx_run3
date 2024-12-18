#ifndef MESSAGESVC_CPP
#define MESSAGESVC_CPP

#include "MessageSvc.hpp"

bool MessageSvc::SILENCED = false;


void MessageSvc::Debug(TString _func, TString _string1 , TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 ) {
    cout << YELLOW;
    Print(cout, Message("DEBUG"), _func, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8);
    cout << RESET;
};


void MessageSvc::Error(TString _func, TString _string1 , TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 ) {
    cerr << RED;
    cerr << endl;
    Print(cerr, Message("ERROR"), _func, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8);
    cerr << RESET;
    if ((_string1 == "logic_error") || (_string2 == "logic_error") || (_string3 == "logic_error") || (_string4 == "logic_error") || (_string5 == "logic_error") || (_string6 == "logic_error")) { throw logic_error(_func); }
    if ((_string1 == "EXIT_FAILURE") || (_string2 == "EXIT_FAILURE") || (_string3 == "EXIT_FAILURE") || (_string4 == "EXIT_FAILURE") || (_string5 == "EXIT_FAILURE") || (_string6 == "EXIT_FAILURE")) exit(EXIT_FAILURE);
};
void MessageSvc::Error(int _expression, TString _func, TString _string1 , TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 ) {
    if (_expression) {
        Error(_func, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8);
        if ((_string1 == "assert") || (_string2 == "assert") || (_string3 == "assert") || (_string4 == "assert") || (_string5 == "assert") || (_string6 == "assert")) assert(_expression);
    }
};

void MessageSvc::Info(Color _COLOR, TString _func, TString _string1 , TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 ) {
    if( !MessageSvc::SILENCED){
        cout << color_to_string(_COLOR);
        Print(cout, Message("INFO"), _func, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8);
        cout << RESET;
    }
};
void MessageSvc::Info(TString _func, TString _string1 , TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 ) {
    if( !MessageSvc::SILENCED){

        cout << GREEN;
        Print(cout, Message("INFO"), _func, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8);
        cout << RESET;
    }
};
void MessageSvc::Info(TString _func, bool _property) {
    if( !MessageSvc::SILENCED){
        cout << GREEN;
        Print(cout, Message("INFO"), _func, TString(_property ? "True" : "False"));
        cout << RESET;
    }
};


void MessageSvc::Warning(TString _func, TString _string1 , TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 ) {
    cout << MAGENTA;
    Print(cout, Message("WARNING"), _func, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8);
    cout << RESET;
};

void MessageSvc::Print(TString _string1, TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 , TString _string9 ) { Print(cout, _string1, _string2, _string3, _string4, _string5, _string6, _string7, _string8, _string9); };
void MessageSvc::Print(ostream & os, TString _string1, TString _string2 , TString _string3 , TString _string4 , TString _string5 , TString _string6 , TString _string7 , TString _string8 , TString _string9 , TString _string10 ) {
    if(!MessageSvc::SILENCED){
        os << setw(m_setw1) << left << _string1;
        os << setw(m_setw2) << left << _string2;
        if (_string3 !="") os << _string3 << " ";
        if (_string4 !="") os << _string4 << " ";
        if (_string5 !="") os << _string5 << " ";
        if (_string6 !="") os << _string6 << " ";
        if (_string7 !="") os << _string7 << " ";
        if (_string8 !="") os << _string8 << " ";
        if (_string9 !="") os << _string9 << " ";
        if (_string10 !="") os << _string10 << " ";
        os << endl;
    }
};

void MessageSvc::Line() { Line(cout); };
void MessageSvc::Line(ostream & os) { 
    if( !MessageSvc::SILENCED){
        os << string(m_line, '=') << endl; 
    };
}

TString MessageSvc::Message(TString _message) { 
    return string(m_char, '=') + " " + _message + " " + string(m_char, '=') + " "; 
};

double MessageSvc::ShowPercentage(int ientry, int nentries, time_t start , int ntimes, bool dobar , bool doentry ) {
    int first_entry = ientry;

    int div = (nentries - first_entry) / ntimes;
    if (div < 1) div = 1;

    int myentry = ientry - first_entry;
    if ((((int) (myentry) % div) == 0) || (myentry == 0)) {
        // int i = 0;
        // if(i > 3) i = 0;
        // char c[] = {'|','/','-','\\'};
        // cout << "\r" << c[++i];
        double perc = (double) ientry / (nentries - 1);

        cout << GREEN;
        cout << "\r";
        cout << setw(m_setw1) << left << Message("INFO");
        cout << setw(m_setw2) << left << "ShowPercentage";
        cout << fixed << setprecision(1) << perc * 100 << "%  ";

        if (dobar) {
            cout << WHITE << "[";
            for (int p = 0; p < perc * 20; p++) cout << ">";
            for (int p = 0; p < (1 - perc) * 20; p++) cout << "_";
            cout << "]"
                    << "  ";
            cout << GREEN;
        }
        if (doentry) cout << "Entry # " << ientry + 1;
        if (start != 0) {
            time_t stop = time(nullptr);
            double diff = difftime(stop, start);
            cout << "  (";

            double t_left = ((double) nentries / ientry - 1) * diff;

            cout << "~";
            if (t_left > 60)
                cout << t_left / 60 << " min to the end)";
            else
                cout << t_left << " s to the end)";
            cout << flush;
        }
        if (ientry == (nentries - 1)) cout << RESET << endl;
        return perc;
    }
    if (ientry == (nentries - 1)) cout << RESET << endl;
    return 100;
};

void MessageSvc::Banner() {
    cout << YELLOW;
    cout << endl;
    Line();
    Line();
    cout << "||        W     W EEEE L     CCC  OOO  M   M EEEE    TTTTTT  OOO     TTTTTT H  H EEEE    RRRR  X   X    FFFF RRRR   AA  M   M EEEE W     W  OOO  RRRR  K  K      S      ||" << endl;
    cout << "||        W     W E    L    C    O   O MM MM E         TT   O   O      TT   H  H E       R   R  X X     F    R   R A  A MM MM E    W     W O   O R   R K K       B      ||" << endl;
    cout << "||        W  W  W EEE  L    C    O   O M M M EEE       TT   O   O      TT   HHHH EEE     RRRR    X      FFF  RRRR  AAAA M M M EEE  W  W  W O   O RRRR  KK               ||" << endl;
    cout << "||         W W W  E    L    C    O   O M   M E         TT   O   O      TT   H  H E       R R    X X     F    R R   A  A M   M E     W W W  O   O R R   K K       R      ||" << endl;
    cout << "||          W W   EEEE LLLL  CCC  OOO  M   M EEEE      TT    OOO       TT   H  H EEEE    R  RR X   X    F    R  RR A  A M   M EEEE   W W    OOO  R  RR K  K      Q      ||" << endl;
    Line();
    Line();
    cout << endl;
    cout << RESET;
};



TString MessageSvc::color_to_string(const Color & _enum) {
    switch (_enum) {
        case Color::Reset: return RESET; break;
        case Color::Black: return BLACK; break;
        case Color::Red: return RED; break;
        case Color::Green: return GREEN; break;
        case Color::Yellow: return YELLOW; break;
        case Color::Blue: return BLUE; break;
        case Color::Magenta: return MAGENTA; break;
        case Color::Cyan: return CYAN; break;
        case Color::White: return WHITE; break;
        default: Error("MessageSvc", (TString) "to_string Color failed", "EXIT_FAILURE"); break;
    }
    Error("MessageSvc", (TString) "to_string Color failed", "EXIT_FAILURE");
    return "";
};

#endif 