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

#ifndef _VIPER_AGENT_APP_H_
#define _VIPER_AGENT_APP_H_

#include "viper/configuration.h"

#include "internal/error.h"

#include "core/app/core.h"

class App final
{
public:
    App();
    ~App();

public:
    viper::internal::ErrorCode Run(int argc, char *argv[]);
    void                     Close();

private:
    viper::internal::ErrorCode GuardLoop();
    void                     DumpConfiguration();
    viper::internal::ErrorCode InitFlags();
    viper::internal::ErrorCode InitLogs();
    viper::internal::ErrorCode InitController();
    viper::internal::ErrorCode LoadConfig(viper::app::ContextPtr ctx);
    std::error_code          Execute(viper::app::ContextPtr ctx);

private:
    ConfigurationPtr  _localConfig;
    viper::app::CorePtr _core;
    std::atomic_bool  _needStop = false;
};

#endif

