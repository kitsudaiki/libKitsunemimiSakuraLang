/**
 * @file        sakura_file_collector.h
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

#ifndef SAKURAFILECOLLECTOR_H
#define SAKURAFILECOLLECTOR_H

#include <string>
#include <map>
#include <mutex>
#include <unistd.h>
#include <filesystem>
#include <sys/ioctl.h>

namespace Kitsunemimi
{
class TableItem;
namespace Sakura
{
class SakuraLangInterface;

class SakuraFileCollector
{
public:
    SakuraFileCollector(SakuraLangInterface* interface);

    bool readFilesInDir(const std::string &directoryPath,
                        std::string &errorMessage);

private:
    SakuraLangInterface* m_interface = nullptr;

    bool collectFiles(const std::filesystem::path &rootPath,
                      const std::filesystem::path &dirPath,
                      std::string &errorMessage);
    bool collectResources(const std::filesystem::path &rootPath,
                          const std::filesystem::path &dirPath,
                          std::string &errorMessage);
    bool collectTemplates(const std::filesystem::path &rootPath,
                          const std::filesystem::path &dirPath,
                          std::string &errorMessage);
    bool getFilesInDir(const std::filesystem::path &rootPath,
                       const std::filesystem::path &directory,
                       const std::string &type,
                       std::string &errorMessage);

    void initErrorOutput(TableItem &errorOutput);
};

}
}

#endif // SAKURAFILECOLLECTOR_H
