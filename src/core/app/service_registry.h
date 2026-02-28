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

#ifndef _VIPER_CORE_APP_SERVICE_REGISTRY_H_
#define _VIPER_CORE_APP_SERVICE_REGISTRY_H_

#include "core/error/error.h"

#include <etcd/Client.hpp>
#include <etcd/Response.hpp>
#include <etcd/Watcher.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <system_error>

// clang-format off

#define VIPER_CORE_APP_SERVICE_REGISTRY_TTL_DFT       30
#define VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT 3

// clang-format on

namespace viper {
namespace app {

using EtcdClientPtr = std::shared_ptr<etcd::Client>;
using RootKeyValue  = std::atomic<std::shared_ptr<std::string>>;

class ServiceRegistry final
{
public:
    ServiceRegistry(const std::string& serviceName, const std::string& etcdURLs,
                    const std::string& user, const std::string& password, int ttl = 0);

    ~ServiceRegistry() = default;

public:
    EtcdClientPtr   EtcdClient();
    std::error_code Run(const std::string& rootKeyValue);
    std::error_code Set(const std::string& key, const std::string& value, int retryMax = 0);
    std::string     GetID();
    std::string     GetRootKey();
    void            Close();

private:
    void            CreateEtcdClient();
    std::error_code DoRegister(const std::string& key, const std::string& value, int retryMax = 0);
    void            CheckHealthy(const std::string& key, const std::string& value);
    bool            CheckRegistrationActive(const std::string& key, const std::string& endpoint);

private:
    // eg: http://127.0.0.1:2379;http://127.0.0.1:2379
    std::string                      _etcdURLs;
    std::string                      _user;
    std::string                      _password;
    std::string                      _serviceName;
    std::string                      _instanceID;
    std::string                      _rootKey;
    RootKeyValue                     _rootKeyValue;
    std::shared_ptr<etcd::Client>    _client;
    int                              _ttl     = VIPER_CORE_APP_SERVICE_REGISTRY_TTL_DFT;
    int64_t                          _leaseID = 0;
    std::shared_ptr<etcd::KeepAlive> _keepalive;
    std::thread                      _checkHealthyThread;
    std::atomic_bool                 _keepRunning = true;
};

using ServiceRegistryPtr = std::shared_ptr<ServiceRegistry>;

} // namespace app
} // namespace viper

#endif

