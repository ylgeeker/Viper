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

#include "core/host/net.h"

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

} // namespace

std::error_code GetNetInfo(NetInfo& out)
{
    const char*   netdevPath = "/proc/net/dev";
    std::ifstream netdev(netdevPath);
    if (!netdev.is_open())
    {
        return std::make_error_code(std::io_errc::stream);
    }

    out.interfaces.clear();
    std::string line;
    int         lineNo = 0;
    while (std::getline(netdev, line))
    {
        ++lineNo;
        if (lineNo <= 2)
        {
            continue;
        }
        std::string::size_type colon = line.find(':');
        if (colon == std::string::npos)
        {
            continue;
        }
        std::string ifName = line.substr(0, colon);
        std::string rest   = line.substr(colon + 1);
        Trim(ifName);
        Trim(rest);
        if (ifName.empty())
        {
            continue;
        }

        std::istringstream iss(rest);
        uint64_t           n[16] = {};
        for (int i = 0; i < 16 && iss >> n[i]; ++i)
        {
        }
        InterfaceInfo info;
        info.name      = ifName;
        info.rxBytes   = n[0];
        info.rxPackets = n[1];
        info.txBytes   = n[8];
        info.txPackets = n[9];
        out.interfaces.push_back(info);
    }

    return std::error_code{};
}

} // namespace host
} // namespace viper
