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

#ifndef LIBKITSUNE_SAKURA_PARSER_H
#define LIBKITSUNE_SAKURA_PARSER_H

#include <utility>
#include <string>

namespace Kitsunemimi
{
namespace Common {
class DataItem;
}
namespace Sakura
{
class SakuraParserInterface;

class SakuraParsing
{
public:
    SakuraParsing(const bool traceParsing);
    ~SakuraParsing();

    std::pair<Common::DataItem*, bool> parse(const std::string &inputString);

private:
    SakuraParserInterface* m_parser = nullptr;
};

}  // namespace Sakura
}  // namespace Kitsunemimi

#endif // LIBKITSUNE_SAKURA_PARSER_H
