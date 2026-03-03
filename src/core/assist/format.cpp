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

#include "core/assist/format.h"

#include <iomanip>
#include <sstream>

namespace viper {
namespace assist {

std::string FormatKb(uint64_t kb)
{
    double             gb = static_cast<double>(kb) / (1024.0 * 1024.0);
    double             mb = static_cast<double>(kb) / 1024.0;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    if (gb >= 1.0)
    {
        oss << gb << " GiB";
    }
    else if (mb >= 1.0)
    {
        oss << mb << " MiB";
    }
    else
    {
        oss << kb << " KiB";
    }
    return oss.str();
}

} // namespace assist
} // namespace viper
