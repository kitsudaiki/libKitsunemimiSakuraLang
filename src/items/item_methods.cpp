/**
 * @file        item_methods.cpp
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

#include "item_methods.h"

#include <items/value_item_functions.h>
#include <libKitsunemimiSakuraLang/blossom.h>

#include <libKitsunemimiJinja2/jinja2_converter.h>
#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/common_items/table_item.h>

namespace Kitsunemimi
{
namespace Sakura
{

using Kitsunemimi::Jinja2::Jinja2Converter;

/**
 * @brief process a value-item by handling its function-calls
 *
 * @param valueItem value-item, which should be processed
 * @param insertValues data-map with information to fill into the jinja2-string
 * @param errorMessage error-message for output
 *
 * @return false, if something went wrong while processing and filling, else true. If false
 *         an error-message was sent directly into the sakura-root-object
*/
bool
getProcessedItem(ValueItem &valueItem,
                 DataMap &insertValues,
                 std::string &errorMessage)
{
    for(const Kitsunemimi::Sakura::FunctionItem& functionItem : valueItem.functions)
    {
        if(valueItem.item == nullptr) {
            return false;
        }
        DataItem* tempItem = nullptr;
        const std::string type = functionItem.type;

        //------------------------------------------------------------------------------------------
        if(type == "get")
        {
            if(functionItem.arguments.size() != 1)
            {
                errorMessage = type + "-function requires 1 argument";
                return false;
            }

            ValueItem arg = functionItem.arguments.at(0);
            if(fillValueItem(arg, insertValues, errorMessage) == false) {
                return false;
            }

            tempItem = getValue(valueItem.item,
                                arg.item->toValue(),
                                errorMessage);
        }
        //------------------------------------------------------------------------------------------
        if(type == "split")
        {
            if(functionItem.arguments.size() != 1)
            {
                errorMessage = type + "-function requires 1 argument";
                return false;
            }

            ValueItem arg = functionItem.arguments.at(0);
            if(fillValueItem(arg, insertValues, errorMessage) == false) {
                return false;
            }

            tempItem = splitValue(valueItem.item->toValue(),
                                  arg.item->toValue(),
                                  errorMessage);
        }
        //------------------------------------------------------------------------------------------
        if(type == "contains")
        {
            if(functionItem.arguments.size() != 1)
            {
                errorMessage = type + "-function requires 1 argument";
                return false;
            }

            ValueItem arg = functionItem.arguments.at(0);
            if(fillValueItem(arg, insertValues, errorMessage) == false) {
                return false;
            }

            tempItem = containsValue(valueItem.item,
                                     arg.item->toValue(),
                                     errorMessage);
        }
        //------------------------------------------------------------------------------------------
        if(type == "size")
        {
            if(functionItem.arguments.size() != 0)
            {
                errorMessage = type + "-function requires 0 arguments";
                return false;
            }

            tempItem = sizeValue(valueItem.item, errorMessage);
        }
        //------------------------------------------------------------------------------------------
        if(type == "insert")
        {
            if(functionItem.arguments.size() != 2)
            {
                errorMessage = type + "-function requires 2 arguments";
                return false;
            }

            ValueItem arg1 = functionItem.arguments.at(0);
            ValueItem arg2 = functionItem.arguments.at(1);

            if(fillValueItem(arg1, insertValues, errorMessage) == false
                    || fillValueItem(arg2, insertValues, errorMessage) == false)
            {
                return false;
            }

            tempItem = insertValue(valueItem.item->toMap(),
                                   arg1.item->toValue(),
                                   arg2.item,
                                   errorMessage);
        }
        //------------------------------------------------------------------------------------------
        if(type == "append")
        {
            if(functionItem.arguments.size() != 1)
            {
                errorMessage = type + "-function requires 1 argument";
                return false;
            }

            ValueItem arg = functionItem.arguments.at(0);
            if(fillValueItem(arg, insertValues, errorMessage) == false) {
                return false;
            }

            tempItem = appendValue(valueItem.item->toArray(),
                                   arg.item,
                                   errorMessage);
        }
        //------------------------------------------------------------------------------------------
        if(type == "clear_empty")
        {
            if(functionItem.arguments.size() != 0)
            {
                errorMessage = type + "-function requires 0 arguments";
                return false;
            }

            tempItem = clearEmpty(valueItem.item->toArray(),
                                  errorMessage);

        }
        //------------------------------------------------------------------------------------------
        if(type == "parse_json")
        {
            if(functionItem.arguments.size() != 0)
            {
                errorMessage = type + "-function requires 0 arguments";
                return false;
            }

            tempItem = parseJson(valueItem.item->toValue(),
                                 errorMessage);

        }
        //------------------------------------------------------------------------------------------

        delete valueItem.item;
        valueItem.item = tempItem;

        if(tempItem == nullptr) {
            return false;
        }
    }

    return true;
}

