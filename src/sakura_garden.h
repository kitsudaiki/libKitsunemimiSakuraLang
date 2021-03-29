/**
 * @file        sakura_garden.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_GARDEN_H
#define KITSUNEMIMI_SAKURA_LANG_GARDEN_H

#include <vector>
#include <string>
#include <map>

#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace Kitsunemimi
{
struct DataBuffer;

namespace Sakura
{
class TreeItem;
class Validator;

class SakuraGarden
{
public:
    SakuraGarden();
    ~SakuraGarden();

    const bfs::path getRelativePath(const bfs::path &blossomFilePath,
                                    const bfs::path &blossomInternalRelPath);
    // add
    bool addTree(const std::string &id, TreeItem* tree);
    bool addResource(const std::string &id, TreeItem* resource);
    bool addTemplate(const std::string &id, const std::string &templateContent);
    bool addFile(const std::string &id, Kitsunemimi::DataBuffer* fileContent);

    // check
    bool containsTree(std::string id);

    // get
    TreeItem* getTree(std::string id);
    TreeItem* getRessource(const std::string &id);
    const std::string getTemplate(const std::string &id);
    DataBuffer* getFile(const std::string &id);

    // object-handling
    std::string rootPath = "";

private:
    friend Validator;

    std::map<std::string, TreeItem*> m_trees;
    std::map<std::string, TreeItem*> m_resources;
    std::map<std::string, std::string> m_templates;
    std::map<std::string, Kitsunemimi::DataBuffer*> m_files;
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_GARDEN_H
