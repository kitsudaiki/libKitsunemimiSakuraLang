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
    registerField("output", OUTPUT_TYPE, true);
}

bool
TestBlossom::runTask(BlossomLeaf &blossomLeaf, std::string &)
{
    LOG_DEBUG("TestBlossom");
    DataValue* value = blossomLeaf.input.get("input")->toValue();
    m_sessionTest->compare(value->getInt(), 42);
    blossomLeaf.output.insert("output", new Kitsunemimi::DataValue(42));
    return true;
}

}
}
