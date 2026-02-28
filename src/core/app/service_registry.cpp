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

#include "core/app/service_registry.h"
#include "core/assist/macro.h"
#include "core/assist/math.h"
#include "core/assist/time.h"
#include "core/error/error.h"
#include "core/log/log.h"

#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>

#include <exception>
#include <memory>
#include <string>
#include <system_error>

namespace viper {
namespace app {

ServiceRegistry::ServiceRegistry(const std::string& serviceName, const std::string& etcdURLs,
                                 const std::string& user, const std::string& password, int ttl)
{
    _serviceName = serviceName;
    _etcdURLs    = etcdURLs;
    _user        = user;
    _password    = password;
    _instanceID  = assist::UUID();
    _rootKey     = serviceName + "/" + _instanceID;

    if (ttl < VIPER_CORE_APP_SERVICE_REGISTRY_TTL_DFT)
    {
        ttl = VIPER_CORE_APP_SERVICE_REGISTRY_TTL_DFT;
    }

    _ttl = ttl;
}

EtcdClientPtr ServiceRegistry::EtcdClient()
{
    return _client;
}

std::error_code ServiceRegistry::Run(const std::string& rootKeyValue)
{
    if (_client == nullptr)
    {
        CreateEtcdClient();
    }

    _rootKeyValue.store(std::make_shared<std::string>(rootKeyValue));
    auto ec = DoRegister(_rootKey, rootKeyValue, VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT);
    if (!viper::error::IsSuccess(ec))
    {
        LOG_WARN("can not register service. name:{}", _serviceName);
        return ec;
    }

    _keepRunning        = true;
    _checkHealthyThread = std::thread([this]() {
        while (_keepRunning)
        {
            auto value = _rootKeyValue.load();
            CheckHealthy(_rootKey, *value);
            assist::MilliSleep(50);
        }
    });

    return viper::error::ErrorCode::SUCCESS;
}

std::error_code ServiceRegistry::Set(const std::string& key, const std::string& value, int retryMax)
{
    if (retryMax < VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT)
    {
        retryMax = VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT;
    }

    auto _value = _rootKeyValue.load();
    if (key == _rootKey && *_value == value)
    {
        return viper::error::ErrorCode::SUCCESS;
    }

    if (key == _rootKey && *_value != value)
    {
        _rootKeyValue.store(std::make_shared<std::string>(value));
    }

    while (--retryMax > 0)
    {
        auto resp = _client->set(key, value, _leaseID).get();
        if (!resp.is_ok())
        {
            assist::MilliSleep(50);
            continue;
        }

        return viper::error::ErrorCode::SUCCESS;
    }

    return viper::error::ErrorCode::ERROR;
}

std::string ServiceRegistry::GetID()
{
    return _instanceID;
}

std::string ServiceRegistry::GetRootKey()
{
    return _rootKey;
}

void ServiceRegistry::Close()
{
    _keepRunning = false;

    if (_keepalive)
    {
        _keepalive->Cancel();
    }

    if (_checkHealthyThread.joinable())
    {
        _checkHealthyThread.join();
    }

    _client->rm(_serviceName).wait();
}

void ServiceRegistry::CreateEtcdClient()
{
    if (_client == nullptr)
    {
        _client = std::make_shared<etcd::Client>(_etcdURLs, _user, _password);
    }
}

std::error_code ServiceRegistry::DoRegister(const std::string& key, const std::string& value, int retryMax)
{
    if (retryMax < VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT)
    {
        retryMax = VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT;
    }

    etcd::Response resp = _client->leasegrant(_ttl).get();
    if (!resp.is_ok())
    {
        LOG_WARN("service registry, lease grant failed. key:{}, errmsg:{}", key, resp.error_message());
        return viper::error::ErrorCode::APP_DISCOVERY_CREATE_LEASE_FAILED;
    }

    _leaseID   = resp.value().lease();
    _keepalive = std::make_shared<etcd::KeepAlive>(*_client, _ttl, _leaseID);

    while (--retryMax > 0)
    {
        auto resp = _client->set(key, value, _leaseID).get();
        if (!resp.is_ok())
        {
            assist::MilliSleep(50);
            continue;
        }

        return viper::error::ErrorCode::SUCCESS;
    }

    return viper::error::ErrorCode::APP_DISCOVERY_REGISTER_SERVICE_FAILED;
}

void ServiceRegistry::CheckHealthy(const std::string& key, const std::string& value)
{
    BEGIN_TRY

    _keepalive->Check();

    END_TRY_BEGIN_CATCH(std::exception, e)

    LOG_WARN("service registry, keepalive is invalid, renew one. key:{}, ttl:{}, errmsg:{}", key, _ttl, e.what());

    _keepalive = std::make_shared<etcd::KeepAlive>(*_client, _ttl, _leaseID);

    END_CATCH

    if (CheckRegistrationActive(key, value))
    {
        return;
    }

    LOG_DEBUG("register service. name:{}", key);

    auto ec = DoRegister(key, value, VIPER_CORE_APP_SERVICE_REGISTRY_RETRY_MAX_DFT);
    if (!viper::error::IsSuccess(ec))
    {
        LOG_WARN("can not register service. name:{}", key);
    }
}

bool ServiceRegistry::CheckRegistrationActive(const std::string& key, const std::string& endpoint)
{
    static uint64_t lastCheck  = 0;
    bool            lastResult = true;

    auto now = assist::TimestampTickCountSecond();
    if (now - lastCheck < 5)
    {
        return lastResult;
    }

    BEGIN_TRY

    etcd::Response keyResp   = _client->get(key).get();
    etcd::Response leaseResp = _client->leasetimetolive(_leaseID).get();

    bool keyOk   = keyResp.is_ok() && (keyResp.value().as_string() == endpoint);
    bool leaseOk = leaseResp.is_ok() && (leaseResp.value().ttl() > 0);

    lastResult = keyOk && leaseOk;
    lastCheck  = now;

    LOG_DEBUG("check registration result. key:{}, value:{}, endpoint:{}, result:{}", key, keyResp.value().as_string(), endpoint, lastResult);

    END_TRY
    BEGIN_CATCH(std::exception, e)

    LOG_WARN("check registration failed. key:{}, endpoint:{}, errmsg:{}", key, endpoint, e.what());
    lastResult = false;

    END_CATCH

    return lastResult;
}

} // namespace app
} // namespace viper
