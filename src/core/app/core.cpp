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

#include <system_error>

namespace viper {
namespace app {

Core::Core()
{
    _ctx              = std::make_shared<Context>();
    _ctx->_fileCfg    = std::make_shared<ConfigFile>();
    _root             = std::make_shared<option::Command>();
    _root->_use       = "viper";
    _root->_short     = "Viper application";
    _interactiveRoot  = std::make_shared<option::Command>();
    _interactiveRoot->_use   = "viper";
    _interactiveRoot->_short = "Viper (interactive)";
}

Core::~Core()
{
    Close();
}

void Core::AddFlag(const std::string& name, char shortName, const std::string& description, const option::Value& defaultValue)
{
    auto flag = std::make_shared<option::Flag<option::Value>>(name, std::string(1, shortName), description, defaultValue);
    _root->AddFlag(flag);
}

void Core::AddCommand(const std::string& name, const std::string& shortDesc, CommandHandler handler)
{
    auto cmd = std::make_shared<option::Command>();
    cmd->_use   = name;
    cmd->_short = shortDesc;
    cmd->_run   = [this, handler](const option::Args& args) {
        _ctx->SetArgs(args);
        std::error_code ec = handler(_ctx);
        return ec ? 1 : 0;
    };
    _root->AddCommand(cmd);
}

void Core::AddCommand(std::shared_ptr<option::Command> cmd)
{
    _root->AddCommand(cmd);
}

void Core::AddInteractiveCommand(const std::string& name, const std::string& shortDesc, CommandHandler handler)
{
    auto cmd = std::make_shared<option::Command>();
    cmd->_use   = name;
    cmd->_short = shortDesc;
    cmd->_run   = [this, handler](const option::Args& args) {
        _ctx->SetArgs(args);
        std::error_code ec = handler(_ctx);
        return ec ? 1 : 0;
    };
    _interactiveRoot->AddCommand(cmd);
}

void Core::AddInteractiveCommand(std::shared_ptr<option::Command> cmd)
{
    _interactiveRoot->AddCommand(cmd);
}

ContextPtr Core::GetContext() const
{
    return _ctx;
}

std::error_code Core::Run(int argc, char* argv[], CommandHandler executor)
{
    _root->_run = [this, executor](const option::Args& args) {
        _ctx->SetArgs(args);
        std::error_code ec = executor(_ctx);
        return ec ? 1 : 0;
    };

    int code = _root->Execute(argc, argv);
    if (code == 0)
    {
        return std::error_code{};
    }
    return std::make_error_code(std::errc::invalid_argument);
}

int Core::ExecuteArgs(int argc, char* argv[])
{
    return _interactiveRoot->Execute(argc, argv);
}

void Core::Close()
{
}

} // namespace app
} // namespace viper
