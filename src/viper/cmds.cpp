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

std::error_code ShowCpu(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "cpu info (placeholder)" << std::endl;
    return std::error_code{};
}

std::error_code ShowMem(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "memory info (placeholder)" << std::endl;
    return std::error_code{};
}

std::error_code ShowDisk(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "disk info (placeholder)" << std::endl;
    return std::error_code{};
}

std::error_code ShowNet(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "network info (placeholder)" << std::endl;
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

    auto cpuCmd    = std::make_shared<viper::option::Command>();
    cpuCmd->_use   = "cpu";
    cpuCmd->_short = "show CPU info";
    cpuCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowCpu(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(cpuCmd);

    auto memCmd    = std::make_shared<viper::option::Command>();
    memCmd->_use   = "mem";
    memCmd->_short = "show memory info";
    memCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowMem(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(memCmd);

    auto diskCmd    = std::make_shared<viper::option::Command>();
    diskCmd->_use   = "disk";
    diskCmd->_short = "show disk info";
    diskCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowDisk(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(diskCmd);

    auto netCmd    = std::make_shared<viper::option::Command>();
    netCmd->_use   = "net";
    netCmd->_short = "show network info";
    netCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowNet(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(netCmd);

    core->AddCommand(showCmd);

    return viper::internal::ErrorCode::SUCCESS;
}

viper::internal::ErrorCode RegisterInteractiveCommands(viper::app::CorePtr core, ConfigurationPtr config)
{
    core->AddInteractiveCommand("version", "show the version",
                                [config](viper::app::ContextPtr ctx) { return VersionCommand(ctx, config); });

    core->AddInteractiveCommand("clear", "clear the screen",
                                [](viper::app::ContextPtr ctx) { return ClearScreenCommand(ctx); });

    auto showCmd    = std::make_shared<viper::option::Command>();
    showCmd->_use   = "show";
    showCmd->_short = "show system or resource info";

    auto cpuCmd    = std::make_shared<viper::option::Command>();
    cpuCmd->_use   = "cpu";
    cpuCmd->_short = "show CPU info";
    cpuCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowCpu(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(cpuCmd);

    auto memCmd    = std::make_shared<viper::option::Command>();
    memCmd->_use   = "mem";
    memCmd->_short = "show memory info";
    memCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowMem(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(memCmd);

    auto diskCmd    = std::make_shared<viper::option::Command>();
    diskCmd->_use   = "disk";
    diskCmd->_short = "show disk info";
    diskCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowDisk(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(diskCmd);

    auto netCmd    = std::make_shared<viper::option::Command>();
    netCmd->_use   = "net";
    netCmd->_short = "show network info";
    netCmd->_run   = [core](const viper::option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = ShowNet(core->GetContext());
        return ec ? 1 : 0;
    };
    showCmd->AddCommand(netCmd);

    core->AddInteractiveCommand(showCmd);

    return viper::internal::ErrorCode::SUCCESS;
}
