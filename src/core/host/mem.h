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

#ifndef _VIPER_CORE_HOST_MEM_H_
#define _VIPER_CORE_HOST_MEM_H_

#include <cstdint>
#include <system_error>

namespace viper {
namespace host {

struct MemInfo
{
    uint64_t memTotalKb     = 0;
    uint64_t memFreeKb      = 0;
    uint64_t memAvailableKb = 0;
    uint64_t buffersKb      = 0;
    uint64_t cachedKb       = 0;
    uint64_t swapTotalKb    = 0;
    uint64_t swapFreeKb     = 0;
};

std::error_code GetMemInfo(MemInfo& out);

} // namespace host
} // namespace viper

#endif
