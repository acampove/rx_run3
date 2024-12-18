#include "TupleCreate.hpp"
#include "ParserSvc.hpp"

/**
 * TupleCreate
 * tupleCreate.out --yaml yaml/tuples/config-TupleCreate-###.yaml
 */
int main(int argc, char ** argv) {

    auto tStart = chrono::high_resolution_clock::now();

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    SettingDef::Tuple::addTuple = true;

    EventType et = EventType();

    TupleCreate tc = TupleCreate(et, parser.YAML(), "");
    tc.Create();

    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, (TString) "Failed to load", to_string(SettingDef::Events::fails.size()), "samples");
    for (auto & _fail : SettingDef::Events::fails) { MessageSvc::Warning((TString) SettingDef::IO::exe, _fail); }
    MessageSvc::Line();

    auto tEnd = chrono::high_resolution_clock::now();
    MessageSvc::Line();
    MessageSvc::Warning((TString) SettingDef::IO::exe, "Took", to_string(chrono::duration_cast< chrono::seconds >(tEnd - tStart).count()), "seconds");
    MessageSvc::Line();

    return 0;
}
