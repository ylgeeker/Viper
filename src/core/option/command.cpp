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

#include "core/option/command.h"
#include "core/option/value.h"
#include "core/text/strings.h"

#include <algorithm>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace viper::option {

Command::~Command()
{
    _subCmds.clear();
    _flagVals.clear();
}

template <typename T>
static void PrintFlags(const std::vector<std::shared_ptr<Flag<T>>>& flags)
{
    if (flags.empty())
    {
        return;
    }

    size_t maxFlagLen = 0;
    size_t maxTypeLen = 0;
    for (const auto& flag : flags)
    {
        size_t flagLen = (2 + 1 + 2 + 2 + flag->_shorthand.length() + flag->_name.length()); // "  -" + short + ", --" + name
        size_t typeLen = flag->_valueType.length();
        maxFlagLen     = std::max(maxFlagLen, flagLen);
        maxTypeLen     = std::max(maxTypeLen, typeLen);
    }

    const int         flagColumnWidth = static_cast<int>(maxFlagLen) + 2;
    const int         typeColumnWidth = static_cast<int>(maxTypeLen) + 2;
    const std::string shortIndent(2 + flagColumnWidth + 1 + typeColumnWidth + 1, ' ');

    for (const auto& flag : flags)
    {
        std::string flagStr = "  -" + flag->_shorthand + ", --" + flag->_name;
        std::cout << std::left << std::setw(flagColumnWidth) << flagStr << " "
                  << std::left << std::setw(typeColumnWidth) << flag->_valueType << " ";

        std::ostringstream oss;
        oss << flag->_usage << " (default: " << flag->_value << ")";
        std::vector<std::string> lines = text::Split(oss.str(), '\n');
        if (lines.empty())
        {
            std::cout << std::endl;
            continue;
        }
        std::cout << lines[0] << std::endl;
        for (size_t i = 1; i < lines.size(); ++i)
        {
            std::cout << shortIndent << lines[i] << std::endl;
        }
    }
}

static void PrintCommands(const std::map<std::string, std::shared_ptr<Command>>& cmds)
{
    if (cmds.empty())
    {
        return;
    }

    size_t maxNameLen = 0;
    for (const auto& cmd : cmds)
    {
        maxNameLen = std::max(maxNameLen, cmd.second->_use.length());
    }

    const int         nameColumnWidth = static_cast<int>(maxNameLen) + 2;
    const std::string indent(4 + nameColumnWidth, ' ');

    auto printOne = [&](const std::shared_ptr<Command>& c) {
        std::cout << "    " << std::left << std::setw(nameColumnWidth) << c->_use;
        std::vector<std::string> lines = text::Split(c->_short, '\n');
        if (lines.empty())
        {
            std::cout << std::endl;
            return;
        }
        std::cout << lines[0] << std::endl;
        for (size_t i = 1; i < lines.size(); ++i)
        {
            std::cout << indent << lines[i] << std::endl;
        }
    };

    auto itHelp = cmds.find("help");
    for (const auto& cmd : cmds)
    {
        if (cmd.first == "help")
        {
            continue;
        }
        printOne(cmd.second);
    }

    if (itHelp != cmds.end())
    {
        printOne(itHelp->second);
    }
}

template <>
void Command::AddFlag<Value>(std::shared_ptr<Flag<Value>> flag)
{
    _flagMaxWith     = std::max(_flagMaxWith, static_cast<int>(flag->_name.length()));
    flag->_valueType = flag->_value.TypeName();
    _flagVals.push_back(flag);
}

void Command::AddCommand(std::shared_ptr<Command> cmd)
{
    _commandMaxWith     = std::max(_commandMaxWith, static_cast<int>(cmd->_use.length()));
    cmd->_parent        = shared_from_this();
    _subCmds[cmd->_use] = cmd;

    if (auto it = cmd->_subCmds.find("help"); it == cmd->_subCmds.end())
    {
        auto helpCmd = std::make_shared<Command>();

        helpCmd->_use           = "help";
        helpCmd->_short         = "Help about any command";
        helpCmd->_parent        = shared_from_this();
        _commandMaxWith         = std::max(_commandMaxWith, static_cast<int>(helpCmd->_use.length()));
        _subCmds[helpCmd->_use] = helpCmd;
    }
}

