#include "standalone_blossom.h"

#include <libKitsunemimiCommon/logger.h>
#include <interface_test.h>

namespace Kitsunemimi
{
namespace Sakura
{

StandaloneBlossom::StandaloneBlossom(Interface_Test* sessionTest)
    : Blossom("")
{
    m_sessionTest = sessionTest;
    registerInputField("input", SAKURA_INT_TYPE, true, "comment1");
    registerInputField("should_fail", SAKURA_BOOL_TYPE, false, "comment2");

    registerInputField("default", SAKURA_STRING_TYPE, false, "default-field");
    addFieldDefault("default", new DataValue("defaultVal"));

    registerOutputField("output", SAKURA_INT_TYPE, "comment3");
}

bool
StandaloneBlossom::runTask(BlossomIO &blossomIO,
                           const DataMap&,
                           BlossomStatus &status,
                           ErrorContainer &error)
{
    LOG_DEBUG("StandaloneBlossom");

    if(blossomIO.input.contains("should_fail"))
    {
        const bool shouldFail = blossomIO.input.get("should_fail").getBool();
        if(shouldFail)
        {
            status.statusCode = 1337;
            error.addMeesage("successfully failed");
            status.errorMessage = error.toString();
            return false;
        }
    }

    blossomIO.output.insert("output", 42);
    return true;
}

}
}
