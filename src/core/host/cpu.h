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

#ifndef _VIPER_CORE_HOST_CPU_H_
#define _VIPER_CORE_HOST_CPU_H_

#include <string>
#include <system_error>

namespace viper {
namespace host {

struct CpuInfo
{
    std::string modelName;
    std::string vendor;
    int         logicalCount   = 0;
    int         threadsPerCore = 1;
    int         coresPerSocket = 0;
    int         sockets        = 1;
    std::string cacheSize;
    double      frequencyMhz = -1.0;
    std::string cpuFamily;
    std::string model;
    std::string stepping;
    std::string microcode;
};

std::error_code GetCpuInfo(CpuInfo& out);

} // namespace host
} // namespace viper

#endif
