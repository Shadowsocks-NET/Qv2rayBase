#pragma once

#include "Qv2rayBaseFeatures.hpp"

#include <QJsonObject>
#include <QMap>
#include <QString>

constexpr int QV2RAY_SETTINGS_VERSION = 16;

namespace Qv2rayBase::Models
{
    struct NetworkProxyConfig
    {
        enum ProxyType
        {
            PROXY_NONE,
            PROXY_SYSTEM,
            PROXY_HTTP,
            PROXY_SOCKS5,
        };

        ProxyType type = PROXY_SYSTEM;
        QString address = "127.0.0.1";
        int port = 8000;
        QString ua = "Qv2rayBase/$VERSION WebRequestHelper";
    };

    struct PluginConfigObject
    {
        QMap<QString, bool> plugin_states;
        int plugin_port_allocation = 15490;
    };

    struct Qv2rayBaseConfigObject
    {
        const int config_version = QV2RAY_SETTINGS_VERSION;
        NetworkProxyConfig network_config;
        PluginConfigObject plugin_config;
        QJsonObject extra_options;
    };

#if QV2RAYBASE_FEATURE(latency)
    constexpr unsigned int LATENCY_TEST_VALUE_ERROR = 99999;
    constexpr unsigned int LATENCY_TEST_VALUE_NODATA = LATENCY_TEST_VALUE_ERROR - 1;
#endif

} // namespace Qv2rayBase::Models