/**
 * @brief fill and process an identifier value by filling with incoming information and
 *        processing it functions-calls
 *
 * @param valueItem value-item, which should be processed
 * @param insertValues data-map with information to fill into the jinja2-string
 * @param errorMessage error-message for output
 *
 * @return false, if something went wrong while processing and filling, else true. If false
 *         an error-message was sent directly into the sakura-root-object
 */
bool
fillIdentifierItem(ValueItem &valueItem,
                   DataMap &insertValues,
                   std::string &errorMessage)
{
    // replace identifier with value from the insert-values
    DataItem* tempItem = insertValues.get(valueItem.item->toString());
    if(tempItem == nullptr)
    {
        // TODO: error-message to sakura-root
        return false;
    }

    delete valueItem.item;
    valueItem.item = tempItem->copy();
    valueItem.isIdentifier = false;
    valueItem.functions = valueItem.functions;

    return getProcessedItem(valueItem, insertValues, errorMessage);
}

/**
 * @brief interprete a string as jinja2-string, parse it and fill it with incoming information
 *
 * @param original value-item wiht string-content, which shuold be handled as jinja2-string
 * @param insertValues data-map with information to fill into the jinja2-string
 * @param errorMessage error-message for output
 *
 * @return false, if something went wrong while processing and filling, else true. If false
 *         an error-message was sent directly into the sakura-root-object
 */
bool
fillJinja2Template(ValueItem &valueItem,
                   DataMap &insertValues,
                   std::string &errorMessage)
{
    // convert jinja2-string
    Jinja2Converter* converter = Jinja2Converter::getInstance();
    std::string convertResult = "";
    bool ret = converter->convert(convertResult,
                                  valueItem.item->toString(),
                                  &insertValues,
                                  errorMessage);

    if(ret == false)
    {
        errorMessage = createError("jinja2-converter", errorMessage);
        return false;
    }

    delete valueItem.item;
    valueItem.item = new DataValue(convertResult);

    return true;
}

/**
 * @brief fill a single value-item with the information of the values of in incoming
 *        data-map, which processing all functions within the value-item-map
 *
 * @param valueItem value-item, which should be processed and filled
 * @param insertValues data-map with the values to fill the value-item
 * @param errorMessage error-message for output
 *
 * @return false, if something went wrong while processing and filling, else true. If false
 *         an error-message was sent directly into the sakura-root-object
 */
bool
fillValueItem(ValueItem &valueItem,
              DataMap &insertValues,
              std::string &errorMessage)
{
    // process and fill incoming string, which is interpreted as jinja2-template
    if(valueItem.isIdentifier == false
            && valueItem.type != ValueItem::OUTPUT_PAIR_TYPE
            && valueItem.item->isStringValue())
    {
        return fillJinja2Template(valueItem, insertValues, errorMessage);
    }
    // process and fill incoming identifier
    else if(valueItem.isIdentifier
            && valueItem.type != ValueItem::OUTPUT_PAIR_TYPE)
    {
        return fillIdentifierItem(valueItem, insertValues, errorMessage);
    }
    else if(valueItem.type != ValueItem::OUTPUT_PAIR_TYPE)
    {
        return getProcessedItem(valueItem, insertValues, errorMessage);
    }

    // if value is an int-value, output-value or something else, then do nothing with the value
    return true;
}