void Command::Usage()
{
    // Print the description.
    if (!_long.empty())
    {
        std::cout << _long << std::endl;
    }
    else if (!_short.empty())
    {
        std::cout << _short << std::endl;
    }

    // Print the usage message.
    if (_subCmds.empty() && !HasFlags())
    {
        return;
    }

    std::cout << "\nUsage:" << std::endl;
    std::cout << std::left
              << std::setw(4)
              << " "
              << _use;

    if (HasFlags() && !_subCmds.empty())
    {
        std::cout << " [flags] | [command]";
    }
    else if (HasFlags())
    {
        std::cout << " [flags]";
    }
    else
    {
        std::cout << " [command]";
    }

    std::cout << std::endl;

    // Print the subcommands.
    if (!_subCmds.empty())
    {
        std::cout << "\nCommands:" << std::endl;
        PrintCommands(_subCmds);
    }

    if (HasFlags())
    {
        std::cout << "\nFlags:" << std::endl;
        PrintFlags(_flagVals);
    }
}

void Command::Help()
{
    Usage();

    if (!_subCmds.empty())
    {
        std::cout << std::endl
                  << "Use \""
                  << _use
                  << " [command] --help\" for more information about a given command."
                  << std::endl;
    }
}

int Command::Execute(int argc, char* argv[])
{
    if (argc < 1)
    {
        Usage();
        return -1;
    }

    Args runArgs;
    for (const auto& flagVal : _flagVals)
    {
        // set the default value for the flag.
        runArgs.Set(flagVal->_name, flagVal->_value);
    }

    std::vector<std::string_view> args(argv, argv + argc);
    for (size_t i = 1; i < args.size(); ++i)
    {
        // --option(long option)
        if (text::StartWith(args[i], "--"))
        {
            std::string_view flag = args[i];
            std::string_view value;

            if (flag == "--help")
            {
                Help();
                return 0;
            }

            flag = flag.substr(2);

            if (text::Contains(flag, "="))
            {
                auto parts = text::Split(flag, "=");
                flag       = parts[0];
                value      = parts[1];

                auto flagVal = FindFlag(flag);
                if (flagVal == nullptr)
                {
                    std::cerr << "Unknow flag: " << flag << std::endl;
                    return -1;
                }

                if (!UpdateFlagValue(flagVal, runArgs, std::string(value)))
                {
                    return -1;
                }

                continue;
            }

            if (i + 1 < args.size() && !text::StartWith(args[i + 1], "-"))
            {
                value = args[++i];
            }

            auto flagVal = FindFlag(flag);
            if (flagVal == nullptr)
            {
                std::cerr << "Unknow flag: " << flag << std::endl;
                return -1;
            }

            if (!UpdateFlagValue(flagVal, runArgs, std::string(value)))
            {
                return -1;
            }

            continue;
        }

        // -o (short option)
        if (text::StartWith(args[i], "-"))
        {
            std::string_view flag = args[i];
            std::string_view value;

            if (flag == "-h")
            {
                Help();
                return 0;
            }

            flag = flag.substr(1);

            if (text::Contains(flag, "="))
            {
                auto parts = text::Split(flag, "=");
                flag       = parts[0];
                value      = parts[1];

                auto flagVal = FindFlag(flag);
                if (flagVal == nullptr)
                {
                    std::cerr << "Unknow flag: " << flag << std::endl;
                    return -1;
                }

                if (!UpdateFlagValue(flagVal, runArgs, std::string(value)))
                {
                    return -1;
                }

                continue;
            }

            if (i + 1 < args.size() && !text::StartWith(args[i + 1], "-"))
            {
                value = args[++i];
            }

            auto flagVal = FindFlag(flag);
            if (flagVal == nullptr)
            {
                std::cerr << "Unknow flag: " << flag << std::endl;
                return -1;
            }

            if (!UpdateFlagValue(flagVal, runArgs, std::string(value)))
            {
                return -1;
            }

            continue;
        }

        // subcommand
        std::string_view cmdName = args[i];

        if (cmdName == "help")
        {
            std::string_view value;
            if (i + 1 < args.size())
            {
                value = args[++i];
            }

            if (value.empty())
            {
                Help();
                return 0;
            }

            auto it = _subCmds.find(std::string(value));
            if (it == _subCmds.end())
            {
                std::cerr << "Unknown command: " << value << std::endl;
                return -1;
            }

            it->second->Help();
            return 0;
        }

        auto it = _subCmds.find(std::string(cmdName));
        if (it != _subCmds.end())
        {
            auto cmd = it->second;
            return cmd->Execute(static_cast<int>(args.size() - i), argv + i);
        }

        std::cerr << "Unknown command: " << cmdName << std::endl;
        Usage();
        return -1;
    }

    // If there is no subcommand, run the command.
    if (_run)
    {
        return _run(runArgs);
    }

    Usage();
    return 0;
}

