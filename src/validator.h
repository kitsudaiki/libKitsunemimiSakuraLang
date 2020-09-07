/**
 * @file        validator.h
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

#ifndef SAKURA_LANG_VALIDATOR_H
#define SAKURA_LANG_VALIDATOR_H

#include <string>
#include <map>

#include <libKitsunemimiSakuraLang/items/sakura_items.h>
#include <libKitsunemimiSakuraLang/sakura_garden.h>

namespace Kitsunemimi
{
namespace Sakura
{
class SakuraLangInterface;

bool checkBlossomItem(SakuraLangInterface *interface,
                      BlossomItem &blossomItem,
                      std::string &errorMessage);

bool checkSakuraItem(SakuraLangInterface *interface,
                     SakuraItem* sakuraItem,
                     const std::string &filePath,
                     std::string &errorMessage);

bool checkAllItems(SakuraLangInterface *interface,
                   std::string &errorMessage);

}
}

#endif // SAKURA_LANG_VALIDATOR_H
