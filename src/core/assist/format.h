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

#ifndef _VIPER_CORE_ASSIST_FORMAT_H_
#define _VIPER_CORE_ASSIST_FORMAT_H_

#include <cstdint>
#include <string>

namespace viper {
namespace assist {

/**
 * @brief FormatKb format size in KiB to human-readable string (GiB / MiB / KiB).
 * @param kb size in kibibytes
 * @return formatted string e.g. "1.50 GiB", "512.00 MiB", "1024 KiB"
 */
std::string FormatKb(uint64_t kb);

} // namespace assist
} // namespace viper

#endif
