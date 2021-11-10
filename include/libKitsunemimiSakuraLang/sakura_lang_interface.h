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
#include <filesystem>
#include <sys/ioctl.h>

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi
{
struct DataBuffer;
namespace Jinja2 {
class Jinja2Converter;
}
namespace Sakura
{
class SakuraGarden;
class TreeItem;
class ThreadPool;
class SubtreeQueue;
class SakuraThread;
class Blossom;
class BlossomGroupItem;
class BlossomItem;
class BlossomLeaf;
class Validator;
class SakuraParserInterface;
class SakuraFileCollector;
struct GrowthPlan;
struct BlossomStatus;

class SakuraLangInterface
{
public:
    static SakuraLangInterface* getInstance();

    ~SakuraLangInterface();

    bool triggerTree(DataMap& result,
                     const std::string &id,
                     const DataMap &initialValues,
                     BlossomStatus &status,
                     std::string &errorMessage);
    bool triggerBlossom(DataMap& result,
                        const std::string &blossomName,
                        const std::string &blossomGroupName,
                        const DataMap &initialValues,
                        BlossomStatus &status,
                        std::string &errorMessage);

    bool readFilesInDir(const std::string &directoryPath,
                        ErrorContainer &error);

    // blossom getter and setter
    bool doesBlossomExist(const std::string &groupName,
                          const std::string &itemName);
    bool addBlossom(const std::string &groupName,
                    const std::string &itemName,
                    Blossom *newBlossom);
    Blossom* getBlossom(const std::string &groupName,
                        const std::string &itemName);

    // add
    bool addTree(std::string id,
                 const std::string &treeContent,
                 std::string &errorMessage);
    bool addTemplate(const std::string &id,
                     const std::string &templateContent);
    bool addFile(const std::string &id,
                 Kitsunemimi::DataBuffer* data);
    bool addResource(std::string id,
                     const std::string &content,
                     std::string &errorMessage);

    // getter
    const std::string getTemplate(const std::string &id);
    DataBuffer* getFile(const std::string &id);

private:
    friend SakuraThread;
    friend Validator;

    SakuraLangInterface(const uint16_t numberOfThreads = 6,
                        const bool enableDebug = false);

    static SakuraLangInterface* m_instance;

    SakuraParserInterface* m_parser = nullptr;

    // internally used objects
    SakuraGarden* m_garden = nullptr;
    SubtreeQueue* m_queue = nullptr;
    ThreadPool* m_threadPoos = nullptr;
    Validator* m_validator = nullptr;
    SakuraFileCollector* m_fileCollector = nullptr;
    std::mutex m_lock;

    std::map<std::string, std::map<std::string, Blossom*>> m_registeredBlossoms;

    bool runProcess(DataMap &result,
                    GrowthPlan* plan,
                    TreeItem* tree);

    // output
    void printOutput(const BlossomGroupItem &blossomGroupItem);
    void printOutput(const BlossomLeaf &blossomItem);
    void printOutput(const std::string &output);
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_INTERFACE_H