/**
 * @brief fill the entries of a value-item-map with the information of the values of in incoming
 *        data-map, which processing all functions within the value-item-map
 *
 * @param items value-item-map, which should be processed and filled
 * @param insertValues data-map with the values to fill the value-item-map
 * @param errorMessage error-message for output
 *
 * @return false, if something went wrong while processing and filling, else true. If false
 *         an error-message was sent directly into the sakura-root-object
 */
bool
fillInputValueItemMap(ValueItemMap &items,
                      DataMap &insertValues,
                      std::string &errorMessage)
{
    // fill values
    std::map<std::string, ValueItem>::iterator it;
    for(it = items.m_valueMap.begin();
        it != items.m_valueMap.end();
        it++)
    {
        if(fillValueItem(it->second, insertValues, errorMessage) == false) {
            return false;
        }
    }

    // fill childs
    std::map<std::string, ValueItemMap*>::iterator itChild;
    for(itChild = items.m_childMaps.begin();
        itChild != items.m_childMaps.end();
        itChild++)
    {
        const bool ret = fillInputValueItemMap(*itChild->second, insertValues, errorMessage);
        if(ret == false) {
            return false;
        }
    }

    return true;
}

/**
 * @brief wirte the output back into a value-item-map
 *
 * @param items value-item-map, where the output should be inserted
 * @param output output of the blossom-item as data-item
 *
 * @return true, if the output was written in at least one point of the value-item-map
 */
bool
fillOutputValueItemMap(ValueItemMap &items,
                       DataMap &output)
{
    std::map<std::string, ValueItem>::iterator it;
    for(it = items.m_valueMap.begin();
        it != items.m_valueMap.end();
        it++)
    {
        // replace only as output-marked values
        if(it->second.type == ValueItem::OUTPUT_PAIR_TYPE)
        {
            DataItem* tempItem = output.get(it->second.item->toString());
            if(tempItem == nullptr)
            {
                // TODO: error-output
                return false;
            }

            ValueItem valueItem;
            valueItem.item = tempItem->copy();
            valueItem.type = ValueItem::OUTPUT_PAIR_TYPE;
            it->second = valueItem;
        }
    }

    return true;
}

/**
 * @brief override data of a data-map with new incoming information
 *
 * @param original data-map with the original key-values, which should be updates with the
 *                 information of the override-map
 * @param override map with the new incoming information
 * @param type type of override
 */
void
overrideItems(DataMap &original,
              const DataMap &override,
              OverrideType type)
{
    if(type == ONLY_EXISTING)
    {
        std::map<std::string, DataItem*>::const_iterator overrideIt;
        for(overrideIt = override.map.begin();
            overrideIt != override.map.end();
            overrideIt++)
        {
            std::map<std::string, DataItem*>::iterator originalIt;
            originalIt = original.map.find(overrideIt->first);

            if(originalIt != original.map.end()) {
                original.insert(overrideIt->first, overrideIt->second->copy(), true);
            }
        }
    }
    if(type == ONLY_NON_EXISTING)
    {
        std::map<std::string, DataItem*>::const_iterator overrideIt;
        for(overrideIt = override.map.begin();
            overrideIt != override.map.end();
            overrideIt++)
        {
            std::map<std::string, DataItem*>::iterator originalIt;
            originalIt = original.map.find(overrideIt->first);

            if(originalIt == original.map.end()) {
                original.insert(overrideIt->first, overrideIt->second->copy(), true);
            }
        }
    }
    else if(type == ALL)
    {
        std::map<std::string, DataItem*>::const_iterator overrideIt;
        for(overrideIt = override.map.begin();
            overrideIt != override.map.end();
            overrideIt++)
        {
            original.insert(overrideIt->first, overrideIt->second->copy(), true);
        }
    }
}

/**
 * @brief override data of a data-map with new incoming information
 *
 * @param original data-map with the original key-values, which should be updates with the
 *                 information of the override-map
 * @param override map with the new incoming information
 * @param type type of override
 */
