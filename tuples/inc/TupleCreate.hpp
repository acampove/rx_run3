#ifndef TUPLECREATE_HPP
#define TUPLECREATE_HPP

#include "HelperSvc.hpp"
#include "IOSvc.hpp"
#include "MessageSvc.hpp"
#include "ParserSvc.hpp"

#include "EventType.hpp"

#include "core.h"
#include <fstream>
#include <iostream>

#include "TString.h"

#include <ROOT/RDFHelpers.hxx>
#include <ROOT/RDataFrame.hxx>
#include <ROOT/RResultPtr.hxx>

/**
 * \class TupleCreate
 * \brief Tuple info
 */
class TupleCreate {

  public:
    /**
     * \brief Default constructor
     */
    TupleCreate() = default;

    /**
     * \brief Constructor with EventType
     */
    TupleCreate(const EventType & _eventType, TString _yaml, TString _option);

    /**
     * \brief Copy constructor
     */
    TupleCreate(const TupleCreate & _tupleProcess);

    /**
     * \brief Init Tuple
     */
    void Init();

    /**
     * \brief Process Tuple
     */
    void Create();

    /**
     * \brief Get EventType
     */
    const EventType GetEventType() const { return m_eventType; };

    /**
     * \brief Get samples
     */
    const map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > Samples() const { return m_samples; };

    /**
     * \brief Get option used to process Tuple
     */
    const TString Option() const { return m_option; };

    /**
     * \brief Set option used to process Tuple
     * @param  _option [description]
     */
    void SetOption(TString _option) { m_option = _option; };

    bool IsOption(TString _option) { return m_option.Contains(_option); };

  private:
    EventType m_eventType;

    map< TString, pair< TupleHolder, vector< tuple< ConfigHolder, CutHolder, WeightHolder > > > > m_samples;

    TString m_option;

    /**
     * \brief Check allowed arguments
     * @param  _option [description]
     */
    bool Check(TString _option);

    tuple< TString, TString, TString, bool > GetSelections(tuple< ConfigHolder, CutHolder, WeightHolder > & _info, TupleHolder & _tuple);

    bool m_debug = false;
    /**
     * \brief Activate debug
     * @param  _debug [description]
     */
    void SetDebug(bool _debug) { m_debug = _debug; };
};

ostream & operator<<(ostream & os, const TupleCreate & _tupleProcess);

#endif
