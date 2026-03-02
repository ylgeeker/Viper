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

// local header file
#include "viper/app.h"
#include "viper/cmds.h"

#include "internal/error.h"

// third party header file
#include "core/app/core.h"
#include "core/assist/time.h"
#include "core/core.h"
#include "core/error/error.h"
#include "core/log/log.h"
#include "core/option/value.h"
#include "core/text/strings.h"

// c++ standard header file
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

App::App()
{
    _core        = std::make_shared<viper::app::Core>();
    _localConfig = std::make_shared<Configuration>();
}

App::~App()
{
    Close();
}

viper::internal::ErrorCode App::Run(int argc, char* argv[])
{
    auto ec = viper::Init();
    if (!viper::error::IsSuccess(ec))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    _needStop = false;

    ec = InitFlags();
    if (!viper::error::IsSuccess(ec))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    ec = InitCommands();
    if (!viper::error::IsSuccess(ec))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    ec = InitInteractiveCommands();
    if (!viper::error::IsSuccess(ec))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    ec = _core->Run(argc, argv, std::bind(&App::Execute, this, std::placeholders::_1));
    if (!viper::error::IsSuccess(ec))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    return viper::internal::ErrorCode::SUCCESS;
}

void App::Close()
{
    _needStop = true;
    _core->Close();
    viper::Uninit();
}

viper::internal::ErrorCode App::GuardLoop()
{
    while (!_needStop)
    {
        viper::assist::MilliSleep(1000);
    }

    LOG_INFO(VIPER_MSG_EXITED);
    return viper::internal::ErrorCode::SUCCESS;
}

void App::DumpConfiguration()
{
    LOG_INFO("{}", DumpConfig(*_localConfig));
}

viper::internal::ErrorCode App::InitFlags()
{
    _core->AddFlag("config", 'c', "the config file", viper::option::Value(""));
    _core->AddFlag("interactive", 'i', "run in interactive mode", viper::option::Value(false));
    return viper::internal::ErrorCode::SUCCESS;
}

viper::internal::ErrorCode App::InitLogs()
{
    viper::log::LogConfig cfg;

    cfg._fileCount  = _localConfig->_maxFileCount;
    cfg._fileSizeMB = _localConfig->_maxFileSizeMB;
    cfg._path       = _localConfig->_logPath;
    cfg._level      = _localConfig->_logLevel;

    auto ec = viper::log::Logger::Instance().Init(_localConfig->_name, cfg);
    if (!viper::error::IsSuccess(ec))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    return viper::internal::ErrorCode::SUCCESS;
}

viper::internal::ErrorCode App::InitCommands()
{
    return RegisterCommands(_core, _localConfig);
}

viper::internal::ErrorCode App::InitInteractiveCommands()
{
    return RegisterInteractiveCommands(_core, _localConfig);
}

viper::internal::ErrorCode App::LoadConfig(viper::app::ContextPtr ctx)
{
    _localConfig->_ctx = ctx;

    if (!ctx->FlagExist("config"))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    std::string configPath = ctx->GetFlagValue<std::string>("config");
    std::cout << "config:" << configPath << std::endl;

    auto ec = ctx->LoadConfig(configPath);
    if (!viper::internal::IsSuccess(ec))
    {
        LOG_STD("can not load config file: %s", configPath.c_str());
        return viper::internal::ErrorCode::ERROR;
    }

    // parse base
    _localConfig->_name    = ctx->GetFileConfig<std::string>("name");
    _localConfig->_version = ctx->GetFileConfig<std::string>("version");

    // parse log
    _localConfig->_logLevel      = ctx->GetFileConfig<std::string>("log.level", VIPER_LOG_LEVEL_DFT);
    _localConfig->_logPath       = ctx->GetFileConfig<std::string>("log.path", VIPER_LOG_PATH_DFT);
    _localConfig->_maxFileCount  = ctx->GetFileConfig<uint32_t>("log.file-count", VIPER_LOG_FILE_MAX_COUNT_DFT);
    _localConfig->_maxFileSizeMB = ctx->GetFileConfig<uint32_t>("log.file-sizeMB", VIPER_LOG_FILE_MAX_FILE_SIZE_MB_DFT);

    return viper::internal::ErrorCode::SUCCESS;
}

