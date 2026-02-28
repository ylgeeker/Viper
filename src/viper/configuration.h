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

#ifndef _VIPER_AGENT_CONFIG_H_
#define _VIPER_AGENT_CONFIG_H_

#include "core/application/context.h"

#include <cstdint>
#include <string>

// clang-format off

#define VIPER_AGENT_LOG_LEVEL_DFT                 "debug"
#define VIPER_AGENT_LOG_PATH_DFT                  "/var/log/viper/"
#define VIPER_AGENT_LOG_FILE_MAX_COUNT_DFT        10
#define VIPER_AGENT_LOG_FILE_MAX_FILE_SIZE_MB_DFT 100

// clang-format on

struct Configuration
{
    std::string          _name;
    std::string          _version;
    std::string          _controllerRemoteIP;
    uint16_t             _controllerRemotePort = 0;
    std::string          _logPath              = VIPER_AGENT_LOG_PATH_DFT;
    std::string          _logLevel             = VIPER_AGENT_LOG_LEVEL_DFT;
    uint32_t             _maxFileCount         = VIPER_AGENT_LOG_FILE_MAX_COUNT_DFT;
    uint32_t             _maxFileSizeMB        = VIPER_AGENT_LOG_FILE_MAX_FILE_SIZE_MB_DFT;
    viper::app::ContextPtr _ctx;
};

static std::string DumpConfig(const Configuration& cfg)
{
    std::stringstream ss;
    ss << "\nname: " << cfg._name
       << "\nversion: " << cfg._version;

    ss << "\ncontroller endpoint:"
       << "\n\tip:" << cfg._controllerRemoteIP
       << "\n\tport:" << cfg._controllerRemotePort;

    ss << "\nlog.path: " << cfg._logPath
       << "\nlog.level: " << cfg._logLevel
       << "\nlog.file-count: " << cfg._maxFileCount
       << "\nlog.file-sizeMB: " << cfg._maxFileSizeMB;

    return ss.str();
}

using ConfigurationPtr = std::shared_ptr<Configuration>;

#endif
