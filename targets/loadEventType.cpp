#include "EventType.hpp"
#include "ParserSvc.hpp"

/**
 * Load EventType
 */
int main(int argc, char ** argv) {

    ParserSvc parser("");
    parser.Init(argc, argv);
    if (parser.Run(argc, argv) != 0) return 1;

    SettingDef::trowLogicError  = true;
    SettingDef::Tuple::addTuple = false;

    try {
        EventType et = EventType();
        et.Init();
        cout << et << endl;
        if (SettingDef::Tuple::addTuple) et.GetTupleEntries();
    } catch (const exception & e) {
        cout << e.what() << endl;
        return 1;
    }

    return 0;
}
