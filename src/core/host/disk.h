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

#ifndef _VIPER_CORE_HOST_DISK_H_
#define _VIPER_CORE_HOST_DISK_H_

#include <cstdint>
#include <string>
#include <system_error>
#include <vector>

namespace viper {
namespace host {

struct MountInfo
{
    std::string device;
    std::string mountPoint;
    std::string fstype;
    uint64_t    totalBytes = 0;
    uint64_t    freeBytes  = 0;
    uint64_t    availBytes = 0;
};

struct DiskInfo
{
    std::vector<MountInfo> mounts;
};

std::error_code GetDiskInfo(DiskInfo& out);

} // namespace host
} // namespace viper

#endif
