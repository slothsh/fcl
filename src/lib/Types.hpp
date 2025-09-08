#pragma once

#include <string>

using ConfigString = std::string;
using ConfigNumber = double;
using ConfigBoolean = bool;

template<typename... Fs>
struct Visitors : Fs... {
    using Fs::operator()...;
};
