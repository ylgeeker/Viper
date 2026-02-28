/**
 * Copyright 2025 Viper authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#ifndef _VIPER_CORE_APP_FLAG_H_
#define _VIPER_CORE_APP_FLAG_H_

#include "third-party/cmdline/cmdline.h"

#include <functional>
#include <string>
#include <vector>

namespace viper {
namespace app {

using FlagHandler = std::function<std::error_code(void* ctx)>;

struct FlagObject
{
};

template <typename T>
struct Flag
{
    std::string    _fullName;
    char           _shortName;
    std::string    _description;
    FlagHandler    _handler;
    bool           _required = false;
    T              _value    = T();
    std::vector<T> _range;
    std::vector<T> _oneof;
    bool           _setted = false;
};

struct NoValue
{
};

template <typename T>
void AddFlag(cmdline::parser* parser, const Flag<T>& flag)
{
    parser->add<T>(flag._fullName, flag._shortName, flag._description, flag._required, flag._value);
}

template <typename T>
T GetFlagValue(cmdline::parser* parser, const Flag<T>& flag)
{
    return parser->get<T>(flag._fullName);
}

template <typename T>
bool FlagExist(cmdline::parser* parser, const Flag<T>& flag)
{
    return parser->exist(flag._fullName);
}

using FlagInt     = Flag<int>;
using FlagBool    = Flag<bool>;
using FlagNoValue = Flag<NoValue>;
using FlagString  = Flag<std::string>;

template <>
void AddFlag<NoValue>(cmdline::parser* parser, const Flag<NoValue>& flag);

} // namespace app
} // namespace viper

#endif
