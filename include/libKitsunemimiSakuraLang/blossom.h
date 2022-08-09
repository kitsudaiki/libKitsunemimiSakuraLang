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

#include <libKitsunemimiCommon/items/data_items.h>
#include <libKitsunemimiCommon/logger.h>
#include <libKitsunemimiSakuraLang/structs.h>

namespace Kitsunemimi
{
namespace Sakura
{
class BlossomItem;
class SakuraThread;
class InitialValidator;
class SakuraLangInterface;
class ValueItemMap;

class Blossom
{
public:
    Blossom(const std::string &comment);
    virtual ~Blossom();

    const std::string comment;

    const std::map<std::string, FieldDef>* getInputValidationMap() const;
    const std::map<std::string, FieldDef>* getOutputValidationMap() const;

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
    bool addFieldMatch(const std::string &name,
                       DataItem* match);
    bool addFieldDefault(const std::string &name,
                         DataItem* defaultValue);
    bool addFieldRegex(const std::string &name,
                       const std::string &regex);
    bool addFieldBorder(const std::string &name,
                   const long lowerBorder,
                   const long upperBorder);

private:
    friend SakuraThread;
    friend InitialValidator;
    friend SakuraLangInterface;

    std::map<std::string, FieldDef> m_inputValidationMap;
    std::map<std::string, FieldDef> m_outputValidationMap;

    bool growBlossom(BlossomLeaf &blossomLeaf,
                     const DataMap* context,
                     BlossomStatus &status,
                     ErrorContainer &error);
    bool validateFieldsCompleteness(const DataMap &input,
                                    const std::map<std::string, FieldDef> &validationMap,
                                    const FieldDef::IO_ValueType valueType,
                                    std::string &errorMessage);
    bool validateInput(BlossomItem &blossomItem,
                       const std::map<std::string, FieldDef> &validationMap,
                       const std::string &filePath,
                       ErrorContainer &error);
    void getCompareMap(std::map<std::string, FieldDef::IO_ValueType> &compareMap,
                       const ValueItemMap &valueMap);
    void fillDefaultValues(DataMap &values);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_BLOSSOM_H
