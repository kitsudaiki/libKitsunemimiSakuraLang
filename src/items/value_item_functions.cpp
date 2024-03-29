/**
 * @file        value_item_functions.cpp
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

#include "value_item_functions.h"

#include <libKitsunemimiCommon/methods/string_methods.h>
#include <libKitsunemimiCommon/items/data_items.h>

#include <libKitsunemimiJson/json_item.h>

using Kitsunemimi::splitStringByDelimiter;

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief request a value from a map- or array-item
 *
 * @param item map- or array-item
 * @param key key in map or position in array of the requested object
 * @param error reference for error-output
 *
 * @return requested value, if found, else nullptr
 */
DataItem*
getValue(DataItem* item,
         DataValue* key,
         ErrorContainer &error)
{
    // precheck
    if(item == nullptr
            || key == nullptr)
    {
        error.addMeesage("inputs for get-function are invalid");
        return nullptr;
    }

    // get value in case of item is a map
    if(item->isMap())
    {
        DataItem* resultItem = item->get(key->toString());
        if(resultItem == nullptr)
        {
            error.addMeesage("key " + key->toString() + " doesn't exist in the map");
            return nullptr;
        }

        return resultItem->copy();
    }

    // get value in case of item is an array
    if(item->isArray())
    {
        // check that value for access is an integer
        if(key->isIntValue() == false)
        {
            error.addMeesage("input for the get-function is not an integer-typed value-item");
            return nullptr;
        }
        if(key->getLong() < 0)
        {
            error.addMeesage("input for the get-function has a negative value");
            return nullptr;
        }

        const uint64_t pos = static_cast<uint64_t>(key->getLong());
        if(item->size() <= pos)
        {
            error.addMeesage("input value for get-function is too but for the array");
            return nullptr;
        }

        DataItem* resultItem = item->get(pos)->copy();
        return resultItem;
    }

    error.addMeesage("item for calling the get-function is a value-item");
    return nullptr;
}

/**
 * @brief splitValue split a value-item by a delimiter
 *
 * @param item value-item, which should be splited
 * @param delimiter delimiter as string-value to identify the positions, where to split
 * @param error reference for error-output
 *
 * @return array-item with the splitted content
 */
DataArray*
splitValue(DataValue* item,
           DataValue* delimiter,
           ErrorContainer &error)
{
    // precheck
    if(item == nullptr
            || delimiter == nullptr)
    {
        error.addMeesage("inputs for split-function are invalid");
        return nullptr;
    }

    // get and check delimiter-string
    const std::string delimiterString = delimiter->toString();
    if(delimiterString.size() == 0) {
        return nullptr;
    }

    // get first character as delimiter and handle line-break as special rule
    char demilimter = delimiterString.at(0);
    if(delimiterString == "\\n") {
        demilimter = '\n';
    }

    // split string into string-array
    std::vector<std::string> splittedContent;
    splitStringByDelimiter(splittedContent, item->toString(), demilimter);

    // convert string-array into a DataArray-object
    DataArray* resultArray = new DataArray();
    for(const std::string& object : splittedContent)
    {
        resultArray->append(new DataValue(object));
    }

    return resultArray;
}

/**
 * @brief sizeValue get the size of an item
 *
 * @param item data-item, which should be checked
 * @param error reference for error-output
 *
 * @return data-item of int-type with the size of the incoming item as value
 */
DataValue*
sizeValue(DataItem* item,
          ErrorContainer &error)
{
    // precheck
    if(item == nullptr)
    {
        error.addMeesage("inputs for size-function are invalid");
        return nullptr;
    }

    const long size = static_cast<long>(item->size());
    DataValue* resultItem = new DataValue(size);

    return resultItem;
}

/**
 * @brief check if a map or array item contains a specific value
 *
 * @param item data-item, which should be checked
 * @param key value, which should be searched in the item
 * @param error reference for error-output
 *
 * @return data-value with true, if key was found, else data-value with false
 */
