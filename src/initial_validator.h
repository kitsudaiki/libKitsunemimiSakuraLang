﻿/**
 * @file        initial_validator.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_INITIAL_VALIDATOR_H
#define KITSUNEMIMI_SAKURA_LANG_INITIAL_VALIDATOR_H

#include <string>
#include <map>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
namespace Sakura
{
class SakuraLangInterface;
class BlossomItem;
class SakuraItem;

class InitialValidator
{
public:
    InitialValidator();
    ~InitialValidator();

    bool checkBlossomItem(BlossomItem &blossomItem,
                          const std::string &filePath,
                          ErrorContainer &error);

    bool checkSakuraItem(SakuraItem* sakuraItem,
                         const std::string &filePath,
                         ErrorContainer &error);
    bool checkAllItems(ErrorContainer &error);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_INITIAL_VALIDATOR_H
