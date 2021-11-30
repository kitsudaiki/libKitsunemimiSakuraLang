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
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{
class BlossomItem;
class SakuraThread;
class Validator;
class SakuraLangInterface;
class ValueItemMap;

//--------------------------------------------------------------------------------------------------

struct BlossomLeaf
{
    std::string blossomType = "";
    std::string blossomName = "";
    std::string blossomPath = "";
    std::string blossomGroupType = "";
    std::vector<std::string> nameHirarchie;

    DataMap output;
    DataMap input;

    DataMap* parentValues = nullptr;

    std::string terminalOutput = "";
};

//--------------------------------------------------------------------------------------------------

struct BlossomStatus
{
    uint64_t statusCode = 0;
    std::string errorMessage = "";
};

//--------------------------------------------------------------------------------------------------

enum FieldType
{
    SAKURA_UNDEFINED_TYPE = 0,
    SAKURA_INT_TYPE = 1,
    SAKURA_FLOAT_TYPE = 2,
    SAKURA_BOOL_TYPE = 3,
    SAKURA_STRING_TYPE = 4,
    SAKURA_ARRAY_TYPE = 5,
    SAKURA_MAP_TYPE = 6
};

//--------------------------------------------------------------------------------------------------

class Blossom
{
public:
    Blossom();
    virtual ~Blossom();

    enum IO_ValueType
    {
        UNDEFINED_VALUE_TYPE = 0,
        INPUT_TYPE = 1,
        OUTPUT_TYPE = 2,
    };

    struct BlossomValidDef
    {
        const IO_ValueType ioType;
        const FieldType fieldType;
        const bool isRequired;
        const std::string comment;

        BlossomValidDef(const IO_ValueType ioType,
                        const FieldType fieldType,
                        const bool isRequired,
                        const std::string &comment)
            : ioType(ioType),
              fieldType(fieldType),
              isRequired(isRequired),
              comment(comment) { }
    };

    std::map<std::string, BlossomValidDef> validationMap;

protected:
    virtual bool runTask(BlossomLeaf &blossomLeaf,
                         const DataMap &context,
                         BlossomStatus &status,
                         ErrorContainer &error) = 0;
    bool allowUnmatched = false;

    bool registerInputField(const std::string &name,
                            const FieldType fieldType,
                            const bool required,
                            const std::string &comment);
    bool registerOutputField(const std::string &name,
                             const FieldType fieldType,
                             const std::string &comment);

private:
    friend SakuraThread;
    friend Validator;
    friend SakuraLangInterface;

    bool registerField(const std::string &name,
                       const IO_ValueType type,
                       const FieldType fieldType,
                       const bool required,
                       const std::string &comment);


    bool growBlossom(BlossomLeaf &blossomLeaf,
                     const DataMap* context,
                     BlossomStatus &status,
                     ErrorContainer &error);
    bool validateInput(const DataMap &input,
                       ErrorContainer &error);
    bool validateInput(BlossomItem &blossomItem,
                       const std::string &filePath,
                       ErrorContainer &error);
    void getCompareMap(const ValueItemMap &valueMap,
                       std::map<std::string, IO_ValueType> &compareMap);

    bool checkValues(const DataMap &values,
                    const IO_ValueType ioType,
                    ErrorContainer &error);
    bool checkType(DataItem* item,
                   const FieldType fieldType);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_BLOSSOM_H