void
overrideItems(DataMap &original,
              const ValueItemMap &override,
              OverrideType type)
{
    if(type == ONLY_EXISTING)
    {
        std::map<std::string, ValueItem>::const_iterator overrideIt;
        for(overrideIt = override.m_valueMap.begin();
            overrideIt != override.m_valueMap.end();
            overrideIt++)
        {
            std::map<std::string, DataItem*>::iterator originalIt;
            originalIt = original.map.find(overrideIt->first);

            if(originalIt != original.map.end()) {
                original.insert(overrideIt->first, overrideIt->second.item->copy(), true);
            }
        }
    }
    else if(type == ONLY_NON_EXISTING)
    {
        std::map<std::string, ValueItem>::const_iterator overrideIt;
        for(overrideIt = override.m_valueMap.begin();
            overrideIt != override.m_valueMap.end();
            overrideIt++)
        {
            std::map<std::string, DataItem*>::iterator originalIt;
            originalIt = original.map.find(overrideIt->first);

            if(originalIt == original.map.end()) {
                original.insert(overrideIt->first, overrideIt->second.item->copy(), true);
            }
        }
    }
    else if(type == ALL)
    {
        std::map<std::string, ValueItem>::const_iterator overrideIt;
        for(overrideIt = override.m_valueMap.begin();
            overrideIt != override.m_valueMap.end();
            overrideIt++)
        {
            original.insert(overrideIt->first, overrideIt->second.item->copy(), true);
        }
    }
}

/**
 * @brief override data of a value-item-map with new incoming information
 *
 * @param original map with the original key-values, which should be updates with the
 *                 information of the override-map
 * @param override map with the new incoming information
 * @param type type of override
 */
void
overrideItems(ValueItemMap &original,
              const ValueItemMap &override,
              OverrideType type)
{
    if(type == ONLY_EXISTING)
    {
        std::map<std::string, ValueItem>::const_iterator overrideIt;
        for(overrideIt = override.m_valueMap.begin();
            overrideIt != override.m_valueMap.end();
            overrideIt++)
        {
            std::map<std::string, ValueItem>::iterator originalIt;
            originalIt = original.m_valueMap.find(overrideIt->first);

            if(originalIt != original.m_valueMap.end())
            {
                ValueItem temp = overrideIt->second;
                original.insert(overrideIt->first, temp, true);
            }
        }
    }
    else if(type == ONLY_NON_EXISTING)
    {
        std::map<std::string, ValueItem>::const_iterator overrideIt;
        for(overrideIt = override.m_valueMap.begin();
            overrideIt != override.m_valueMap.end();
            overrideIt++)
        {
            std::map<std::string, ValueItem>::iterator originalIt;
            originalIt = original.m_valueMap.find(overrideIt->first);

            if(originalIt == original.m_valueMap.end())
            {
                ValueItem temp = overrideIt->second;
                original.insert(overrideIt->first, temp, true);
            }
        }
    }
    else if(type == ALL)
    {
        std::map<std::string, ValueItem>::const_iterator overrideIt;
        for(overrideIt = override.m_valueMap.begin();
            overrideIt != override.m_valueMap.end();
            overrideIt++)
        {
            ValueItem temp = overrideIt->second;
            original.insert(overrideIt->first, temp, true);
        }
    }
}

/**
 * @brief check if given values match with existing one of a value-item-map
 *
 * @param original original, which should contain the values
 * @param itemInputValues map which is checked agains the original
 *
 * @return list of key, which doesn't match
 */
const std::vector<std::string>
checkInput(ValueItemMap &original,
           const DataMap &itemInputValues)
{
    std::vector<std::string> result;

    std::map<std::string, DataItem*>::const_iterator it;
    for(it = itemInputValues.map.begin();
        it != itemInputValues.map.end();
        it++)
    {
        if(original.contains(it->first) == false) {
            result.push_back(it->first);
        }
    }

    return result;
}

/**
 * @brief check value-item-map for uninitialized values
 *
 * @param items value-map to check
 *
 * @return list of not initialized values
 */
const std::vector<std::string>
checkItems(DataMap &items)
{
    std::vector<std::string> result;

    std::map<std::string, DataItem*>::const_iterator it;
    for(it = items.map.begin();
        it != items.map.end();
        it++)
    {
        if(it->second->getString() == "{{}}") {
            result.push_back(it->first);
        }
    }

    return result;
}

/**
 * @brief print output of a blossom-item
 *
 * @param blossom item with the information to print
 *
 * @return output as string
 */
