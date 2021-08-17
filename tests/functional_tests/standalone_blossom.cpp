#include "standalone_blossom.h"

#include <libKitsunemimiPersistence/logger/logger.h>
#include <interface_test.h>

namespace Kitsunemimi
{
namespace Sakura
{

StandaloneBlossom::StandaloneBlossom(Interface_Test* sessionTest)
    : Blossom()
{
    m_sessionTest = sessionTest;
    registerField("input", INPUT_TYPE, true);
    registerField("output", OUTPUT_TYPE, true);
}

bool
StandaloneBlossom::runTask(BlossomLeaf &blossomLeaf, std::string &)
{
    LOG_DEBUG("StandaloneBlossom");
    DataValue* value = blossomLeaf.input.get("input")->toValue();
    m_sessionTest->compare(value->getInt(), 42);
    blossomLeaf.output.insert("output", new Kitsunemimi::DataValue(42));
    return true;
}

}
}
