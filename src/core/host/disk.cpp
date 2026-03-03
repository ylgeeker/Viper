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

#include "core/host/disk.h"

#include <ios>
#include <mntent.h>
#include <sys/statvfs.h>
#include <system_error>

#include <string>

namespace viper {
namespace host {

std::error_code GetDiskInfo(DiskInfo& out)
{
    out.mounts.clear();
    FILE* fp = setmntent("/proc/mounts", "r");
    if (!fp)
    {
        return std::make_error_code(std::io_errc::stream);
    }

    struct mntent* ent;
    while ((ent = getmntent(fp)) != nullptr)
    {
        if (!ent->mnt_fsname || !ent->mnt_dir)
        {
            continue;
        }
        std::string device(ent->mnt_fsname);
        if (device.compare(0, 5, "/dev/") != 0)
        {
            continue;
        }

        struct statvfs vfs;
        if (statvfs(ent->mnt_dir, &vfs) != 0)
        {
            continue;
        }
        uint64_t frsize = static_cast<uint64_t>(vfs.f_frsize);
        uint64_t total  = static_cast<uint64_t>(vfs.f_blocks) * frsize;
        uint64_t freeB  = static_cast<uint64_t>(vfs.f_bfree) * frsize;
        uint64_t avail  = static_cast<uint64_t>(vfs.f_bavail) * frsize;

        MountInfo info;
        info.device     = device;
        info.mountPoint = ent->mnt_dir;
        info.fstype     = ent->mnt_type ? ent->mnt_type : "";
        info.totalBytes = total;
        info.freeBytes  = freeB;
        info.availBytes = avail;
        out.mounts.push_back(info);
    }
    endmntent(fp);
    return std::error_code{};
}

} // namespace host
} // namespace viper
