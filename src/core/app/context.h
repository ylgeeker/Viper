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

#ifndef _VIPER_CORE_APP_CONTEXT_H_
#define _VIPER_CORE_APP_CONTEXT_H_

#include "core/app/config_file.h"
#include "core/app/flag.h"
#include "core/app/service_discovery.h"
#include "core/app/service_registry.h"
#include "core/container/safe_map.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>

namespace viper {
namespace app {

struct ContextFlag
{
    std::map<std::string, FlagInt>     _iFlags;
    std::map<std::string, FlagString>  _sFlags;
    std::map<std::string, FlagBool>    _bFlags;
    std::map<std::string, FlagNoValue> _nvFlags;
};

using CacheName      = std::string;
using ContextFlagPtr = std::shared_ptr<ContextFlag>;

class Context final
{
public:
    template <typename T>
    std::error_code Save(const CacheName& name, T* data);

    template <typename T>
    T* Get(const CacheName& name);

    template <typename T>
    T GetFlagValue(const std::string& name) { return T{}; };

    bool FlagExist(const std::string& name);

    template <typename T>
    T GetFileConfig(const std::string& name, T defaultValue = T{});

    bool                FileConfigExist(const std::string& name);
    std::error_code     LoadConfig(const std::string& fileName);
    void                SaveRegistry(ServiceRegistryPtr registry);
    void                SaveDiscovery(ServiceDiscoveryPtr discovery);
    ServiceRegistryPtr  GetRegistry();
    ServiceDiscoveryPtr GetDiscocery();

private:
    friend class Core;
    container::SafeMap<std::string, void*> _caches;
    ContextFlagPtr                         _flags;
    ConfigFilePtr                          _fileCfg;
    ServiceDiscoveryPtr                    _discovery;
    ServiceRegistryPtr                     _registry;
};

using ContextPtr = std::shared_ptr<Context>;

template <>
FlagInt Context::GetFlagValue<FlagInt>(const std::string& name);

template <>
FlagBool Context::GetFlagValue<FlagBool>(const std::string& name);

template <>
FlagString Context::GetFlagValue<FlagString>(const std::string& name);

template <>
FlagNoValue Context::GetFlagValue<FlagNoValue>(const std::string& name);

template <>
bool Context::GetFileConfig<bool>(const std::string& name, bool defaultValue);

template <>
float Context::GetFileConfig<float>(const std::string& name, float defaultValue);

template <>
double Context::GetFileConfig<double>(const std::string& name, double defaultValue);

template <>
std::string Context::GetFileConfig<std::string>(const std::string& name, std::string defaultValue);

template <>
int Context::GetFileConfig<int>(const std::string& name, int defaultValue);

template <>
int64_t Context::GetFileConfig<int64_t>(const std::string& name, int64_t defaultValue);

template <>
uint32_t Context::GetFileConfig<uint32_t>(const std::string& name, uint32_t defaultValue);

template <>
uint64_t Context::GetFileConfig<uint64_t>(const std::string& name, uint64_t defaultValue);

template <>
int16_t Context::GetFileConfig<int16_t>(const std::string& name, int16_t defaultValue);

template <>
uint16_t Context::GetFileConfig<uint16_t>(const std::string& name, uint16_t defaultValue);

} // namespace app
} // namespace viper

#endif
