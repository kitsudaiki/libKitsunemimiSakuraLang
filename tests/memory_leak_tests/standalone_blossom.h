#ifndef STANDALONE_BLOSSOM_H
#define STANDALONE_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

namespace Kitsunemimi
{
namespace Sakura
{
class Interface_Test;

class StandaloneBlossom
        : public Blossom
{
public:
    StandaloneBlossom(Interface_Test* sessionTest);

protected:
    bool runTask(BlossomLeaf &blossomLeaf,
                 const DataMap*,
                 BlossomStatus &status,
                 ErrorContainer &error);

private:
    Interface_Test* m_sessionTest = nullptr;
};

}
}

#endif // STANDALONE_BLOSSOM_H
