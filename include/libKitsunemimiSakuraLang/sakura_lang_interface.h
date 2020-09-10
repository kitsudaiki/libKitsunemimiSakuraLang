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

#ifndef KITSUNEMIMI_SAKURA_LANG_INTERFACE_H
#define KITSUNEMIMI_SAKURA_LANG_INTERFACE_H

#include <string>
#include <map>
#include <mutex>
#include <unistd.h>
#include <sys/ioctl.h>
#include <boost/filesystem.hpp>

#include <libKitsunemimiCommon/common_items/data_items.h>

namespace Kitsunemimi
{
class DataBuffer;
namespace Jinja2 {
class Jinja2Converter;
}
namespace Sakura
{
class SakuraGarden;
class SakuraItem;
class ThreadPool;
class SubtreeQueue;
class SakuraThread;
class Blossom;
class BlossomGroupItem;
class BlossomItem;
class BlossomLeaf;
class Validator;

namespace bfs = boost::filesystem;

class SakuraLangInterface
{
public:
    SakuraLangInterface(const bool enableDebug = false);
    ~SakuraLangInterface();

    // processing
    bool processFiles(const std::string &inputPath,
                      const DataMap &initialValues,
                      const bool dryRun,
                      std::string &errorMessage);

    // getter
    const std::string getTemplate(const std::string &relativePath);
    DataBuffer* getFile(const std::string &relativePath);
    const bfs::path getRelativePath(const bfs::path &blossomFilePath,
                                    const bfs::path &blossomInternalRelPath);

    // blossom getter and setter
    bool doesBlossomExist(const std::string &groupName,
                          const std::string &itemName);
    bool addBlossom(const std::string &groupName,
                    const std::string &itemName,
                    Blossom *newBlossom);
    Blossom* getBlossom(const std::string &groupName,
                        const std::string &itemName);

    // output
    void printOutput(const BlossomGroupItem &blossomGroupItem);
    void printOutput(const BlossomLeaf &blossomItem);
    void printOutput(const std::string &output);


    Kitsunemimi::Jinja2::Jinja2Converter* jinja2Converter = nullptr;

private:
    friend SakuraThread;
    friend Validator;

    SakuraGarden* m_garden = nullptr;

    SubtreeQueue* m_queue = nullptr;
    ThreadPool* m_threadPoos = nullptr;
    std::mutex m_mutex;

    std::map<std::string, std::map<std::string, Blossom*>> m_registeredBlossoms;

    bool runProcess(SakuraItem* item,
                    const DataMap &initialValues,
                    std::string &errorMessage);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_INTERFACE_H
