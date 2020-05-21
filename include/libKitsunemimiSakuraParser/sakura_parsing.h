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
#include <boost/filesystem.hpp>

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

    bool parseFiles(SakuraGarden &result,
                    const std::string &rootPath,
                    std::string &errorMessage);

    TreeItem* parseString(const std::string &content,
                          std::string &errorMessage);

private:
    SakuraParserInterface* m_parser = nullptr;
    bool m_debug = false;

    bool parseAllFiles(SakuraGarden &result,
                       const std::string &rootPath,
                       std::string &errorMessage);
    TreeItem* parseSingleFile(const std::string &relativePath,
                              const std::string &rootPath,
                              std::string &errorMessage);
    SakuraItem* getParsedFileContent(const std::string &name="");

    void initErrorOutput(TableItem &errorOutput);
    void collectFiles(SakuraGarden &result,
                      const std::string &path);
    void getFilesInDir(SakuraGarden &result,
                       const boost::filesystem::path &directory,
                       const std::string &rootPath,
                       const std::string &type);
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // SAKURA_PARSING_H
