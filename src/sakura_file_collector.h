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

#ifndef KITSUNEMIMI_SAKURA_LANG_SAKURAFILECOLLECTOR_H
#define KITSUNEMIMI_SAKURA_LANG_SAKURAFILECOLLECTOR_H

#include <string>
#include <map>
#include <mutex>
#include <unistd.h>
#include <filesystem>
#include <sys/ioctl.h>
#include <libKitsunemimiCommon/logger.h>

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
                        ErrorContainer &error);

private:
    SakuraLangInterface* m_interface = nullptr;

    bool collectFiles(const std::filesystem::path &rootPath,
                      const std::filesystem::path &dirPath,
                      ErrorContainer &error);
    bool collectResources(const std::filesystem::path &rootPath,
                          const std::filesystem::path &dirPath,
                          ErrorContainer &error);
    bool collectTemplates(const std::filesystem::path &rootPath,
                          const std::filesystem::path &dirPath,
                          ErrorContainer &error);
    bool getFilesInDir(const std::filesystem::path &rootPath,
                       const std::filesystem::path &directory,
                       const std::string &type,
                       ErrorContainer &error);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_SAKURAFILECOLLECTOR_H
