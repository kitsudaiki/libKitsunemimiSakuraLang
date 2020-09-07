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

namespace Kitsunemimi
{
namespace Sakura
{

Blossom::Blossom() {}

Blossom::~Blossom() {}

/**
 * @brief SakuraBlossom::growBlossom
 * @return
 */
void
Blossom::growBlossom(BlossomItem &blossomItem,
                     std::string &errorMessage)
{
    //----------------------------------------------------------------------------------------------
    LOG_DEBUG("initBlossom " + blossomItem.blossomName);

    initBlossom(blossomItem);

    if(blossomItem.success == false)
    {
        errorMessage = createError(blossomItem, "blossom init", blossomItem.outputMessage);
        return;
    }

    //----------------------------------------------------------------------------------------------
    LOG_DEBUG("preCheck " + blossomItem.blossomName);

    preCheck(blossomItem);

    if(blossomItem.success == false)
    {
        errorMessage = createError(blossomItem, "blossom pre-check", blossomItem.outputMessage);
        return;
    }

    if(blossomItem.skip) {
        return;
    }

    //----------------------------------------------------------------------------------------------
    LOG_DEBUG("runTask " + blossomItem.blossomName);

    runTask(blossomItem);

    if(blossomItem.success == false)
    {
        errorMessage = createError(blossomItem, "blossom execute", blossomItem.outputMessage);
        return;
    }

    //----------------------------------------------------------------------------------------------
    LOG_DEBUG("postCheck " + blossomItem.blossomName);

    postCheck(blossomItem);

    if(blossomItem.success == false)
    {
        errorMessage = createError(blossomItem, "blossom post-check", blossomItem.outputMessage);
        return;
    }

    //----------------------------------------------------------------------------------------------
    LOG_DEBUG("closeBlossom " + blossomItem.blossomName);

    closeBlossom(blossomItem);

    if(blossomItem.success == false)
    {
        errorMessage = createError(blossomItem, "blossom close", blossomItem.outputMessage);
        return;
    }

    //----------------------------------------------------------------------------------------------

    return;
}

/**
 * @brief Blossom::validateInput
 * @param blossomItem
 * @return
 */
bool
Blossom::validateInput(BlossomItem &blossomItem,
                       std::string &errorMessage)
{
    // if "*" is in the list of required key, there is more allowed as the list contains items
    // for example the print-blossom allows all key
    if(m_requiredKeys.contains("*") == false)
    {
        // check if all keys in the values of the blossom-item also exist in the required-key-list
        std::map<std::string, ValueItem>::const_iterator it;
        for(it = blossomItem.values.m_valueMap.begin();
            it != blossomItem.values.m_valueMap.end();
            it++)
        {
            ValueItem tempItem = blossomItem.values.getValueItem(it->first);
            if(tempItem.item == nullptr
                    && tempItem.type == ValueItem::INPUT_PAIR_TYPE)
            {
                // build error-output
                const std::string message = "variable \""
                                            + it->first
                                            + "\" is not in the list of allowed keys";
                errorMessage = createError(blossomItem, "validator", message);
                return false;
            }
        }
    }

    // check that all keys in the required keys are also in the values of the blossom-item
    std::map<std::string, DataItem*>::const_iterator it;
    for(it = m_requiredKeys.m_map.begin();
        it != m_requiredKeys.m_map.end();
        it++)
    {
        if(it->first == "*") {
            continue;
        }

        // if values of the blossom-item doesn't contain the key and the key is not optional,
        // then create an error-message
        if(blossomItem.values.contains(it->first) == false
                && it->second->toValue()->getBool() == true)
        {
            const std::string message = "variable \""
                                        + it->first
                                        + "\" is required, but is not set.";
            errorMessage = createError(blossomItem, "validator", message);
            return false;
        }
    }

    // check output
    std::map<std::string, ValueItem>::const_iterator outputId;
    for(outputId = blossomItem.values.m_valueMap.begin();
        outputId != blossomItem.values.m_valueMap.end();
        outputId++)
    {
        ValueItem tempItem = blossomItem.values.getValueItem(outputId->first);
        if(m_hasOutput == false
                && tempItem.type == ValueItem::OUTPUT_PAIR_TYPE)
        {
            // build error-output
            const std::string message = "variable \""
                                        + outputId->first
                                        + "\" is declared as output-variable, "
                                          "but the blossom has no "
                                          "output, which could be written into a variable.";
            errorMessage = createError(blossomItem, "validator", message);
            return false;
        }
    }

    return true;
}

}
}
