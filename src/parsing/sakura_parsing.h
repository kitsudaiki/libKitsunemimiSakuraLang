/**
 * @file        sakura_parsing.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_PARSING_H
#define KITSUNEMIMI_SAKURA_LANG_PARSING_H

#include <string>
#include <vector>
#include <utility>
#include <assert.h>
#include <fstream>
#include <map>
#include <deque>
#include <boost/filesystem.hpp>

namespace bfs = boost::filesystem;

namespace Kitsunemimi
{
class TableItem;

namespace Sakura
{
class TreeItem;
class SakuraItem;
class SakuraGarden;
class Validator;

class SakuraParserInterface;

class SakuraParsing
{
public:
    SakuraParsing(const bool debug = false);
    ~SakuraParsing();

    TreeItem* parseTreeString(const std::string &name,
                              const std::string &content,
                              std::string &errorMessage);

    bool parseTreeFiles(SakuraGarden &garden,
                        const bfs::path &initialFilePath,
                        std::string &errorMessage);

    // for internal usage
    void addFileToQueue(bfs::path relativePath);

private:
    SakuraParserInterface* m_parserInterface = nullptr;
    Validator* m_validator = nullptr;
    std::deque<std::string> m_fileQueue;
    std::vector<std::string> m_collectedDirectories;
    bfs::path m_rootPath;
    bfs::path m_currentFilePath;

    TreeItem* parseSingleFile(const bfs::path &relativePath,
                              const bfs::path &rootPath,
                              std::string &errorMessage);
    TreeItem* parseStringToTree(const std::string &content,
                                const std::string &filePath,
                                std::string &errorMessage);

    void initErrorOutput(TableItem &errorOutput);
    bool collectFiles(SakuraGarden &garden,
                      const bfs::path &dirPath,
                      std::string &errorMessage);
    bool collectResources(SakuraGarden &garden,
                           const bfs::path &dirPath,
                           std::string &errorMessage);
    bool collectTemplates(SakuraGarden &garden,
                          const bfs::path &dirPath,
                          std::string &errorMessage);
    bool getFilesInDir(SakuraGarden &garden,
                       const bfs::path &directory,
                       const std::string &type,
                       std::string &errorMessage);
    bool alreadyCollected(const bfs::path &path);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_PARSING_H
