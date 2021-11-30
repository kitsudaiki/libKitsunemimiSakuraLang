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
    registerInputField("input", SAKURA_INT_TYPE, true, "comment1");
    registerInputField("should_fail", SAKURA_BOOL_TYPE, false, "comment2");

    registerOutputField("output", SAKURA_INT_TYPE, "comment3");
}

bool
TestBlossom::runTask(BlossomLeaf &blossomLeaf,
                     const DataMap &context,
                     BlossomStatus &status,
                     ErrorContainer &error)
{
    LOG_DEBUG("TestBlossom");
    DataValue* value = blossomLeaf.input.get("input")->toValue();
    m_sessionTest->compare(value->getInt(), 42);
    m_sessionTest->compare(context.get("test-key")->getString(), std::string("asdf"));

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
