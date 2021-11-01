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
    registerField("input", INPUT_TYPE, true);
    registerField("should_fail", INPUT_TYPE, false);
    registerField("output", OUTPUT_TYPE, true);
}

bool
TestBlossom::runTask(BlossomLeaf &blossomLeaf, uint64_t &status, std::string &errorMessage)
{
    LOG_DEBUG("TestBlossom");
    DataValue* value = blossomLeaf.input.get("input")->toValue();
    m_sessionTest->compare(value->getInt(), 42);

    if(blossomLeaf.input.contains("should_fail"))
    {
        const bool shouldFail = blossomLeaf.input.get("should_fail")->toValue()->getBool();
        if(shouldFail)
        {
            status = 1337;
            errorMessage = "successfully failed";
            return false;
        }
    }

    blossomLeaf.output.insert("output", new Kitsunemimi::DataValue(42));
    return true;
}

}
}
