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

// local header file
#include "viper/cmds.h"
#include "viper/local/local.h"

// third party header file
#include "core/option/command.h"

// c++ standard header file
#include <iostream>
#include <system_error>

namespace {

std::error_code VersionCommand(viper::app::ContextPtr ctx, ConfigurationPtr config)
{
    (void)ctx;
    std::cout << "viper version: " << config->_version << std::endl;
    return std::error_code{};
}

std::error_code ClearScreenCommand(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "\033[2J\033[H" << std::flush;
    return std::error_code{};
}

} // namespace

viper::internal::ErrorCode RegisterCommands(viper::app::CorePtr core, ConfigurationPtr config)
{
    core->AddCommand("version", "show the version",
                     [config](viper::app::ContextPtr ctx) { return VersionCommand(ctx, config); });

    auto showCmd    = std::make_shared<viper::option::Command>();
    showCmd->_use   = "show";
    showCmd->_short = "show system or resource info";

    core->AddCommand(showCmd);

    auto ec = viper::local::RegisterLocalCommand(core);
    if (ec != viper::internal::ErrorCode::SUCCESS)
    {
        return ec;
    }

    return viper::internal::ErrorCode::SUCCESS;
}

viper::internal::ErrorCode RegisterInteractiveCommands(viper::app::CorePtr core, ConfigurationPtr config)
{
    core->AddInteractiveCommand("clear", "clear the screen",
                                [](viper::app::ContextPtr ctx) { return ClearScreenCommand(ctx); });

    core->AddInteractiveCommand("use", "switch to a context (e.g. use show)",
                                [](viper::app::ContextPtr) { return std::error_code{}; });

    core->AddInteractiveCommand("back", "go back to previous context",
                                [](viper::app::ContextPtr) { return std::error_code{}; });

    auto showCmd    = std::make_shared<viper::option::Command>();
    showCmd->_use   = "show";
    showCmd->_short = "show system or resource info";

    core->AddInteractiveCommand(showCmd);

    auto ec = viper::local::RegisterLocalInteractiveCommand(core);
    if (ec != viper::internal::ErrorCode::SUCCESS)
    {
        return ec;
    }

    return viper::internal::ErrorCode::SUCCESS;
}
