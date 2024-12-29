#include "MessageSvc.hpp"
#include "TSystem.h"
#include <cstdlib>

bool MessageSvc::SILENCED = false;

// -----------------------------------------------------------------------
MessageSvc::MessageSvc(const google::LogSeverity &glevel)
{
    google::InitGoogleLogging("");
    google::SetStderrLogging(glevel);
    google::InstallPrefixFormatter(&MessageSvc::PrefixFormatter);
}

MessageSvc::~MessageSvc()
{
    google::ShutdownGoogleLogging();
}

google::LogSeverity MessageSvc::LevelFromInt(const short &level)
{
    if (level < 0)
        return google::INFO;

    if (level > 3) 
        LOG(FATAL) << "Invalid logging level " << level;

    switch (level)
    {
        case 0:
            return google::INFO;
        case 1:
            return google::WARNING;
        case 2:
            return google::ERROR;
        case 3:
            return google::FATAL;
    }
}

MessageSvc& MessageSvc::Initialize(const short &level)
{
    if (level < 0)
    {
        FLAGS_v           = std::abs(level); 
        FLAGS_logbufsecs  = 0;
    }

    auto glevel = MessageSvc::LevelFromInt(level);

    static MessageSvc msg(glevel);
    return msg;
}
// -----------------------------------------------------------------------
void MessageSvc::PrefixFormatter(std::ostream& stream, const google::LogMessage& message, void* /*data*/) 
{
    std::string name = google::GetLogSeverityName(message.severity());
    auto color       = ColorFromLevel(name);

    name.insert(name.begin(), 10 - name.size(), ' ');

    stream << color << "["  << name;
}

std::string MessageSvc::ColorFromLevel(const std::string &level)
{
    if (level == "INFO")
        return "\033[1;34m";

    if (level == "WARNING")
        return "\033[1;33m";

    if (level == "ERROR")
        return "\033[1;31m";

    if (level == "FATAL")
        return "\033[1;31m";

    return "\033[0m";
}

// ---------------------------------------------------

void MessageSvc::Debug(
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 ,
        const char *file  ,
        const int  &line  ) 
{
    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    VLOG(1) << file_name << ":" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

// -----------------------------------

void MessageSvc::Debug(
            TString        _func, 
            const TObject  * robj,
            TString        _option,
            const char    * file,
            const int     & line)
{
    auto file_name  = (TString) gSystem->BaseName(file);

    VLOG(1) << file_name << ":" << _func << ":" << line << "]" << "\033[0m";
    if (robj != nullptr) 
    {
        if (_option == "v") 
            robj->Print("v");
        else
            robj->Print("");
    }
    else
        LOG(ERROR) << "Object is nullptr";
};

// --------------------------------------------
void MessageSvc::Info(
        Color    _COLOR, 
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 ,
        const char *file  ,
        const int  &line  ) 
{
    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(INFO) << file_name << ":" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

void MessageSvc::Info(
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 , 
        const char *file  ,
        const int  &line  ) 
{
    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(INFO) << file_name << ":" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};


void MessageSvc::Info(
            TString        _func, 
            const TObject * robj,
            TString        _option,
            const char    * file,
            const int     & line)
{
    auto file_name  = (TString) gSystem->BaseName(file);

    LOG(INFO) << file_name << ":" << _func << ":" << line << "]" << "\033[0m";
    if (robj != nullptr) 
    {
        if (_option == "v") 
            robj->Print("v");
        else
            robj->Print("");
    }
    else
        LOG(ERROR) << "Object is nullptr";
};
// --------------------------------------------

void MessageSvc::Warning(
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 , 
        const char *file  ,
        const int  &line  ) 
{
    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(WARNING) << file_name << ":" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

void MessageSvc::Warning(
            TString        _func, 
            const TObject * robj,
            TString        _option,
            const char    * file,
            const int     & line)
{
    auto file_name  = (TString) gSystem->BaseName(file);

    LOG(WARNING) << file_name << ":" << _func << ":" << line << "]" << "\033[0m";
    if (robj != nullptr) 
    {
        if (_option == "v") 
            robj->Print("v");
        else
            robj->Print("");
    }
    else
        LOG(ERROR) << "Object is nullptr";
};
// ---------------------------------------------------

void MessageSvc::Error(
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 , 
        const char *file  ,
        const int  &line  ) 
{
    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(ERROR) << file_name << ":" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

void MessageSvc::Error(
        bool     _expression, 
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 ,
        const char *file  ,
        const int  &line  ) 
{
    if (! _expression ) 
        return;

    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(ERROR) << file_name << ":" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

// ---------------------------------------------------

void MessageSvc::Fatal(
        const TString  &_func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 , 
        const char *file  ,
        const int  &line  ) 
{
    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(FATAL) << file_name << " f1:" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

void MessageSvc::Fatal(
        const bool &_expression, 
        TString  _func, 
        TString  _string1 , 
        TString  _string2 , 
        TString  _string3 , 
        TString  _string4 , 
        TString  _string5 , 
        TString  _string6 , 
        TString  _string7 , 
        TString  _string8 ,
        const char *file  ,
        const int  &line  ) 
{
    if (! _expression ) 
        return;

    auto file_name = (TString) gSystem->BaseName(file);
    auto message   = _string1 + _string2 + _string3 + _string4 + _string5 + _string6 + _string7 + _string8;

    LOG(FATAL) << file_name << " f2:" << _func << ":" << line << "]" << "\033[0m" << "  " << message;
};

// ---------------------------------------------------

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

