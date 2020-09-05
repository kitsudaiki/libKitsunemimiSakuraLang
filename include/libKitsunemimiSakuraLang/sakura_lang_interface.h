/**
 * @file       sakura_lang_interface.h
 *
 * @author     Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright  Apache License Version 2.0
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

#ifndef SAKURA_LANG_INTERFACE_H
#define SAKURA_LANG_INTERFACE_H

#include <string>
#include <map>

namespace Kitsunemimi
{
namespace Jinja2 {
class Jinja2Converter;
}
namespace Sakura
{
class SakuraGarden;
class ThreadPool;
class SubtreeQueue;
class Blossom;

class SakuraLangInterface
{
public:
    SakuraLangInterface();
    ~SakuraLangInterface();

    bool doesBlossomExist(const std::string &groupName,
                          const std::string &itemName);
    bool addBlossom(const std::string &groupName,
                    const std::string &itemName,
                    const Blossom* newBlossom);
    Blossom* getBlossom(const std::string &groupName,
                        const std::string &itemName);

    SakuraGarden* m_garden = nullptr;
    SubtreeQueue* m_queue = nullptr;
    ThreadPool* m_threadPoos = nullptr;
    Kitsunemimi::Jinja2::Jinja2Converter* m_jinja2Converter = nullptr;

private:
    std::map<std::string, std::map<std::string, Blossom*>> m_registeredBlossoms;
};

}
}

#endif // SAKURA_LANG_INTERFACE_H
