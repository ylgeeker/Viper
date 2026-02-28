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

#ifndef _VIPER_CORE_APP_CONFIG_FILE_H_
#define _VIPER_CORE_APP_CONFIG_FILE_H_

#include <map>
#include <memory>
#include <string>
#include <system_error>

#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

namespace viper {
namespace app {

class ConfigFile final
{
public:
    std::error_code Load(const std::string& filePath);
    void            PrintVars();

public:
    std::string Get(const std::string& key);
    bool Exist(const std::string& key);

private:
    void Traverse(const YAML::Node& node, const std::string& key);

private:
    std::string                        _fileName;
    std::map<std::string, std::string> _keyValues;
};

using ConfigFilePtr = std::shared_ptr<ConfigFile>;

} // namespace app
} // namespace viper

#endif
