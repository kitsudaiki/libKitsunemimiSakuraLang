/**
 * @file        value_item_functions.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_VALUE_ITEM_FUNCTIONS_H
#define KITSUNEMIMI_SAKURA_LANG_VALUE_ITEM_FUNCTIONS_H

#include <string>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
class DataItem;
class DataMap;
class DataArray;
class DataValue;

namespace Sakura
{

DataItem* getValue(DataItem* item,
                   DataValue* key,
                   ErrorContainer &error);
DataArray* splitValue(DataValue* item,
                      DataValue* delimiter,
                      ErrorContainer &error);
DataValue* sizeValue(DataItem* item,
                     ErrorContainer &error);
DataValue* containsValue(DataItem* item,
                         DataValue* key,
                         ErrorContainer &error);
DataArray* appendValue(DataArray* item,
                       DataItem* value,
                       ErrorContainer &error);
DataMap* insertValue(DataMap* item,
                     DataValue* key,
                     DataItem* value,
                     ErrorContainer &error);
DataArray* clearEmpty(DataArray* item,
                      ErrorContainer &error);
DataItem* parseJson(DataValue* intput,
                    ErrorContainer &error);

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_VALUE_ITEM_FUNCTIONS_H
