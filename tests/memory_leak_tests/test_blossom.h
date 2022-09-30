#ifndef TEST_BLOSSOM_H
#define TEST_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

namespace Kitsunemimi
{
namespace Sakura
{
class Interface_Test;

class TestBlossom
        : public Blossom
{
public:
    TestBlossom(Interface_Test* sessionTest);

protected:
    bool runTask(BlossomIO &blossomIO,
                 const DataMap&,
                 BlossomStatus &status,
                 ErrorContainer &error);

private:
    Interface_Test* m_sessionTest = nullptr;
};

}
}

#endif // TEST_BLOSSOM_H