std::shared_ptr<Flag<Value>> Command::FindFlag(std::string_view name)
{
    for (const auto& it : _flagVals)
    {
        if (it->_name == name || it->_shorthand == name)
        {
            return it;
        }
    }

    return nullptr;
}

bool Command::UpdateFlagValue(std::shared_ptr<Flag<Value>> flagVal, Args& args, const std::string& value)
{
    if (flagVal->_value.IsString())
    {
        flagVal->_value = value;
        args.Set(flagVal->_name, flagVal->_value);
        return true;
    }

    if (flagVal->_value.IsInt())
    {
        try
        {
            flagVal->_value = std::stoi(value);
            args.Set(flagVal->_name, flagVal->_value);
            return true;
        }
        catch (...)
        {
            std::cerr << "flag: " << flagVal->_name
                      << " value: " << value
                      << " errmsg: invalid value" << std::endl;
            return false;
        }
    }

    if (flagVal->_value.IsBool())
    {
        if (value.empty())
        {
            flagVal->_value = true;
            args.Set(flagVal->_name, flagVal->_value);
            return true;
        }
        try
        {
            flagVal->_value = text::ToBool(value);
            args.Set(flagVal->_name, flagVal->_value);
            return true;
        }
        catch (...)
        {
            std::cerr << "flag: " << flagVal->_name
                      << " value: " << value
                      << " errmsg: invalid value" << std::endl;
            return false;
        }
    }

    if (flagVal->_value.IsFloat())
    {
        try
        {
            flagVal->_value = std::stof(value);
            args.Set(flagVal->_name, flagVal->_value);
            return true;
        }
        catch (...)
        {
            std::cerr << "flag: " << flagVal->_name
                      << " value: " << value
                      << " errmsg: invalid value" << std::endl;
            return false;
        }
    }

    if (flagVal->_value.IsDouble())
    {
        try
        {
            flagVal->_value = std::stod(value);
            args.Set(flagVal->_name, flagVal->_value);
            return true;
        }
        catch (...)
        {
            std::cerr << "flag: " << flagVal->_name
                      << " value: " << value
                      << " errmsg: invalid value" << std::endl;
            return false;
        }
    }

    return false;
}

void Command::Help(std::string_view cmdName)
{
    auto it = _subCmds.find(std::string(cmdName));
    if (it != _subCmds.end())
    {
        auto cmd = it->second;
        cmd->Usage();
        return;
    }

    std::cerr << "Unknown command: " << cmdName << std::endl;
    Usage();
}

bool Command::HasFlags()
{
    return !_flagVals.empty();
}

std::shared_ptr<Command> Command::GetSubcommand(const std::string& name) const
{
    auto it = _subCmds.find(name);
    if (it != _subCmds.end())
    {
        return it->second;
    }
    return nullptr;
}

bool Command::HasSubcommands() const
{
    return !_subCmds.empty();
}

} // namespace viper::option
