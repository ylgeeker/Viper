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

#ifndef _VIPER_APP_H_
#define _VIPER_APP_H_

// local header file
#include "viper/configuration.h"

#include "internal/error.h"

// third party header file
#include "core/app/core.h"

// c++ standard header file
#include <string>
#include <vector>

#include <termios.h>

class App final
{
public:
    App();
    ~App();

public:
    viper::internal::ErrorCode Run(int argc, char *argv[]);
    void                       Close();

private:
    enum class InteractiveResult
    {
        Continue,
        Exit,
        Execute,
    };

    viper::internal::ErrorCode GuardLoop();
    viper::internal::ErrorCode RunInteractiveLoop();
    void                       DumpConfiguration();
    viper::internal::ErrorCode InitFlags();
    viper::internal::ErrorCode InitCommands();
    viper::internal::ErrorCode InitInteractiveCommands();
    viper::internal::ErrorCode InitLogs();
    viper::internal::ErrorCode InitController();
    viper::internal::ErrorCode LoadConfig(viper::app::ContextPtr ctx);
    std::error_code            Execute(viper::app::ContextPtr ctx);

    std::string BuildInteractivePrompt(const std::vector<std::string>& contextStack) const;
    bool        SetupRawTerminal(struct termios* saved) const;
    void        RestoreTerminal(const struct termios* saved) const;
    bool        ReadInteractiveLine(const std::string& prompt, std::string& lineOut) const;
    std::vector<std::string> ParseLineToTokens(const std::string& line) const;
    InteractiveResult ProcessBuiltinCommands(const std::vector<std::string>& tokens,
                                            std::vector<std::string>&       contextStack);
    void ExecuteInteractiveCommand(const std::vector<std::string>& contextStack,
                                   const std::vector<std::string>& tokens);

private:
    ConfigurationPtr    _localConfig;
    viper::app::CorePtr _core;
    std::atomic_bool    _needStop = false;
};

#endif
