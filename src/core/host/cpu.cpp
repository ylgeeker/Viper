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

#include "core/host/cpu.h"
#include "core/assist/host.h"

#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace viper {
namespace host {

namespace {

static void TrimKeyValue(std::string& key, std::string& value)
{
    auto trim = [](std::string& s) {
        const char* ws = " \t";
        s.erase(0, s.find_first_not_of(ws));
        s.erase(s.find_last_not_of(ws) + 1);
    };
    trim(key);
    trim(value);
}

} // namespace

std::error_code GetCpuInfo(CpuInfo& out)
{
    const char*   cpuinfoPath = "/proc/cpuinfo";
    std::ifstream cpuinfo(cpuinfoPath);
    if (!cpuinfo.is_open())
    {
        return std::make_error_code(std::io_errc::stream);
    }

    std::string                        line;
    std::map<std::string, std::string> firstBlock;
    std::set<int>                      physicalIds;
    std::set<std::pair<int, int>>      physicalCorePairs;
    int                                logicalCount = 0;
    std::map<std::string, std::string> block;

    auto flushBlock = [&]() {
        if (block.empty())
        {
            return;
        }
        ++logicalCount;
        if (firstBlock.empty())
        {
            firstBlock = block;
        }
        int physId = 0;
        int coreId = 0;
        if (block.count("physical id"))
        {
            std::istringstream(block["physical id"]) >> physId;
            physicalIds.insert(physId);
        }
        if (block.count("core id"))
        {
            std::istringstream(block["core id"]) >> coreId;
            physicalCorePairs.insert({physId, coreId});
        }
        block.clear();
    };

    while (std::getline(cpuinfo, line))
    {
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos)
        {
            flushBlock();
            continue;
        }
        std::string key   = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        TrimKeyValue(key, value);
        block[key] = value;
    }
    flushBlock();

    int sockets    = static_cast<int>(physicalIds.empty() ? 1 : physicalIds.size());
    int totalCores = static_cast<int>(physicalCorePairs.size());
    if (totalCores == 0 && logicalCount > 0 && firstBlock.count("cpu cores"))
    {
        totalCores = std::stoi(firstBlock["cpu cores"]) * sockets;
    }
    if (totalCores == 0 && logicalCount > 0)
    {
        totalCores = logicalCount;
    }
    int coresPerSocket = (sockets > 0 && totalCores > 0) ? (totalCores / sockets) : 0;
    if (coresPerSocket == 0 && logicalCount > 0)
    {
        coresPerSocket = firstBlock.count("cpu cores") ? std::stoi(firstBlock["cpu cores"]) : 1;
    }
    int threadsPerCore = (totalCores > 0 && logicalCount > 0) ? (logicalCount / totalCores) : 1;

    out.modelName      = firstBlock.count("model name") ? firstBlock["model name"] : "n/a";
    out.vendor         = firstBlock.count("vendor_id") ? firstBlock["vendor_id"] : "n/a";
    out.logicalCount   = logicalCount;
    out.threadsPerCore = threadsPerCore;
    out.coresPerSocket = coresPerSocket;
    out.sockets        = sockets;
    out.cacheSize      = firstBlock.count("cache size") ? firstBlock["cache size"] : "n/a";
    out.frequencyMhz   = viper::assist::HostCPUFrequency();
    out.cpuFamily      = firstBlock.count("cpu family") ? firstBlock["cpu family"] : "";
    out.model          = firstBlock.count("model") ? firstBlock["model"] : "";
    out.stepping       = firstBlock.count("stepping") ? firstBlock["stepping"] : "";
    out.microcode      = firstBlock.count("microcode") ? firstBlock["microcode"] : "";

    return std::error_code{};
}

} // namespace host
} // namespace viper
