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

#include "core/app/core.h"
#include "core/app/config_file.h"
#include "core/app/context.h"
#include "core/app/flag.h"

#include <system_error>

namespace viper {
namespace app {

Core::Core()
{
    _ctx           = std::make_shared<Context>();
    _ctx->_flags   = std::make_shared<ContextFlag>();
    _ctx->_fileCfg = std::make_shared<ConfigFile>();
}

Core::~Core()
{
    Close();
}

void Core::AddCommand(const FlagInt& flag)
{
    AddFlag(&_parser, flag);
    _iFlags.push_back(flag);
}

void Core::AddCommand(const FlagString& flag)
{
    AddFlag(&_parser, flag);
    _sFlags.push_back(flag);
}

void Core::AddCommand(const FlagBool& flag)
{
    AddFlag(&_parser, flag);
    _bFlags.push_back(flag);
}

void Core::AddCommand(const FlagNoValue& flag)
{
    AddFlag(&_parser, flag);
    _nvFlags.push_back(flag);
}

std::error_code Core::Run(int argc, char* argv[], CommandHandler executor)
{
    _parser.parse_check(argc, argv);

    for (auto& flag : _iFlags)
    {
        if (!FlagExist(&_parser, flag))
        {
            continue;
        }

        flag._setted = true;
        flag._value  = GetFlagValue(&_parser, flag);

        _ctx->_flags->_iFlags[flag._fullName] = flag;
    }

    for (auto& flag : _sFlags)
    {
        if (!FlagExist(&_parser, flag))
        {
            continue;
        }

        flag._setted = true;
        flag._value  = GetFlagValue(&_parser, flag);

        _ctx->_flags->_sFlags[flag._fullName] = flag;
    }

    for (auto& flag : _bFlags)
    {
        if (!FlagExist(&_parser, flag))
        {
            continue;
        }

        flag._setted = true;
        flag._value  = GetFlagValue(&_parser, flag);

        _ctx->_flags->_bFlags[flag._fullName] = flag;
    }

    for (auto& flag : _nvFlags)
    {
        if (!FlagExist(&_parser, flag))
        {
            continue;
        }

        flag._setted = true;

        _ctx->_flags->_nvFlags[flag._fullName] = flag;
    }

    return executor(_ctx);
}

void Core::Close()
{
}

} // namespace app
} // namespace viper

