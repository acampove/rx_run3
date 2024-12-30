#pragma once

#ifndef GLOG_USE_GLOG_EXPORT
#define GLOG_USE_GLOG_EXPORT
#endif

#include <assert.h>
#include <iomanip>
#include <iostream>
#include <glog/logging.h>
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

enum class Color 
{ 
    Reset   = 0, 
    Black   = 1, 
    Red     = 2, 
    Green   = 3, 
    Yellow  = 4, 
    Blue    = 5, 
    Magenta = 6, 
    Cyan    = 7, 
    White   = 8 };

/**
 * \class MessageSvc
 * \brief Message service
 */
class MessageSvc 
{        
  public:
    /**
     * \brief Constructor
     */
    static bool SILENCED;

    /**
     * @brief Initializes logging service with glog as backend
     * @param level This is an integer:
     *
     *   < 0  This is verbose, the value corresponds to FLAGS_v and will make it more verbose, the lower the value gets \n
     *   == 0: Info \n
     *   == 1: Warning \n
     *   == 2: Error \n
     *   == 3: Fatal, this will end the execution, after showing the message  \n
    */
    static MessageSvc& Initialize(const short &level);
    // -----------------------------------
    static void Debug(
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    static void Debug(
            TString        _func, 
            const TObject * robj,
            TString         option   = "",
            const char    * file     = __builtin_FILE(),
            const int     & line     = __builtin_LINE());
    // -----------------------------------
    static void Info(
            Color    _COLOR, 
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    static void Info(
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    /**
     * @brief Function that prints at info level a pointer to a ROOT object
     * @param _func The name of the function where this function is called
     * @params robj A pointer to an object inheriting from TObject, `robj->Print()` will be called internally
     * @params option A string that is passed to the `Print()`, method, by default empty, can be "v"
    */
    static void Info(
            TString        _func, 
            const TObject * robj,
            TString         option   = "",
            const char    * file     = __builtin_FILE(),
            const int     & line     = __builtin_LINE());

    /**
     * @brief Function template that prints logging messages at info level
     *
     * @param _func The name of the function where this function is called
     * @param _v_data a STL vector containing the data to print
     * &return void
     */
    template < typename T > 
    static void Info(TString _func, const vector< T > &_v_data)
    {        
        if ( MessageSvc::SILENCED )
            return;

        LOG(INFO)<< "At " << _func << ", vector size:" << _v_data.size(); 
        for (const auto & entry : _v_data) 
            LOG(INFO) << "    " << entry << "\n";
    };
    // -----------------------------------
    static void Warning(
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    static void Warning(
            TString        _func, 
            const TObject * robj,
            TString         option   = "",
            const char    * file     = __builtin_FILE(),
            const int     & line     = __builtin_LINE());
    // -----------------------------------
    static void Error(
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    static void Error(
            bool     _expression, 
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());
    // -----------------------------------
    static void Fatal(
            const TString  &_func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    static void Fatal(
            const bool &_expression, 
            TString  _func, 
            TString  _string1 = "", 
            TString  _string2 = "", 
            TString  _string3 = "", 
            TString  _string4 = "", 
            TString  _string5 = "", 
            TString  _string6 = "", 
            TString  _string7 = "", 
            TString  _string8 = "",
            const char *file  = __builtin_FILE(),
            const int  &line  = __builtin_LINE());

    // -----------------------------------

    static void Print(TString _string1, TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "", TString _string9 = "");
    static void Print(ostream & os, TString _string1, TString _string2 = "", TString _string3 = "", TString _string4 = "", TString _string5 = "", TString _string6 = "", TString _string7 = "", TString _string8 = "", TString _string9 = "", TString _string10 = "");
    static void Line();
    static void Line(ostream & os);
    static TString Message(TString _message);
    static double ShowPercentage(int ientry, int nentries, time_t start = 0, int ntimes = 2000, bool dobar = true, bool doentry = true);
    static void Banner();
    static void PrefixFormatter(std::ostream& stream, const google::LogMessage& message, void* /*data*/);
    static std::string ColorFromLevel(const std::string &level);

    /**
     * @brief Takes integer, positive or negative, returns log level to be used when building
     * singleton instance of logging service.
     * @param level logging level, has to be in range [0-3] or else exception is thrown
     * @return 
     * 0 -> google::INFO \n
     * 1 -> google::WARNING \n
     * 2 -> google::ERROR \n
     * 3 -> google::FATAL \n
     */
    static google::LogSeverity LevelFromInt(const short &level);

  private:
    MessageSvc(const google::LogSeverity &glevel);
    ~MessageSvc();

    MessageSvc(const MessageSvc&) = delete;
    MessageSvc& operator=(const MessageSvc&) = delete;

    static const int m_line = 170;
    static const int m_char = 1;

    static const int m_setw1 = 20;
    static const int m_setw2 = 35;

    static TString color_to_string(const Color & _enum);
};