const std::string
convertBlossomOutput(const BlossomLeaf &blossom)
{
    std::string output = "";

    // print call-hierarchy
    for(uint32_t i = 0; i < blossom.nameHirarchie.size(); i++)
    {
        for(uint32_t j = 0; j < i; j++)
        {
            output += "   ";
        }
        output += blossom.nameHirarchie.at(i) + "\n";
    }

    // print error-output
    if(blossom.terminalOutput.size() > 0)
    {
        output += "\n";
        output += blossom.terminalOutput + "\n";
    }

    return output;
}

/**
 * @brief convert value-item-map into data-map
 *
 * @param result resulting data-map
 * @param input input value-item-map
 */
void
convertValueMap(DataMap &result, const ValueItemMap &input)
{
    // fill values
    std::map<std::string, ValueItem>::const_iterator it;
    for(it = input.m_valueMap.begin();
        it != input.m_valueMap.end();
        it++)
    {
        result.insert(it->first, it->second.item->copy());
    }

    // fill childs
    std::map<std::string, ValueItemMap*>::const_iterator itChild;
    for(itChild = input.m_childMaps.begin();
        itChild != input.m_childMaps.end();
        itChild++)
    {
        DataMap* internalMap = new DataMap();
        convertValueMap(*internalMap, *itChild->second);
        result.insert(itChild->first, internalMap);
    }
}

/**
 * @brief create an error-output
 *
 * @param blossomItem blossom-item with information of the error-location
 * @param blossomPath file-path, which contains the blossom
 * @param errorLocation location where the error appeared
 * @param errorMessage message to describe, what was wrong
 * @param possibleSolution message with a possible solution to solve the problem
 */
const std::string
createError(const BlossomItem &blossomItem,
            const std::string &blossomPath,
            const std::string &errorLocation,
            const std::string &errorMessage,
            const std::string &possibleSolution)
{
    return createError(errorLocation,
                       errorMessage,
                       possibleSolution,
                       blossomItem.blossomType,
                       blossomItem.blossomGroupType,
                       blossomItem.blossomName,
                       blossomPath);
}

/**
 * @brief create an error-output
 *
 * @param blossomLeaf blossom-item with information of the error-location
 * @param errorLocation location where the error appeared
 * @param errorMessage message to describe, what was wrong
 * @param possibleSolution message with a possible solution to solve the problem
 */
const std::string
createError(const BlossomLeaf &blossomLeaf,
            const std::string &errorLocation,
            const std::string &errorMessage,
            const std::string &possibleSolution)
{
    return createError(errorLocation,
                       errorMessage,
                       possibleSolution,
                       blossomLeaf.blossomType,
                       blossomLeaf.blossomGroupType,
                       blossomLeaf.blossomName,
                       blossomLeaf.blossomPath);
}

/**
 * @brief create an error-output
 *
 * @param errorLocation location where the error appeared
 * @param errorMessage message to describe, what was wrong
 * @param possibleSolution message with a possible solution to solve the problem
 * @param blossomType type of the blossom, where the error appeared
 * @param blossomGroup type of the blossom-group, where the error appeared
 * @param blossomName name of the blossom in the script to specify the location
 * @param blossomFilePath file-path, where the error had appeared
 */
const std::string
createError(const std::string &errorLocation,
            const std::string &errorMessage,
            const std::string &possibleSolution,
            const std::string &blossomType,
            const std::string &blossomGroupType,
            const std::string &blossomName,
            const std::string &blossomFilePath)
{
    Kitsunemimi::TableItem errorOutput;
    // initialize error-output
    errorOutput.addColumn("Field");
    errorOutput.addColumn("Value");

    if(errorLocation.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"location", errorLocation});
    }

    if(possibleSolution.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"possible solution", possibleSolution});
    }
    if(blossomType.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-type", blossomType});
    }
    if(blossomGroupType.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-group-type", blossomGroupType});
    }
    if(blossomName.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-name", blossomName});
    }
    if(blossomFilePath.size() > 0) {
        errorOutput.addRow(std::vector<std::string>{"blossom-file-path", blossomFilePath});
    }

    errorOutput.addRow(std::vector<std::string>{"error-message", errorMessage});

    return errorOutput.toString(200);
}

} // namespace Sakura
} // namespace Kitsunemimi
