export module Traits:Types;

import std;

export using ConfigString = std::string;
export using ConfigNumber = double;
export using ConfigBoolean = bool;

export template<typename... Fs>
struct Visitors : Fs... {
    using Fs::operator()...;
};
