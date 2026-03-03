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

#include "core/host/mem.h"

#include <fstream>
#include <sstream>
#include <string>

namespace viper {
namespace host {

namespace {

static void Trim(std::string& s)
{
    const char* ws = " \t";
    s.erase(0, s.find_first_not_of(ws));
    s.erase(s.find_last_not_of(ws) + 1);
}

static uint64_t ParseValueKb(const std::string& value)
{
    std::istringstream iss(value);
    uint64_t           num = 0;
    iss >> num;
    return num;
}

} // namespace

std::error_code GetMemInfo(MemInfo& out)
{
    const char*   meminfoPath = "/proc/meminfo";
    std::ifstream meminfo(meminfoPath);
    if (!meminfo.is_open())
    {
        return std::make_error_code(std::io_errc::stream);
    }

    std::string line;
    while (std::getline(meminfo, line))
    {
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }
        std::string key   = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        Trim(key);
        Trim(value);

        uint64_t kb = ParseValueKb(value);
        if (key == "MemTotal")
        {
            out.memTotalKb = kb;
        }
        else if (key == "MemFree")
        {
            out.memFreeKb = kb;
        }
        else if (key == "MemAvailable")
        {
            out.memAvailableKb = kb;
        }
        else if (key == "Buffers")
        {
            out.buffersKb = kb;
        }
        else if (key == "Cached")
        {
            out.cachedKb = kb;
        }
        else if (key == "SwapTotal")
        {
            out.swapTotalKb = kb;
        }
        else if (key == "SwapFree")
        {
            out.swapFreeKb = kb;
        }
    }

    return std::error_code{};
}

} // namespace host
} // namespace viper
