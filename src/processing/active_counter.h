/**
 * @file        active_counter.h
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

#ifndef KITSUNEMIMI_SAKURA_LANG_ACTIVE_COUNTER_H
#define KITSUNEMIMI_SAKURA_LANG_ACTIVE_COUNTER_H

#include <thread>
#include <chrono>
#include <mutex>
#include <queue>

namespace Kitsunemimi
{
namespace Sakura
{

/**
 * @brief The ActiveCounter struct is only a simple thread-save counter. This counter should be
 *        increased, after the subtree was fully processed. All subtree-queue objects,
 *        which have the same source and belong to each other, share the same instance
 *        of this counter. With this, the source-thread should be able to check, that all its
 *        spawn subtree-objects have finished their task before increasing this counter.
 */
struct ActiveCounter
{
    std::mutex lock;
    uint32_t isCounter = 0;
    uint32_t shouldCount = 0;
    bool success = true;
    std::string outputMessage = "";

    ActiveCounter() {}

    /**
     * @brief increase the counter
     */
    void increaseCounter()
    {
        lock.lock();
        isCounter++;
        lock.unlock();
    }

    /**
     * @brief check, that the counter has reached the expected value
     *
     * @return true, if counter has reached the expected value, else false
     */
    bool isEqual()
    {
        bool result = false;
        lock.lock();
        result = isCounter == shouldCount;
        lock.unlock();
        return result;
    }

    /**
     * @brief register error in one of the spawned threads to inform the other threads
     *
     * @param errorMessage error-message
     */
    void registerError(const std::string &errorMessage)
    {
        lock.lock();
        success = false;
        outputMessage = errorMessage;
        lock.unlock();
    }
};

} // namespace Sakura
} // namespace Kitsunemimi

#endif // KITSUNEMIMI_SAKURA_LANG_ACTIVE_COUNTER_H
