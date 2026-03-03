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

#ifndef _VIPER_CORE_HOST_NET_H_
#define _VIPER_CORE_HOST_NET_H_

#include <cstdint>
#include <string>
#include <system_error>
#include <vector>

namespace viper {
namespace host {

struct InterfaceInfo
{
    std::string name;
    uint64_t    rxBytes   = 0;
    uint64_t    rxPackets = 0;
    uint64_t    txBytes   = 0;
    uint64_t    txPackets = 0;
};

struct NetInfo
{
    std::vector<InterfaceInfo> interfaces;
};

std::error_code GetNetInfo(NetInfo& out);

} // namespace host
} // namespace viper

#endif
