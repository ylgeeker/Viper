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

#include "core/assist/format.h"
#include "core/host/cpu.h"
#include "core/host/disk.h"
#include "core/host/mem.h"
#include "core/host/net.h"
#include "core/option/command.h"

#include <iomanip>
#include <iostream>
#include <system_error>

namespace viper {
namespace local {

namespace {

std::error_code LocalCpu(viper::app::ContextPtr ctx)
{
    (void)ctx;

    viper::host::CpuInfo info;
    std::error_code      ec = viper::host::GetCpuInfo(info);
    if (ec)
    {
        return ec;
    }

    const int labelWidth = 22;
    std::cout << "\n--- CPU ---\n";
    std::cout << std::left << std::setw(labelWidth) << "Model name" << info.modelName << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Vendor" << info.vendor << "\n";
    std::cout << std::left << std::setw(labelWidth) << "CPU(s)" << info.logicalCount << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Thread(s) per core" << info.threadsPerCore << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Core(s) per socket" << info.coresPerSocket << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Socket(s)" << info.sockets << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Cache size" << info.cacheSize << "\n";

    if (info.frequencyMhz >= 0.0)
    {
        std::cout << std::left << std::setw(labelWidth) << "Frequency" << std::fixed << std::setprecision(2)
                  << info.frequencyMhz << " MHz\n";
    }

    if (!info.cpuFamily.empty() || !info.model.empty() || !info.stepping.empty())
    {
        std::cout << std::left << std::setw(labelWidth) << "Family / Model / Stepping"
                  << " " << (info.cpuFamily.empty() ? "?" : info.cpuFamily) << " / "
                  << (info.model.empty() ? "?" : info.model) << " / "
                  << (info.stepping.empty() ? "?" : info.stepping) << "\n";
    }

    if (!info.microcode.empty())
    {
        std::cout << std::left << std::setw(labelWidth) << "Microcode" << info.microcode << "\n";
    }

    std::cout << std::endl;
    return std::error_code{};
}

std::error_code LocalMem(viper::app::ContextPtr ctx)
{
    (void)ctx;
    viper::host::MemInfo info;
    std::error_code      ec = viper::host::GetMemInfo(info);
    if (ec)
    {
        return ec;
    }

    const int labelWidth = 22;
    std::cout << "\n--- Memory ---\n";
    std::cout << std::left << std::setw(labelWidth) << "MemTotal" << viper::assist::FormatKb(info.memTotalKb) << "\n";
    std::cout << std::left << std::setw(labelWidth) << "MemFree" << viper::assist::FormatKb(info.memFreeKb) << "\n";
    std::cout << std::left << std::setw(labelWidth) << "MemAvailable" << viper::assist::FormatKb(info.memAvailableKb) << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Buffers" << viper::assist::FormatKb(info.buffersKb) << "\n";
    std::cout << std::left << std::setw(labelWidth) << "Cached" << viper::assist::FormatKb(info.cachedKb) << "\n";
    std::cout << std::left << std::setw(labelWidth) << "SwapTotal" << viper::assist::FormatKb(info.swapTotalKb) << "\n";
    std::cout << std::left << std::setw(labelWidth) << "SwapFree" << viper::assist::FormatKb(info.swapFreeKb) << "\n";
    std::cout << std::endl;
    return std::error_code{};
}

std::error_code LocalDisk(viper::app::ContextPtr ctx)
{
    (void)ctx;
    viper::host::DiskInfo info;
    std::error_code       ec = viper::host::GetDiskInfo(info);
    if (ec)
    {
        return ec;
    }

    const int labelWidth = 22;
    std::cout << "\n--- Disk ---\n";
    for (const auto& m : info.mounts)
    {
        std::cout << std::left << std::setw(labelWidth) << (m.device + ":") << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  Mount point" << m.mountPoint << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  Fstype" << m.fstype << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  Total" << viper::assist::FormatKb(m.totalBytes / 1024) << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  Free" << viper::assist::FormatKb(m.freeBytes / 1024) << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  Available" << viper::assist::FormatKb(m.availBytes / 1024) << "\n";
    }
    std::cout << std::endl;
    return std::error_code{};
}

std::error_code LocalNet(viper::app::ContextPtr ctx)
{
    (void)ctx;
    viper::host::NetInfo info;
    std::error_code      ec = viper::host::GetNetInfo(info);
    if (ec)
    {
        return ec;
    }

    const int labelWidth = 22;
    std::cout << "\n--- Network ---\n";
    for (const auto& iface : info.interfaces)
    {
        std::cout << std::left << std::setw(labelWidth) << (iface.name + ":") << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  RX bytes" << viper::assist::FormatKb(iface.rxBytes / 1024) << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  RX packets" << iface.rxPackets << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  TX bytes" << viper::assist::FormatKb(iface.txBytes / 1024) << "\n";
        std::cout << std::left << std::setw(labelWidth) << "  TX packets" << iface.txPackets << "\n";
    }
    std::cout << std::endl;
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
