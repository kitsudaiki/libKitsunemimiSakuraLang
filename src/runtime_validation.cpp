/**
 * @file        runtime_validation.cpp
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

#include "runtime_validation.h"

#include <items/item_methods.h>
#include <sakura_garden.h>

#include <libKitsunemimiSakuraLang/blossom.h>

namespace Kitsunemimi
{
namespace Sakura
{

const std::string
createErrorMessage(const std::string &name,
                   const FieldType fieldType)
{
    std::string err = "Value-validation failed, because a value has the false type: "
                       "'" + name + "'.";
    switch(fieldType)
    {
        case SAKURA_UNDEFINED_TYPE:
            break;
        case SAKURA_INT_TYPE:
            err.append("\nExprect int-type.");
            break;
        case SAKURA_FLOAT_TYPE:
            err.append("\nExprect float-type.");
            break;
        case SAKURA_BOOL_TYPE:
            err.append("\nExprect bool-type.");
            break;
        case SAKURA_STRING_TYPE:
            err.append("\nExprect string-type.");
            break;
        case SAKURA_ARRAY_TYPE:
            err.append("\nExprect array-type.");
            break;
        case SAKURA_MAP_TYPE:
            err.append("\nExprect map-type.");
            break;
    }

    return err;
}

/**
 * @brief check valure-types of the blossom-input and -output
 *
 * @param values map of the input or output-values to validate
 * @param ioType select input- or output-values
 * @param error reference for error-output
 *
 * @return true, if everything match, else false
 */
bool
checkBlossomValues(const std::map<std::string, FieldDef> &defs,
                   const DataMap &values,
                   const FieldDef::IO_ValueType ioType,
                   ErrorContainer &error)
{
    std::map<std::string, FieldDef>::const_iterator defIt;
    for(defIt = defs.begin();
        defIt != defs.end();
        defIt++)
    {
        if(defIt->second.ioType != ioType) {
            continue;
        }

        DataItem* item = values.get(defIt->first);

        if(item != nullptr)
        {
            // check type
            if(checkType(item, defIt->second.fieldType) == false)
            {
                error.addMeesage(createErrorMessage(defIt->first, defIt->second.fieldType));
                return false;
            }

            // check match
            if(defIt->second.match != nullptr)
            {
                if(defIt->second.match->toString() != item->toString())
                {
                    std::string errorMessage = "Value '"
                                               + defIt->first
                                               + "' doesn't match the the expected value:\n   ";
                    errorMessage.append(defIt->second.match->toString());
                    error.addMeesage(errorMessage);
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * @brief check valure-types of the blossom-input and -output
 *
 * @param values map of the input or output-values to validate
 * @param ioType select input- or output-values
 * @param error reference for error-output
 *
 * @return true, if everything match, else false
 */
bool
checkTreeValues(const ValueItemMap &defs,
                const DataMap &values,
                const ValueItem::ValueType ioType,
                ErrorContainer &error)
{
    std::map<std::string, ValueItem> m_valueMap;

    std::map<std::string, ValueItem>::const_iterator defIt;
    for(defIt = defs.m_valueMap.begin();
        defIt != defs.m_valueMap.end();
        defIt++)
    {
        if(defIt->second.type == ioType)
        {
            DataItem* item = values.get(defIt->first);
            if(item != nullptr
                    && checkType(item, defIt->second.fieldType) == false)
            {
                error.addMeesage(createErrorMessage(defIt->first, defIt->second.fieldType));
                return false;
            }
        }
    }

    return true;
}


/**
 * @brief Check type of an item with the registered field
 *
 * @param item item to check
 * @param fieldType field-type to compare
 *
 * @return true, if match, else false
 */
bool
checkType(DataItem* item,
          const FieldType fieldType)
{
    if(item->getType() == DataItem::ARRAY_TYPE
            && fieldType == SAKURA_ARRAY_TYPE)
    {
        return true;
    }

    if(item->getType() == DataItem::MAP_TYPE
            && fieldType == SAKURA_MAP_TYPE)
    {
        return true;
    }

    if(item->getType() == DataItem::VALUE_TYPE)
    {
        DataValue* value = item->toValue();
        if(value->getValueType() == DataItem::INT_TYPE
                && fieldType == SAKURA_INT_TYPE)
        {
            return true;
        }
        if(value->getValueType() == DataItem::FLOAT_TYPE
                && fieldType == SAKURA_FLOAT_TYPE)
        {
            return true;
        }
        if(value->getValueType() == DataItem::BOOL_TYPE
                && fieldType == SAKURA_BOOL_TYPE)
        {
            return true;
        }
        if(value->getValueType() == DataItem::STRING_TYPE
                && fieldType == SAKURA_STRING_TYPE)
        {
            return true;
        }
    }

    return false;
}

} // namespace Sakura
} // namespace Kitsunemimi
