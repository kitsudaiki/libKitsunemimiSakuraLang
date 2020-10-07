#include "test_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>
#include <interface_test.h>

namespace Kitsunemimi
{
namespace Sakura
{

TestBlossom::TestBlossom(Interface_Test* sessionTest)
    : Blossom()
{
    m_sessionTest = sessionTest;
    validationMap.emplace("input", BlossomValidDef(IO_ValueType::INPUT_TYPE, true));
}

bool
TestBlossom::runTask(BlossomLeaf &blossomLeaf, std::string &)
{
    LOG_DEBUG("TestBlossom");
    DataValue* value = blossomLeaf.input.get("input")->toValue();
    m_sessionTest->compare(value->getInt(), 42);
    return true;
}

}
}
