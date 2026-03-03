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

#include <termios.h>
#include <unistd.h>

std::string App::BuildInteractivePrompt(const std::vector<std::string>& contextStack) const
{
    std::string prompt = VIPER_APP_NAME;
    for (const auto& c : contextStack)
    {
        prompt += "/";
        prompt += c;
    }
    prompt += "> ";
    return prompt;
}

bool App::SetupRawTerminal(struct termios* saved) const
{
    if (saved == nullptr || tcgetattr(STDIN_FILENO, saved) != 0)
    {
        return false;
    }
    struct termios raw = *saved;
    raw.c_lflag &= static_cast<tcflag_t>(~ICANON);
    raw.c_lflag &= static_cast<tcflag_t>(~ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    return tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0;
}

void App::RestoreTerminal(const struct termios* saved) const
{
    if (saved != nullptr)
    {
        (void)tcsetattr(STDIN_FILENO, TCSANOW, saved);
    }
}

bool App::ReadInteractiveLine(const std::string& prompt, std::string& lineOut) const
{
    lineOut.clear();
    while (true)
    {
        char ch;
        if (read(STDIN_FILENO, &ch, 1) != 1)
        {
            return false; /* EOF */
        }

        const unsigned char u = static_cast<unsigned char>(ch);
        if (u == 0x0c) /* Ctrl+L */
        {
            std::cout << "\033[2J\033[H" << std::flush;
            std::cout << prompt << std::flush;
            lineOut.clear();
            continue;
        }

        if (ch == '\n' || ch == '\r')
        {
            std::cout << std::endl;
            return true;
        }

        if (u == 127 || u == 8) /* backspace / DEL */
        {
            if (!lineOut.empty())
            {
                lineOut.pop_back();
                std::cout << "\b \b" << std::flush;
            }
            continue;
        }
        
        lineOut += ch;
        std::cout << ch << std::flush;
    }
}

std::vector<std::string> App::ParseLineToTokens(const std::string& line) const
{
    std::vector<std::string> tokens;
    tokens.push_back(VIPER_APP_NAME);
    std::istringstream iss(line);
    std::string        token;
    while (iss >> token)
    {
        tokens.push_back(token);
    }
    return tokens;
}

App::InteractiveResult App::ProcessBuiltinCommands(const std::vector<std::string>& tokens,
                                                  std::vector<std::string>&       contextStack)
{
    if (tokens.size() < 2)
    {
        return InteractiveResult::Continue;
    }

    const std::string firstLower = viper::text::ToLower(tokens[1]);
    if (firstLower == "exit" || firstLower == "quit")
    {
        return InteractiveResult::Exit;
    }

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
        if (tokens.size() < 3)
        {
            std::cout << "use <context>" << std::endl;
            return InteractiveResult::Continue;
        }
        const std::string ctxName = tokens[2];
        auto              sub     = current ? current->GetSubcommand(ctxName) : nullptr;
        if (!sub)
        {
            std::cout << "unknown context: " << ctxName << std::endl;
            return InteractiveResult::Continue;
        }
        if (!sub->HasSubcommands())
        {
            std::cout << "context has no subcommands: " << ctxName << std::endl;
            return InteractiveResult::Continue;
        }
        contextStack.push_back(ctxName);
        return InteractiveResult::Continue;
    }

    if (firstLower == "back")
    {
        if (contextStack.empty())
        {
            std::cout << "already at root" << std::endl;
            return InteractiveResult::Continue;
        }
        contextStack.pop_back();
        return InteractiveResult::Continue;
    }

    if (firstLower == "clear")
    {
        std::cout << "\033[2J\033[H" << std::flush;
        return InteractiveResult::Continue;
    }

    return InteractiveResult::Execute;
}

void App::ExecuteInteractiveCommand(const std::vector<std::string>& contextStack,
                                    const std::vector<std::string>& tokens)
{
    std::vector<std::string> fullArgs;
    fullArgs.push_back(VIPER_APP_NAME);
    for (const auto& c : contextStack)
    {
        fullArgs.push_back(c);
    }
    for (size_t i = 1; i < tokens.size(); ++i)
    {
        fullArgs.push_back(tokens[i]);
    }
    std::vector<char*> argvPtrs;
    for (auto& s : fullArgs)
    {
        argvPtrs.push_back(s.data());
    }
    (void)_core->ExecuteArgs(static_cast<int>(argvPtrs.size()), argvPtrs.data());
}

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
    struct termios savedTerm;
    if (!SetupRawTerminal(&savedTerm))
    {
        return viper::internal::ErrorCode::ERROR;
    }

    std::vector<std::string> contextStack;
    while (true)
    {
        const std::string prompt = BuildInteractivePrompt(contextStack);
        std::cout << prompt << std::flush;

        std::string line;
        if (!ReadInteractiveLine(prompt, line))
        {
            RestoreTerminal(&savedTerm);
            break;
        }

        const std::vector<std::string> tokens = ParseLineToTokens(line);
        const InteractiveResult        result = ProcessBuiltinCommands(tokens, contextStack);

        if (result == InteractiveResult::Exit)
        {
            RestoreTerminal(&savedTerm);
            break;
        }
        if (result == InteractiveResult::Continue)
        {
            continue;
        }

        ExecuteInteractiveCommand(contextStack, tokens);
    }

    RestoreTerminal(&savedTerm);
    std::cout << VIPER_MSG_INTERACTIVE_EXITED << std::endl;
    return viper::internal::ErrorCode::SUCCESS;
}
