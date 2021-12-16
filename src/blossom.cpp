/**
 * @file        blossom.cpp
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

#include <libKitsunemimiSakuraLang/blossom.h>

#include <items/item_methods.h>
#include <libKitsunemimiCommon/logger.h>
#include <runtime_validation.h>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
Blossom::Blossom(const std::string &comment)
    : comment(comment) {}

/**
 * @brief destructor
 */
Blossom::~Blossom() {}

/**
 * @brief register input field for validation of incoming messages
 *
 * @param name name of the filed to identifiy value
 * @param fieldType type for value-validation
 * @param required false, to make field optional, true to make it required
 * @param comment additional comment to describe the content of the field
 *
 * @return false, if already name already registered, else true
 */
bool
Blossom::registerInputField(const std::string &name,
                            const FieldType fieldType,
                            const bool required,
                            const std::string &comment)
{
    return registerField(name, FieldDef::INPUT_TYPE, fieldType, required, comment);
}

/**
 * @brief register output field for validation of incoming messages
 *
 * @param name name of the filed to identifiy value
 * @param fieldType type for value-validation
 * @param comment additional comment to describe the content of the field
 *
 * @return false, if already name already registered, else true
 */
bool
Blossom::registerOutputField(const std::string &name,
                             const FieldType fieldType,
                             const std::string &comment)
{
    return registerField(name, FieldDef::OUTPUT_TYPE, fieldType, false, comment);
}

/**
 * @brief get a pointer to the validation-map
 *
 * @return pointer to validation-map
 */
const std::map<std::string, FieldDef>*
Blossom::getValidationMap() const
{
    return &validationMap;
}

/**
 * @brief register field for validation of incoming messages
 *
 * @param name name of the filed to identifiy value
 * @param ioType INPUT_TYPE or OUTPUT_TYPE
 * @param fieldType type for value-validation
 * @param required false, to make field optional, true to make it required
 * @param comment additional comment to describe the content of the field
 *
 * @return false, if already name already registered, else true
 */
bool
Blossom::registerField(const std::string &name,
                       const FieldDef::IO_ValueType ioType,
                       const FieldType fieldType,
                       const bool required,
                       const std::string &comment)
{
    std::map<std::string, FieldDef>::const_iterator defIt;
    defIt = validationMap.find(name);
    if(defIt != validationMap.end()) {
        return false;
    }

    validationMap.emplace(name, FieldDef(ioType, fieldType, required, comment));

    return true;
}

