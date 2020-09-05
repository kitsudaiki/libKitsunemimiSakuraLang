/**
 * @file       sakura_lang_interface.cpp
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

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>

#include <sakura_garden.h>
#include <processing/subtree_queue.h>
#include <processing/thread_pool.h>

#include <libKitsunemimiJinja2/jinja2_converter.h>

namespace Kitsunemimi
{
namespace Sakura
{

SakuraLangInterface::SakuraLangInterface()
{
    m_garden = new SakuraGarden();
    m_queue = new SubtreeQueue();
    m_jinja2Converter = new Kitsunemimi::Jinja2::Jinja2Converter();
    // TODO: make number of threads configurable
    m_threadPoos = new ThreadPool(6, this);
}

SakuraLangInterface::~SakuraLangInterface()
{
    delete m_garden;
    delete m_queue;
    delete m_threadPoos;
    delete m_jinja2Converter;
}

/**
 * @brief SakuraLangInterface::doesBlossomExist
 * @param groupName
 * @param itemName
 * @return
 */
bool
SakuraLangInterface::doesBlossomExist(const std::string &groupName,
                                      const std::string &itemName)
{

}

/**
 * @brief SakuraLangInterface::addBlossom
 * @param groupName
 * @param itemName
 * @param newBlossom
 * @return
 */
bool
SakuraLangInterface::addBlossom(const std::string &groupName,
                                const std::string &itemName,
                                const Blossom* newBlossom)
{

}

/**
 * @brief SakuraLangInterface::getBlossom
 * @param groupName
 * @param itemName
 * @return
 */
Blossom*
SakuraLangInterface::getBlossom(const std::string &groupName,
                                const std::string &itemName)
{

}

}
}
