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

#ifndef _VIPER_OPTION_COMMAND_H_
#define _VIPER_OPTION_COMMAND_H_

#include "core/option/args.h"
#include "core/option/flag.h"
#include "core/option/value.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace viper::option {

/// Signature used for command handlers. The callback receives parsed `Args`
/// and returns an `int` exit code (0 for success).
using RunFunc = std::function<int(const Args& args)>;

class Command final : public std::enable_shared_from_this<Command>
{
public:
    /// Default constructor.
    Command() = default;

    /// Destructor.
    ~Command();

    /// Register a flag with this command. Flags are typically created with
    /// `std::make_shared<Flag<T>>(...)` and then passed here.
    template <typename T>
    void AddFlag(std::shared_ptr<Flag<T>> flag) {}

    /// Add a child/sub-command under this command.
    /// Example: `root->AddCommand(testCmd);`
    void AddCommand(std::shared_ptr<Command> cmd);

    /// Print the short usage line for this command to stdout.
    void Usage();

    /// Print the detailed help message for this command.
    void Help();

    /// Parse `argv` and run the appropriate command/subcommand.
    /// Returns the exit code produced by the invoked command handler.
    int Execute(int argc, char* argv[]);

private:
    /// Find the flag using the flag name or flag short name.
    std::shared_ptr<Flag<Value>> FindFlag(std::string_view name);
    bool                         UpdateFlagValue(std::shared_ptr<Flag<Value>> flagVal, Args& args, const std::string& value);

    /// Print the help message for a subcommand.
    void Help(std::string_view cmdName);

    /// Check if the flag is empty.
    bool HasFlags();

public:
    /// `_use` is the one-line usage identifier (e.g., "test").
    std::string _use;

    /// `_short` is the short description shown in listings and `help` output.
    std::string _short;

    /// `_long` is the extended help text shown by `help <command>`.
    std::string _long;

    /// Command handler invoked after parsing. Should return exit code.
    RunFunc _run = nullptr;

private:
    // Formatting helpers used when printing usage/help (max column widths).
    int _flagMaxWith    = 0;
    int _commandMaxWith = 0;

    /// Registered flags for this command (stored as type-erased `Value` flags).
    std::vector<std::shared_ptr<Flag<Value>>> _flagVals;

    /// Parent command in the command hierarchy (nullptr for root).
    std::shared_ptr<Command> _parent = nullptr;

    /// Subcommands keyed by their `_use` name.
    std::map<std::string, std::shared_ptr<Command>> _subCmds;
};

template <>
void Command::AddFlag<Value>(std::shared_ptr<Flag<Value>> flag);

} // namespace viper::option

#endif
