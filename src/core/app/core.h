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
#include "third-party/cmdline/cmdline.h"

#include <functional>
#include <memory>
#include <system_error>
#include <vector>

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
     * @brief AddCommand add a flag
     *
     */
    void AddCommand(const FlagInt& flag);
    void AddCommand(const FlagString& flag);
    void AddCommand(const FlagBool& flag);
    void AddCommand(const FlagNoValue& flag);

    /**
     * @brief Run block to run the service
     *
     * @param argc the argv count
     * @param argv the all argv
     * @param executor the executor function, if it exited, the process will be exited
     * @return std::error_code Success for sucess, the others for error
     */
    std::error_code Run(int argc, char* argv[], CommandHandler executor);

    /**
     * @brief Close close the application
     *
     */
    void Close();

private:
    ContextPtr               _ctx;
    EnvPtr                   _env;
    ConfigFilePtr            _file;
    cmdline::parser          _parser;
    std::vector<FlagInt>     _iFlags;
    std::vector<FlagString>  _sFlags;
    std::vector<FlagBool>    _bFlags;
    std::vector<FlagNoValue> _nvFlags;
};

using CorePtr = std::shared_ptr<Core>;

} // namespace app
} // namespace viper

#endif
