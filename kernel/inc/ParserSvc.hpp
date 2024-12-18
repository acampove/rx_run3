#ifndef PARSERSVC_HPP
#define PARSERSVC_HPP

#include "EnumeratorSvc.hpp"
#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "StyleSvc.hpp"

#include "cli11.h"
#include "yamlcpp.h"

#include "TString.h"

class FitConfiguration;
class TupleHolder;
class ConfigHolder;
class WeightHolder;
class CutHolder;

/**
 * \class ParserSvc
 * \brief Parser service (https://github.com/CLIUtils/CLI11 and https://github.com/jbeder/yaml-cpp)
 */
class ParserSvc {

  public:
    /**
     * \brief Constructor with TString
     * @param  _option [description]
     */
    ParserSvc(TString _option);

    /**
     * \brief Initialize parser
     * @param  argc [description]
     * @param  argv [description]
     */
    void Init(int argc, char ** argv);

    /**
     * \brief      Initialize the system configurations from a given Yaml file (SettingDef values overloaded)
     * @param[in]  _fileYAML  The file yaml
     * @param[in]  _fileYAML  The file yaml to overload
     */
    void Init(TString _fileYAML, TString _fileYAML2OL = "");

    /**
     * \brief Run parser
     * @param  argc [description]
     * @param  argv [description]
     */
    int Run(int argc, char ** argv);

    /**
     * \brief      Return the configuration file name loaded, if multiple ones, return first one loaded
     * @return     The name of the yaml file to use
     */
    const TString YAML() const { return m_fileYAML.size() != 0 ? m_fileYAML[0] : ""; };

    EventType LoadFromYAML(TString _fileYAML);

    TString GetArgumentYAML(TString _node, TString _key);

    vector< FitConfiguration > GetConfigurationsYAML(map< pair< Prj, Q2Bin >, pair< TString, TString > > _yamls, TString _type = "");

    /**
     * \brief      Gets the list of samples to process  (static function, should be called ParserSvc::GetListOfSamples)
     * @param[in]  _yamlFile  The yaml file to parse (reuse eventually the m_fileYaml)
     * @param[in]  _q2Bin     The q2Bin to parse in the yaml file
     * @param[in]  _ana       The ana to parse in the yaml file
     * @param[in]  _Forefficiencies , flag filtering out among listed samples only the ones having a valid MCDecayTuple (which is used for the total efficiency estimation)
     * @return     a map[SampleName ] = TupleHolder - [ list of aligned Cut/weights which can bbe used afterwards]
     *
     *     Structure of the Node parsed here if you ask for q2 = jps and MM or EE analysis (reads the cutOptions to apply to data and MC and the list of samples to read)
     *     Create :
     *       cutOptionCL    : "-noMVA" #cut to apply for "LPT" samples
     *       cutOptionSigMC : "tmSig-noMVA" #cut to apply for SigMC (grabbed from configHolder.IsSignalMC())
     *       cutOptionBkgMC : "tmBkg-noMVA" #cut to apply for BkgMC
     *       we ask first to match the
     *     #####################################JPsi -q2 Fit tuples needed for RKstar
     *       jps :
     *         MM :
     *           #add sample + splitting level by triggers/ brem categories (cross product will be done for it)
     *           - { sample : "LPT"        ,  triggerAndConfs: ["L0I-exclusive" , "L0L-exclusive" , "L0L-inclusive"] , brems   : [""]                    }
     *           - { sample : "LPT"        ,  triggerAndConfs: ["L0I-exclusive" , "L0L-exclusive" , "L0L-inclusive"] , brems   : [""]  , cutOption: "..." , extraCut = "..."} //overload fully the cutOption declared before , use the ones declared HERE!
     *           - { sample : "Bu2KJPsMM"  ,  triggerAndConfs: ["L0I-exclusive" , "L0L-exclusive" , "L0L-inclusive"] , brems   : [""]                      }
     *           - { sample : "Bu2PiJPsMM" ,  triggerAndConfs: ["L0I-exclusive" , "L0L-exclusive" , "L0L-inclusive"] , brems   : [""]                      }
     *         EE :
     *           - { sample : "LPT"        ,  triggerAndConfs: ["L0I-exclusive" , "L0L-exclusive" , "L0L-inclusive"] , brems   : [""]                    }
     *           - { sample : "Bu2KJPsEE"  ,  triggerAndConfs: ["L0I-exclusive" , "L0L-exclusive" , "L0L-inclusive"] , brems   : ["" , "0G", "1G", "2G"]   }
     *           - { sample : "Bu2PiJPsEE" ,  triggerAndConfs: [" -exclusive"]                                       , brems   : [""]                    }
     *           - { sample : "Bd2KstEE"   ,  triggerAndConfs: [" -exclusive"]                                       , brems   : [""]                    }
     *           - { sample : "Bd2KstJPsEE",  triggerAndConfs: [" -exclusive"]                                       , brems   : [""]                    }
     */
    static map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > GetListOfSamples(TString _yamlFile, Q2Bin _q2Bin, Analysis _ana, bool _Forefficiencies = false);

  private:
    TString m_option;
    
    CLI::App   m_parserCLI11;
    YAML::Node m_parserYAML;

    vector< TString > m_fileYAML = {};

    int m_return;

    /**
     * \brief Initialize CLI11
     */
    void InitCLI11();

    /**
     * \brief Initialize YAML
     */
    void InitYAML();

    void ParseSettingYAML(YAML::Node _nodeYAML);
    void ParseConfigYAML(YAML::Node _nodeYAML);
    void ParseCutYAML(YAML::Node _nodeYAML);
    void ParseWeightYAML(YAML::Node _nodeYAML);
    void ParseTupleYAML(YAML::Node _nodeYAML);
    void ParseEventsYAML(YAML::Node _nodeYAML);
    void ParseEfficiencyYAML(YAML::Node _nodeYAML);
    void ParseFitYAML(YAML::Node _nodeYAML);
    void ParseToyYAML(YAML::Node _nodeYAML);

    EventType                  GetEventTypeYAML(YAML::Node _nodeYAML);
    vector< FitConfiguration > GetConfigurationsYAML(YAML::Node _nodeYAML, TString _type);

    map< pair< Prj, Q2Bin >, pair< TString, TString > > GetYamlsYAML(YAML::Node _nodeYAML, TString _type);

    /**
     * \brief Run CLI11
     * @param  argc [description]
     * @param  argv [description]
     */
    int RunCLI11(int argc, char ** argv);

    /**
     * \brief Run YAML
     */
    int RunYAML();

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };

    /**
     * \brief Warn user and throw error to remove the flag which is useless in this yaml and logic is now up-to-date  
    */
    void ObsoleteFlag( TString flag) const;


};
#endif
