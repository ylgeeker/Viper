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

#include "core/app/config_file.h"
#include "core/assist/macro.h"
#include "core/error/error.h"
#include "core/log/log.h"

#include <cstddef>
#include <iostream>
#include <string>

#include <yaml-cpp/exceptions.h>
#include <yaml-cpp/node/parse.h>

namespace viper {
namespace app {

std::error_code ConfigFile::Load(const std::string& filePath)
{
    BEGIN_TRY

    _fileName   = filePath;
    auto config = YAML::LoadFile(filePath);

    for (const auto& entry : config)
    {
        auto key = entry.first.as<std::string>();
        Traverse(entry.second, key);
    }

    END_TRY_BEGIN_CATCH(YAML::ParserException, ex)

    LOG_ERROR("failed to parse the yaml file. file({}), errmsg({})", filePath, ex.what());
    return error::ErrorCode::APP_CONFIGURATION_INVALID;

    END_TRY_BEGIN_CATCH(YAML::BadFile, ex)

    LOG_ERROR("failed to parse the yaml file. file({}), errmsg({})", filePath, ex.what());
    return error::ErrorCode::APP_CONFIGURATION_INVALID;

    END_CATCH

    return error::ErrorCode::SUCCESS;
}

void ConfigFile::PrintVars()
{
    for (const auto& iter : _keyValues)
    {
        LOG_STD("key(%s), value(%s)", iter.first.c_str(), iter.second.c_str());
    }
}

std::string ConfigFile::Get(const std::string& key)
{
    auto iter = _keyValues.find(key);

    if (iter != _keyValues.end())
    {
        return iter->second;
    }

    return "";
}

bool ConfigFile::Exist(const std::string& key)
{
    auto iter = _keyValues.find(key);
    if (iter != _keyValues.end())
    {
        return true;
    }
    return false;
}

void ConfigFile::Traverse(const YAML::Node& node, const std::string& key)
{
    if (node.IsMap())
    {
        for (const auto& entry : node)
        {
            auto keyName    = entry.first.as<std::string>();
            auto newKeyName = key.empty() ? key : key + "." + keyName;
            Traverse(entry.second, newKeyName);
        }
    }
    else if (node.IsSequence())
    {
        for (std::size_t idx = 0; idx < node.size(); ++idx)
        {
            auto keyName = key + ".[" + std::to_string(idx) + "]";
            Traverse(node[idx], keyName);
        }
    }
    else if (node.IsScalar())
    {
        _keyValues[key] = node.as<std::string>();
    }
    else if (node.IsNull())
    {
        _keyValues[key] = "";
    }
    else
    {
        std::cout << "unknown key: -- " << key << std::endl;
        _keyValues[key] = node.as<std::string>();
    }
}

} // namespace app
} // namespace viper

