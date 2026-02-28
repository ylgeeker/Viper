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

#ifndef _VIPER_CORE_APP_SERVICE_DISCOVERY_H_
#define _VIPER_CORE_APP_SERVICE_DISCOVERY_H_

#include "core/container/safe_map.h"
#include "core/error/error.h"

#include <etcd/Client.hpp>
#include <etcd/Response.hpp>
#include <etcd/Watcher.hpp>

#include <memory>
#include <string>
#include <vector>

namespace viper {
namespace app {

enum class EventType : int32_t
{
    PUT,
    DELETE,
    UNKNOWN,
};

using EventHandler = std::function<void(const std::string& key, const std::string& value, EventType type)>;

struct WatcherHandler
{
    std::string                    _key;
    EventHandler                   _handler;
    bool                           _recursive = true;
    std::shared_ptr<etcd::Watcher> _watcher;
};

using WatcherHandlerPtr = std::shared_ptr<WatcherHandler>;

class ServiceDiscovery
{
public:
    ServiceDiscovery(const std::string& etcdURLs, const std::string& user, const std::string& password);
    ServiceDiscovery(std::shared_ptr<etcd::Client> etcdClient);
    ~ServiceDiscovery();

public:
    std::error_code List(const std::string& key, std::vector<std::string>& values);
    std::error_code OpenWatcher(const std::string& key, EventHandler handler, bool recursive = true);
    void            CloseWatcher(const std::string& key);

private:
    void CreateEtcdClient();
    void HandleWatchResponse(etcd::Response response);

private:
    // eg: http://127.0.0.1:2379;http://127.0.0.1:2379
    std::string                   _etcdURLs;
    std::string                   _user;
    std::string                   _password;
    std::shared_ptr<etcd::Client> _client;

    // Key: service name, Value: watcher
    container::SafeMap<std::string, WatcherHandlerPtr> _watcherHandlers;
};

using ServiceDiscoveryPtr = std::shared_ptr<ServiceDiscovery>;

} // namespace app
} // namespace viper

#endif