/**
 * @brief execute blossom
 *
 * @param blossomLeaf leaf-object for values-handling while processing
 * @param context const-map with global accasible values
 * @param status reference for status-output
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
Blossom::growBlossom(BlossomLeaf &blossomLeaf,
                     const DataMap* context,
                     BlossomStatus &status,
                     ErrorContainer &error)
{
    // process blossom
    LOG_DEBUG("runTask " + blossomLeaf.blossomName);

    // validate input
    if(checkValues(validationMap,
                   *blossomLeaf.input.getItemContent()->toMap(),
                   FieldDef::INPUT_TYPE,
                   error) == false)
    {
        return false;
    }

    // handle result
    if(runTask(blossomLeaf, *context, status, error) == false)
    {
        createError(blossomLeaf, "blossom execute", error);
        return false;
    }

    // validate output
    if(checkValues(validationMap,
                   *blossomLeaf.output.getItemContent()->toMap(),
                   FieldDef::OUTPUT_TYPE,
                   error) == false)
    {
        return false;
    }

    return true;
}

/**
 * @brief validate given input with the required and allowed values of the selected blossom
 *
 * @param input given input values
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
Blossom::validateFields(const DataMap &input,
                        const FieldDef::IO_ValueType valueType,
                        ErrorContainer &error)
{
    if(allowUnmatched == false)
    {
        // check if all keys in the values of the blossom-item also exist in the required-key-list
        std::map<std::string, DataItem*>::const_iterator compareIt;
        for(compareIt = input.map.begin();
            compareIt != input.map.end();
            compareIt++)
        {
            std::map<std::string, FieldDef>::const_iterator defIt;
            defIt = validationMap.find(compareIt->first);
            if(defIt == validationMap.end())
            {
                // build error-output
                error.addMeesage("Validation failed, because variable \""
                                 + compareIt->first
                                 + "\" is not in the list of allowed keys");
                return false;
            }
        }
    }

    // check that all keys in the required keys are also in the values of the blossom-item
    std::map<std::string, FieldDef>::const_iterator defIt;
    for(defIt = validationMap.begin();
        defIt != validationMap.end();
        defIt++)
    {
        if(defIt->second.isRequired == true
                && defIt->second.ioType == valueType)
        {
            // search for values
            const bool ret = input.contains(defIt->first);
            if(ret == false)
            {
                error.addMeesage("Validation failed, because variable \""
                                 + defIt->first
                                 + "\" is required, but is not set.");
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief validate given input with the required and allowed values of the selected blossom
 *
 * @param blossomItem blossom-item with given values
 * @param filePath file-path where the blossom belongs to, only used for error-output
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
Blossom::validateInput(BlossomItem &blossomItem,
                       const std::string &filePath,
                       ErrorContainer &error)
{
    std::map<std::string, FieldDef::IO_ValueType> compareMap;
    getCompareMap(compareMap, blossomItem.values);

    if(allowUnmatched == false)
    {
        // check if all keys in the values of the blossom-item also exist in the required-key-list
        std::map<std::string, FieldDef::IO_ValueType>::const_iterator compareIt;
        for(compareIt = compareMap.begin();
            compareIt != compareMap.end();
            compareIt++)
        {
            std::map<std::string, FieldDef>::const_iterator defIt;
            defIt = validationMap.find(compareIt->first);
            if(defIt == validationMap.end())
            {
                // build error-output
                error.addMeesage("variable \""
                                 + compareIt->first
                                 + "\" is not in the list of allowed keys");
                createError(blossomItem, filePath, "validator", error);
                return false;
            }
        }
    }

    // check that all keys in the required keys are also in the values of the blossom-item
    std::map<std::string, FieldDef>::const_iterator defIt;
    for(defIt = validationMap.begin();
        defIt != validationMap.end();
        defIt++)
    {
        if(defIt->second.isRequired == true)
        {
            // search for values
            std::map<std::string, FieldDef::IO_ValueType>::const_iterator compareIt;
            compareIt = compareMap.find(defIt->first);
            if(compareIt != compareMap.end())
            {
                if(defIt->second.ioType != compareIt->second)
                {
                    error.addMeesage("variable \""
                                     + defIt->first
                                     + "\" has not the correct input/output type");
                    createError(blossomItem, filePath, "validator", error);
                    return false;
                }
            }
            else
            {
                error.addMeesage("variable \""
                                 + defIt->first
                                 + "\" is required, but is not set.");
                createError(blossomItem, filePath, "validator", error);
                return false;
            }
        }
    }

    return true;
}

/**
 * @brief get map for comparism in validator
 *
 * @param compareMap reference for the resulting map
 * @param value-map to compare
 */
void
Blossom::getCompareMap(std::map<std::string, FieldDef::IO_ValueType> &compareMap,
                       const ValueItemMap &valueMap)
{
    // copy items
    std::map<std::string, ValueItem>::const_iterator it;
    for(it = valueMap.m_valueMap.begin();
        it != valueMap.m_valueMap.end();
        it++)
    {
        if(it->second.type == ValueItem::INPUT_PAIR_TYPE) {
            compareMap.emplace(it->first, FieldDef::INPUT_TYPE);
        }

        if(it->second.type == ValueItem::OUTPUT_PAIR_TYPE) {
            compareMap.emplace(it->second.item->toString(), FieldDef::OUTPUT_TYPE);
        }
    }

    // copy child-maps
    std::map<std::string, ValueItemMap*>::const_iterator itChilds;
    for(itChilds = valueMap.m_childMaps.begin();
        itChilds != valueMap.m_childMaps.end();
        itChilds++)
    {
        compareMap.emplace(itChilds->first, FieldDef::INPUT_TYPE);
    }
}

} // namespace Sakura
} // namespace Kitsunemimi
