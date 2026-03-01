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

#include "core/option/value.h"

#include <string>
#include <variant>

namespace viper::option {

Value::Value(int val)
    : _value(val){};

Value::Value(float val)
    : _value(val){};

Value::Value(double val)
    : _value(val){};

Value::Value(bool val)
    : _value(val){};

Value::Value(const char* val)
    : _value(val){};

Value::Value(const std::string& val)
    : _value(val){};

Value& Value::operator=(int val)
{
    _value = val;
    return *this;
}

Value& Value::operator=(float val)
{
    _value = val;
    return *this;
}

Value& Value::operator=(double val)
{
    _value = val;
    return *this;
}

Value& Value::operator=(bool val)
{
    _value = val;
    return *this;
}

Value& Value::operator=(const char* val)
{
    _value = val;
    return *this;
}

Value& Value::operator=(const std::string& val)
{
    _value = val;
    return *this;
}

bool Value::IsInt() const
{
    return std::holds_alternative<int>(_value);
}

bool Value::IsFloat() const
{
    return std::holds_alternative<float>(_value);
}

bool Value::IsDouble() const
{
    return std::holds_alternative<double>(_value);
}

bool Value::IsBool() const
{
    return std::holds_alternative<bool>(_value);
}

bool Value::IsString() const
{
    return std::holds_alternative<std::string>(_value);
}

std::string Value::TypeName() const
{
    return std::visit([](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>)
        {
            return "int";
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            return "float";
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return "double";
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return "bool";
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return "string";
        }
        return "unknown";
    },
                      _value);
}

Value::operator int() const
{
    return std::get<int>(_value);
}

Value::operator float() const
{
    return std::get<float>(_value);
}

Value::operator double() const
{
    return std::get<double>(_value);
}

Value::operator bool() const
{
    return std::get<bool>(_value);
}

Value::operator std::string() const
{
    return std::get<std::string>(_value);
}

Value::operator const char*() const
{
    return std::get<std::string>(_value).c_str();
}

std::ostream& operator<<(std::ostream& os, const Value& val)
{
    std::visit([&os](const auto& arg) {
        os << arg;
    },
               val._value);
    return os;
}

std::ostream& operator<<(std::ostream& os, Value* val)
{
    std::visit([&os](const auto& arg) {
        os << arg;
    },
               val ? val->_value : "nullptr");
    return os;
}
} // namespace viper::option
