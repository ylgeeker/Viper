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

#ifndef _VIPER_CORE_APP_CORE_H_
#define _VIPER_CORE_APP_CORE_H_

#include "core/app/context.h"
#include "core/app/env.h"
#include "core/option/command.h"
#include "core/option/flag.h"
#include "core/option/value.h"

#include <functional>
#include <memory>
#include <system_error>
#include <string>

namespace viper {
namespace app {

using CommandHandler = std::function<std::error_code(ContextPtr ctx)>;

class Core final
{
public:
    Core();
    ~Core();

public:
    /**
     * @brief AddFlag register a command-line flag with default value.
     */
    void AddFlag(const std::string& name, char shortName, const std::string& description, const option::Value& defaultValue);

    /**
     * @brief AddCommand register a subcommand (e.g. "run", "version").
     * @param name Subcommand name as used on CLI (e.g. "run")
     * @param shortDesc Short description for usage/help listing
     * @param handler Called when this subcommand is invoked; receives Context with parsed args.
     */
    void AddCommand(const std::string& name, const std::string& shortDesc, CommandHandler handler);

    /**
     * @brief AddCommand register a pre-built subcommand (may have nested subcommands/flags).
     * @param cmd Subcommand tree; caller must set _use, _short, and optionally _run (use GetContext() to SetArgs and run logic).
     */
    void AddCommand(std::shared_ptr<option::Command> cmd);

    /**
     * @brief AddInteractiveCommand register a subcommand for interactive mode only.
     */
    void AddInteractiveCommand(const std::string& name, const std::string& shortDesc, CommandHandler handler);

    /**
     * @brief AddInteractiveCommand register a pre-built subcommand for interactive mode only.
     */
    void AddInteractiveCommand(std::shared_ptr<option::Command> cmd);

    /**
     * @brief GetContext return the shared context (e.g. for use in custom command _run with SetArgs).
     */
    ContextPtr GetContext() const;

    /**
     * @brief GetInteractiveRoot return the root command for interactive mode (for context navigation).
     */
    std::shared_ptr<option::Command> GetInteractiveRoot() const;

    /**
     * @brief Run parse argv, fill context with parsed args, and call executor.
     *
     * @param argc the argv count
     * @param argv the all argv
     * @param executor the executor function, if it exited, the process will be exited
     * @return std::error_code Success for success, the others for error
     */
    std::error_code Run(int argc, char* argv[], CommandHandler executor);

    /**
     * @brief ExecuteArgs parse and execute a single command line (e.g. for interactive mode).
     * @param argc argument count
     * @param argv argument vector (argv[0] typically "viper")
     * @return exit code as from Command::Execute (0 for success)
     */
    int ExecuteArgs(int argc, char* argv[]);

    /**
     * @brief Close close the application
     */
    void Close();

private:
    ContextPtr                      _ctx;
    EnvPtr                          _env;
    ConfigFilePtr                   _file;
    std::shared_ptr<option::Command> _root;
    std::shared_ptr<option::Command> _interactiveRoot;
};

using CorePtr = std::shared_ptr<Core>;

} // namespace app
} // namespace viper

#endif
