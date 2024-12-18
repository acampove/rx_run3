#ifndef TOYREADER_HPP
#define TOYREADER_HPP

#include "FitConfiguration.hpp"
#include "ToyTupleReader.hpp"

#include <map>

/**
 * \class ToyReader
 * \brief Responsible for reading toys during the toy loops.
 * \brief Owns many ToyTupleReader. Keys the ToyTupleReader using q2Bin and ConfigHolder.GetKey().
 * \brief This reflects the bookkeeping used by FitterTool.
 */
class ToyReader {
  public:
    /**
     * \brief Default constructor.
     *
     */
    ToyReader(){};
    /**
     * \brief Destructor.
     */
    ~ToyReader(){};

    /**
     * \brief Grabs all the YAML config files in SettingDef::Toy::YieldConfigYamls.
     * \brief Construct one ToyTupleReader for each YAML config file.p
     */
    void GetToyTupleReaders();

    void GetToyTupleReaders(vector< FitConfiguration > _configurations);

    /**
     * \brief Returns the reference to a ToyTupleReader keyed using q2Bin and ConfigHolder.GetKey().
     */
    ToyTupleReader & GetToyTupleReader(TString _managerKey, TString _configHolderKey);
    /**
     * \brief Returns a vector of all the q2Bin keys of the ToyTupleReader members.
     */
    vector< TString > IterateManagerKeys() const;
    /**
     * \brief Returns a vector of all the ConfigHolder.GetKey() within a q2Bin.
     */
    vector< TString > IterateKeysInManager(TString _managerKey) const;

  private:
    void AddToManagerKeyIterator(TString _managerKey);
    void AddToReaderKeyIterator(TString _managerKey, TString _configHolderKey);
    map< ConfigHolder, map<TString, double> > ParseConfigurationOverride( TString _overrideFiles );

  private:
    map< TString, map< TString, ToyTupleReader > > m_keysToToyTupleReaderMap;
    vector< TString >                              m_managerKeys;
    map< TString, vector< TString > >              m_managerToReaderKeyMap;
};

#endif