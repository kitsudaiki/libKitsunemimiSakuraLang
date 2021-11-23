#include "test_blossom.h"

#include <libKitsunemimiCommon/logger.h>
#include <interface_test.h>

namespace Kitsunemimi
{
namespace Sakura
{

TestBlossom::TestBlossom(Interface_Test* sessionTest)
    : Blossom()
{
    m_sessionTest = sessionTest;
    registerInputField("input", true);
    registerInputField("should_fail", false);
    registerOutputField("output", true);
}

bool
TestBlossom::runTask(BlossomLeaf &blossomLeaf,
                     const DataMap* context,
                     BlossomStatus &status,
                     ErrorContainer &error)
{
    LOG_DEBUG("TestBlossom");

    if(blossomLeaf.input.contains("should_fail"))
    {
        const bool shouldFail = blossomLeaf.input.get("should_fail")->toValue()->getBool();
        if(shouldFail)
        {
            status.statusCode = 1337;
            error.addMeesage("successfully failed");
            status.errorMessage = error.toString();
            return false;
        }
    }

    blossomLeaf.output.insert("output", new Kitsunemimi::DataValue(42));
    return true;
}

}
}
