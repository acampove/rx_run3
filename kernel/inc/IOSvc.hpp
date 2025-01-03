#ifndef IOSVC_HPP
#define IOSVC_HPP

#include "EnumeratorSvc.hpp"

class ConfigHolder;
class TFile;

const TString EOS    = "LD_LIBRARY_PATH= eos";   // Workaround to solve https://cern.service-now.com/service-portal/view-incident.do?n=INC1985359
const TString XRDSVR = "root://eoslhcb.cern.ch";
const TString XRD    = "xrdfs " + XRDSVR;
const TString XRDCP  = "xrdcp --force --parallel 2 --retry 2 --streams 2";

/**
 * \class IOSvc
 * \brief IO service
 */
class IOSvc {

  public:
    /**
     * \brief Constructor
     */
    IOSvc() = default;

    /*
    Execute command line commands, and ensure waiting the command has finished to run.
    */
    static int runCommand( std::string command );
    static int runCommand( TString command );


    static TString GetDataDir(const TString &_option);

    static TString GetIODir(TString _option);

    static TString GetEfficiencyDir(const ConfigHolder & _configHolder);

    static TString GetEfficiencyDir(TString _project, TString _ana, TString _q2bin, TString _year, TString _trigger);

    static TString GetFitDir(TString _option, const ConfigHolder & _configHolder);

    /**
     * \brief
     * @param[in]  _option       The option
     * @param[in]  _prj          The project
     * @param[in]  _year         The year [only R1,R2p1,R2p2]
     * @param[in]  _trigger      The trigger [the trigger ]
     * @param[in]  _triggerConf  The trigger conf
     * @param[in]  _variable     The variable subdir
     * @param[in]  _fitName      The fit name subdir
     * @param[in]  _binIDX       The bin idx  subdir
     * @return     The flatness dir.
     */
    static TString GetFlatnessDir(const Prj & _prj, const Year & _year, const Trigger & _trigger, const TriggerConf & _triggerConf, const TString _variable = "", const TString _fitName = "", const int _binIDX = -1);

    static TString GetFitCacheDir(TString _option, const ConfigHolder & _configHolder, TString _cutHash, TString _weightHash, TString _tupleOption, TString _cacheType);

    static TString GetToyDir(TString _option, const ConfigHolder & _configHolder);

    static TString GetTupleDirHead(TString _option);

    static TString GetTupleDir(TString _option, const ConfigHolder & _configHolder);

    static TString GetTupleDir(TString _option, TString _project, TString _ana, TString _q2bin, TString _year, TString _trigger);

    static TString GetWeightDir(TString _option);

    /**
     * @brief Checks if path is a file
     * @param _filePath Path to file
     * @return True if it is an existing file, False otherwise
    */
    static bool IsFile(TString _filePath);

    /**
     * @brief Check if file exists in current filesystem
     * @param _name Path to file
     * @return True or False
    */
    static bool ExistFile(const TString &_name);

    static void CopyFile(TString _input, TString _output);

    static void RemoveFile(TString _name);

    static TFile * OpenFile(TString _name, OpenMode _mode);

    static void CloseFile(TFile * _file);

    /**
     * @brief Parses a text file with list of ROOT files
     *
     * @param _name Path to text file containing list of files
     * @param _delimiter 
     *
     * @return 
    */

    static vector< vector < TString > > ParseFile(const TString &_name, const TString &_delimiter);
    // -----------------------------------

    // Check If a given string is associated to a Directory
    static bool IsDir(TString _path);

    static bool ExistDir(TString _name);

    static void MakeDir(TString _name, OpenMode _mode = OpenMode::WARNING);

    /**
     * \brief Get a vector of TString for a matching pattern which can be path_to/\*.root for example
     * @param  _filename [description]
     */
    static vector< TString > Glob(const string & _pattern);

    /**
     * \brief      ListOfDirectoryNamesInFile
     * @param[in]  file  The input TFile
     * @param[in]  depth = depth of directories substructures (1) --> /dir1 , /dir2 , /dir3 --> vector< /dir1,/dir2, /dir3
     *                     dept = 2 ( /dir1/dir1_1, dir1/dir1_2 ...) --> vector< dir1/dir1_1 , dir1/dir1_2 ...with the / included
     * @return     vector<TString> of all "top" directories contained in the file
     */
    static vector< TString > ListOfDirectoryNamesInFile(TFile & file, int depth = 1);



    static TString XRootDFileName(TString _fileName);

  private:
    static TString                      _GetPAPTupleDir  (const ConfigHolder &_configHolder);
    static vector< TString >            _LinesFromEOSFile(const TString &_name);
    static vector< vector < TString > > _ParseEOSFile    (const TString &_name             , const TString &_delimiter);
    static vector< vector < TString > > _ParseLocalFile  (const TString &_name             , const TString &_delimeter);
    static vector< vector < TString > > _FilesFromLines  (const vector<TString> & _allLines, const TString &_delimiter);

    static const bool m_debug = false;
};

#endif
