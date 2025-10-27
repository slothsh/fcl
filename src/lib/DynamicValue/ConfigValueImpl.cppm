export module DynamicValue:ConfigValueImpl;

import :ConfigValue;
import std;

ConfigValue::ConfigValue(ConfigValue const& config_value) {
    m_kind = config_value.m_kind;
    switch (config_value.m_kind) {
        case BOOL: m_value.boolean = config_value.m_value.boolean; break;
        case NUMBER: m_value.number = config_value.m_value.number; break;
        case STRING: m_value.string = config_value.m_value.string; break;
        default: break;
    }
}

ConfigValue& ConfigValue::operator=(ConfigValue const& config_value) {
    m_kind = config_value.m_kind;
    switch (config_value.m_kind) {
        case BOOL: m_value.boolean = config_value.m_value.boolean; break;
        case NUMBER: m_value.number = config_value.m_value.number; break;
        case STRING: m_value.string = config_value.m_value.string; break;
        default: break;
    }

    return *this;
}

ConfigValue::ConfigValue(ConfigValue&& config_value) noexcept {
    m_kind = config_value.m_kind;
    switch (config_value.m_kind) {
        case BOOL: m_value.boolean = config_value.m_value.boolean; break;
        case NUMBER: m_value.number = config_value.m_value.number; break;
        case STRING: m_value.string = config_value.m_value.string; break;
        default: break;
    }

    config_value.m_kind = STRING;
    config_value.m_value.string = "";
}

ConfigValue& ConfigValue::operator=(ConfigValue&& config_value) noexcept {
    m_kind = config_value.m_kind;
    switch (config_value.m_kind) {
        case BOOL: m_value.boolean = config_value.m_value.boolean; break;
        case NUMBER: m_value.number = config_value.m_value.number; break;
        case STRING: m_value.string = config_value.m_value.string; break;
        default: break;
    }

    config_value.m_kind = STRING;
    config_value.m_value.string = "";

    return *this;
}

ConfigValue::~ConfigValue() noexcept {
    if (m_kind == STRING) {
        m_value.string.~basic_string();
    }
}

ConfigValue::ValueUnion::~ValueUnion() noexcept {}