std::error_code App::Execute(viper::app::ContextPtr ctx)
{
    _localConfig->_ctx = ctx;

    const bool interactive = ctx->FlagExist("interactive") && ctx->GetFlagValue<bool>("interactive");
    const bool noConfig    = !ctx->FlagExist("config") || ctx->GetFlagValue<std::string>("config").empty();

    if (interactive && noConfig)
    {
        _localConfig->_name    = VIPER_APP_NAME;
        _localConfig->_version = "interactive";
        return RunInteractiveLoop();
    }

    auto ec = LoadConfig(ctx);
    if (!viper::internal::IsSuccess(ec))
    {
        return ec;
    }

    ec = InitLogs();
    if (!viper::internal::IsSuccess(ec))
    {
        return ec;
    }

    DumpConfiguration();

    if (interactive)
    {
        return RunInteractiveLoop();
    }
    return GuardLoop();
}

viper::internal::ErrorCode App::RunInteractiveLoop()
{
    std::string              line;
    std::vector<std::string> argvStorage;
    std::vector<char*>       argvPtrs;
    std::vector<std::string> contextStack;

    while (true)
    {
        // Prompt format: Viper> or Viper/show> or Viper/show/foo>
        std::string prompt = VIPER_APP_NAME;
        for (const auto& c : contextStack)
        {
            prompt += "/";
            prompt += c;
        }
        prompt += "> ";
        std::cout << prompt << std::flush;

        if (!std::getline(std::cin, line))
        {
            break;
        }

        std::istringstream iss(line);
        std::string        token;
        argvStorage.clear();
        argvStorage.push_back(VIPER_APP_NAME);
        while (iss >> token)
        {
            argvStorage.push_back(token);
        }

        if (argvStorage.size() == 1)
        {
            continue;
        }

        const std::string& first      = argvStorage[1];
        const std::string  firstLower = viper::text::ToLower(first);
        if (firstLower == "exit" || firstLower == "quit")
        {
            break;
        }

        // Resolve current context (interactive root, or follow contextStack)
        auto current = _core->GetInteractiveRoot();
        for (const auto& name : contextStack)
        {
            current = current->GetSubcommand(name);
            if (!current)
            {
                break;
            }
        }

        if (firstLower == "use")
        {
            if (argvStorage.size() < 3)
            {
                std::cout << "use <context>" << std::endl;
                continue;
            }
            const std::string ctxName = argvStorage[2];
            auto              sub     = current ? current->GetSubcommand(ctxName) : nullptr;
            if (!sub)
            {
                std::cout << "unknown context: " << ctxName << std::endl;
                continue;
            }
            if (!sub->HasSubcommands())
            {
                std::cout << "context has no subcommands: " << ctxName << std::endl;
                continue;
            }
            contextStack.push_back(ctxName);
            continue;
        }

        if (firstLower == "back")
        {
            if (contextStack.empty())
            {
                std::cout << "already at root" << std::endl;
                continue;
            }
            contextStack.pop_back();
            continue;
        }

        // Build full argv: [program, ...contextStack, ...user tokens]
        std::vector<std::string> fullArgs;
        fullArgs.push_back(VIPER_APP_NAME);
        for (const auto& c : contextStack)
        {
            fullArgs.push_back(c);
        }
        for (size_t i = 1; i < argvStorage.size(); ++i)
        {
            fullArgs.push_back(argvStorage[i]);
        }

        argvPtrs.clear();
        for (auto& s : fullArgs)
        {
            argvPtrs.push_back(s.data());
        }
        (void)_core->ExecuteArgs(static_cast<int>(argvPtrs.size()), argvPtrs.data());
    }

    std::cout << VIPER_MSG_INTERACTIVE_EXITED << std::endl;
    return viper::internal::ErrorCode::SUCCESS;
}
