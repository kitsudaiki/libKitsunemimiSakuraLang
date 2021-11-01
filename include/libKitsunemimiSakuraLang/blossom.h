/**
 * @file        blossom.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef KITSUNEMIMI_SAKURA_LANG_BLOSSOM_H
#define KITSUNEMIMI_SAKURA_LANG_BLOSSOM_H

#include <libKitsunemimiCommon/common_items/data_items.h>

namespace Kitsunemimi
{
namespace Sakura
{
class BlossomItem;
class SakuraThread;
class Validator;
class SakuraLangInterface;

//--------------------------------------------------------------------------------------------------
struct BlossomLeaf
{
    std::string blossomType = "";
    std::string blossomGroupType = "";
    std::vector<std::string> nameHirarchie;

    std::string blossomName = "";
    std::string blossomPath = "";

    DataMap output;
    DataMap input;

    DataMap* parentValues = nullptr;
    std::string terminalOutput = "";
};
//--------------------------------------------------------------------------------------------------
enum IO_ValueType
{
    UNDEFINED_VALUE_TYPE = 0,
    INPUT_TYPE = 1,
    OUTPUT_TYPE = 2,
};

struct BlossomValidDef
{
    IO_ValueType type = UNDEFINED_VALUE_TYPE;
    bool isRequired = false;

    BlossomValidDef(IO_ValueType type, bool isRequired)
    {
        this->type = type;
        this->isRequired = isRequired;
    }
};

//--------------------------------------------------------------------------------------------------

class Blossom
{
public:
    Blossom();
    virtual ~Blossom();

protected:
    virtual bool runTask(BlossomLeaf &blossomLeaf, uint64_t &status, std::string &errorMessage) = 0;
    bool allowUnmatched = false;

    bool registerField(const std::string &name,
                       const IO_ValueType type,
                       const bool required);
private:
    friend SakuraThread;
    friend Validator;
    friend SakuraLangInterface;

    std::map<std::string, BlossomValidDef> validationMap;

    bool growBlossom(BlossomLeaf &blossomLeaf,
                     uint64_t &status,
                     std::string &errorMessage);
    bool validateInput(DataMap &input,
                       std::string &errorMessage);
    bool validateInput(BlossomItem &blossomItem,
                       const std::string &filePath,
                       std::string &errorMessage);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_BLOSSOM_H
