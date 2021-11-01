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

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief constructor
 */
Blossom::Blossom() {}

/**
 * @brief destructor
 */
Blossom::~Blossom() {}

/**
 * @brief register field for validation of incoming messages
 *
 * @param name name of the filed to identifiy value
 * @param type INPUT_TYPE or OUTPUT_TYPE
 * @param required false, to make field optional, true to make it required
 *
 * @return false, if already name already registered, else true
 */
bool
Blossom::registerField(const std::string &name,
                       const IO_ValueType type,
                       const bool required)
{
    std::map<std::string, BlossomValidDef>::const_iterator defIt;
    defIt = validationMap.find(name);
    if(defIt != validationMap.end()) {
        return false;
    }

    validationMap.emplace(name, BlossomValidDef(type, required));

    return true;
}

/**
 * @brief execute blossom
 *
 * @param blossomLeaf leaf-object for values-handling while processing
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
Blossom::growBlossom(BlossomLeaf &blossomLeaf,
                     uint64_t &status,
                     std::string &errorMessage)
{
    blossomLeaf.output.clear();

    // process blossom
    LOG_DEBUG("runTask " + blossomLeaf.blossomName);
    const bool ret = runTask(blossomLeaf, status, errorMessage);

    // handle result
    if(ret == false)
    {
        errorMessage = createError(blossomLeaf, "blossom execute", errorMessage);
        return false;
    }

    return true;
}

/**
 * @brief validate given input with the required and allowed values of the selected blossom
 *
 * @param input given input values
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
Blossom::validateInput(DataMap &input,
                       std::string &errorMessage)
{
    if(allowUnmatched == false)
    {
        // check if all keys in the values of the blossom-item also exist in the required-key-list
        std::map<std::string, DataItem*>::const_iterator compareIt;
        for(compareIt = input.map.begin();
            compareIt != input.map.end();
            compareIt++)
        {
            std::map<std::string, BlossomValidDef>::const_iterator defIt;
            defIt = validationMap.find(compareIt->first);
            if(defIt == validationMap.end())
            {
                // build error-output
                const std::string message = "variable \""
                                            + compareIt->first
                                            + "\" is not in the list of allowed keys";
                errorMessage = createError("validator", message);
                return false;
            }
        }
    }

    // check that all keys in the required keys are also in the values of the blossom-item
    std::map<std::string, BlossomValidDef>::const_iterator defIt;
    for(defIt = validationMap.begin();
        defIt != validationMap.end();
        defIt++)
    {
        if(defIt->second.isRequired == true
                && defIt->second.type == IO_ValueType::INPUT_TYPE)
        {
            // search for values
            const bool ret = input.contains(defIt->first);
            if(ret == false)
            {
                const std::string message = "variable \""
                                            + defIt->first
                                            + "\" is required, but is not set.";
                errorMessage = createError("validator", message);
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
 * @param errorMessage reference for error-message
 *
 * @return true, if successful, else false
 */
bool
Blossom::validateInput(BlossomItem &blossomItem,
                       const std::string &filePath,
                       std::string &errorMessage)
{
    std::map<std::string, IO_ValueType> compareMap;
    blossomItem.values.getCompareMap(compareMap);

    if(allowUnmatched == false)
    {
        // check if all keys in the values of the blossom-item also exist in the required-key-list
        std::map<std::string, IO_ValueType>::const_iterator compareIt;
        for(compareIt = compareMap.begin();
            compareIt != compareMap.end();
            compareIt++)
        {
            std::map<std::string, BlossomValidDef>::const_iterator defIt;
            defIt = validationMap.find(compareIt->first);
            if(defIt == validationMap.end())
            {
                // build error-output
                const std::string message = "variable \""
                                            + compareIt->first
                                            + "\" is not in the list of allowed keys";
                errorMessage = createError(blossomItem,
                                           filePath,
                                           "validator",
                                           message);
                return false;
            }
        }
    }

    // check that all keys in the required keys are also in the values of the blossom-item
    std::map<std::string, BlossomValidDef>::const_iterator defIt;
    for(defIt = validationMap.begin();
        defIt != validationMap.end();
        defIt++)
    {
        if(defIt->second.isRequired == true)
        {
            // search for values
            std::map<std::string, IO_ValueType>::const_iterator compareIt;
            compareIt = compareMap.find(defIt->first);
            if(compareIt != compareMap.end())
            {
                if(defIt->second.type != compareIt->second)
                {
                    const std::string message = "variable \""
                                                + defIt->first
                                                + "\" has not the correct input/output type";
                    errorMessage = createError(blossomItem,
                                               filePath,
                                               "validator",
                                               message);
                    return false;
                }
            }
            else
            {
                const std::string message = "variable \""
                                            + defIt->first
                                            + "\" is required, but is not set.";
                errorMessage = createError(blossomItem,
                                           filePath,
                                           "validator",
                                           message);
                return false;
            }
        }
    }

    return true;
}

} // namespace Sakura
} // namespace Kitsunemimi
