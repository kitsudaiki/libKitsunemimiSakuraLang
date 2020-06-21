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

#ifndef SAKURA_PARSING_H
#define SAKURA_PARSING_H

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

class SakuraParserInterface;

class SakuraParsing
{
public:
    SakuraParsing(const bool debug = false);
    ~SakuraParsing();

    bool parseTreeFiles(SakuraGarden &result,
                        const bfs::path &initialFilePath,
                        std::string &errorMessage);

    bool parseTreeString(SakuraGarden &result,
                         const bfs::path &relativePath,
                         const std::string &content,
                         std::string &errorMessage);

    bool parseRessourceString(SakuraGarden &result,
                              const std::string &content,
                              std::string &errorMessage);

    void addFileToQueue(bfs::path oldRelativePath);


private:
    SakuraParserInterface* m_parser = nullptr;
    bool m_debug = false;
    std::deque<std::string> m_fileQueue;
    std::vector<std::string> m_collectedDirectories;
    bfs::path m_rootPath;
    bfs::path m_currentFilePath;

    TreeItem* parseSingleFile(const bfs::path &relativePath,
                              const bfs::path &rootPath,
                              std::string &errorMessage);
    TreeItem* parseStringToTree(const std::string &content,
                                std::string &errorMessage);

    void initErrorOutput(TableItem &errorOutput);
    bool collectFiles(SakuraGarden &result,
                      const bfs::path &dirPath,
                      std::string &errorMessage);
    bool collectTemplates(SakuraGarden &result,
                          const bfs::path &dirPath,
                          std::string &errorMessage);
    bool getFilesInDir(SakuraGarden &result,
                       const bfs::path &directory,
                       const std::string &type,
                       std::string &errorMessage);
    bool alreadyCollected(const bfs::path &path);
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // SAKURA_PARSING_H