DataValue*
containsValue(DataItem* item,
              DataValue* key,
              ErrorContainer &error)
{
    // precheck
    if(item == nullptr
            || key == nullptr)
    {
        error.addMeesage("inputs for contains-function are invalid");
        return nullptr;
    }

    // in case, that the item is a map
    if(item->isMap())
    {
        const bool result = item->toMap()->contains(key->toString());
        return new DataValue(result);
    }

    // in case, that the item is an array
    if(item->isArray())
    {
        // iterate over the array in interprete all as string for easier comparing
        DataArray* tempArray = item->toArray();
        for(uint32_t i = 0; i < tempArray->size(); i++)
        {
            if(tempArray->get(i)->toString() == key->toString()) {
                return new DataValue(true);
            }
        }

        return new DataValue(false);
    }

    // in case, that the item is a value
    if(item->isValue())
    {
        // interprete this value as string and check if the substring exist in it
        if (item->toString().find(key->toString()) != std::string::npos) {
            return new DataValue(true);
        }

        return new DataValue(false);
    }

    // will never be reached
    return nullptr;
}

/**
 * @brief add a new object to an existing DataArray-object
 *
 * @param item array-item, which shluld be extended
 * @param value data-item, which should be added
 * @param error reference for error-output
 *
 * @return copy of the original array-item together with the new added object
 */
DataArray*
appendValue(DataArray* item,
            DataItem* value,
            ErrorContainer &error)
{
    // precheck
    if(item == nullptr
            || value == nullptr)
    {
        error.addMeesage("inputs for append-function are invalid");
        return nullptr;
    }

    if(item->isArray() == false)
    {
        error.addMeesage("item, where the item should be added, is not an array-item");
        return nullptr;
    }

    // add oject to the map
    DataArray* result = item->copy()->toArray();
    result->append(value->copy());

    return result;
}

/**
 * @brief add a new key-value-pair to an existing DataMap-object
 *
 * @param item pointer to the map-item, where the new pair should be added
 * @param key key of the new pair
 * @param value value of the new pair
 * @param error reference for error-output
 *
 * @return copy of the original map-item together with the new added pair
 */
DataMap*
insertValue(DataMap* item,
            DataValue* key,
            DataItem* value,
            ErrorContainer &error)
{
    // precheck
    if(item == nullptr
            || key == nullptr
            || value == nullptr)
    {
        error.addMeesage("inputs for insert-function are invalid");
        return nullptr;
    }

    if(item->isMap() == false)
    {
        error.addMeesage("item, where the new key-value-pair should be added, is not a map-item");
        return nullptr;
    }

    // insert new key-value-pair
    const std::string keyString = key->toString();
    DataMap* result = item->copy()->toMap();
    result->insert(key->toString(), value->copy(), true);

    return result;
}


/**
 * @brief delete all empty entries from an array-item
 *
 * @param item array-item, which shluld be cleared
 * @param error reference for error-output
 *
 * @return copy of the original array-item together with the new added object
 */
DataArray*
clearEmpty(DataArray* item,
           ErrorContainer &error)
{
    // precheck
    if(item == nullptr)
    {
        error.addMeesage("inputs for clear-empty-function are invalid");
        return nullptr;
    }

    if(item->isArray() == false)
    {
        error.addMeesage("item, which should be cleared, is not an array-item");
        return nullptr;
    }

    // add oject to the map
    DataArray* result = item->copy()->toArray();
    for(uint32_t i = 0; i < result->size(); i++)
    {
        if(result->get(i)->toString() == "")
        {
            result->remove(i);
            i--;
        }
    }

    return result;
}

/**
 * @brief parse a json-formated string into a data-item
 *
 * @param intput input-value with the json-formated content
 * @param error reference for error-output
 *
 * @return nullptr, if failed, else a data-item with the parsed content
 */
DataItem*
parseJson(DataValue* intput,
          ErrorContainer &error)
{
    // precheck
    if(intput == nullptr)
    {
        error.addMeesage("inputs for size-function are invalid");
        return nullptr;
    }

    // try to parse json
    Kitsunemimi::Json::JsonItem jsonItem;
    if(jsonItem.parse(intput->toString(), error))
    {
        DataItem* resultItem = jsonItem.getItemContent()->copy();
        return resultItem;
    }

    return nullptr;
}

} // namespace Sakura
} // namespace Kitsunemimi
