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

#include "viper/local/local.h"
#include "core/option/command.h"

#include <iostream>
#include <system_error>

namespace viper {
namespace local {

namespace {

std::error_code LocalCpu(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "local cpu info (placeholder)" << std::endl;
    return std::error_code{};
}

std::error_code LocalMem(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "local memory info (placeholder)" << std::endl;
    return std::error_code{};
}

std::error_code LocalDisk(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "local disk info (placeholder)" << std::endl;
    return std::error_code{};
}

std::error_code LocalNet(viper::app::ContextPtr ctx)
{
    (void)ctx;
    std::cout << "local network info (placeholder)" << std::endl;
    return std::error_code{};
}

std::shared_ptr<option::Command> BuildLocalCommand(viper::app::CorePtr core)
{
    auto localCmd    = std::make_shared<option::Command>();
    localCmd->_use   = "local";
    localCmd->_short = "local resource info (cpu, mem, disk, net)";

    auto localCpuCmd    = std::make_shared<option::Command>();
    localCpuCmd->_use   = "cpu";
    localCpuCmd->_short = "local CPU info";
    localCpuCmd->_run   = [core](const option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = LocalCpu(core->GetContext());
        return ec ? 1 : 0;
    };
    localCmd->AddCommand(localCpuCmd);

    auto localMemCmd    = std::make_shared<option::Command>();
    localMemCmd->_use   = "mem";
    localMemCmd->_short = "local memory info";
    localMemCmd->_run   = [core](const option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = LocalMem(core->GetContext());
        return ec ? 1 : 0;
    };
    localCmd->AddCommand(localMemCmd);

    auto localDiskCmd    = std::make_shared<option::Command>();
    localDiskCmd->_use   = "disk";
    localDiskCmd->_short = "local disk info";
    localDiskCmd->_run   = [core](const option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = LocalDisk(core->GetContext());
        return ec ? 1 : 0;
    };
    localCmd->AddCommand(localDiskCmd);

    auto localNetCmd    = std::make_shared<option::Command>();
    localNetCmd->_use   = "net";
    localNetCmd->_short = "local network info";
    localNetCmd->_run   = [core](const option::Args& args) {
        core->GetContext()->SetArgs(args);
        auto ec = LocalNet(core->GetContext());
        return ec ? 1 : 0;
    };
    localCmd->AddCommand(localNetCmd);

    return localCmd;
}

} // namespace

viper::internal::ErrorCode RegisterLocalCommand(viper::app::CorePtr core)
{
    core->AddCommand(BuildLocalCommand(core));
    return viper::internal::ErrorCode::SUCCESS;
}

viper::internal::ErrorCode RegisterLocalInteractiveCommand(viper::app::CorePtr core)
{
    core->AddInteractiveCommand(BuildLocalCommand(core));
    return viper::internal::ErrorCode::SUCCESS;
}

} // namespace local
} // namespace viper
