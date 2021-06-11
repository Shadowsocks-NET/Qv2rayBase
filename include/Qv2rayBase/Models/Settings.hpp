#pragma once

#include "Qv2rayBaseFeatures.hpp"

#include <QMap>
#include <QString>

constexpr int QV2RAY_SETTINGS_VERSION = 16;

namespace Qv2rayBase::Models
{
    struct NetworkProxyConfig
    {
        enum ProxyType
        {
            PROXY_NONE = 0,
            PROXY_SYSTEM = 1,
            PROXY_HTTP = 2,
            PROXY_SOCKS5 = 3,
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
        QVariantMap extra_options;
    };

#if QV2RAYBASE_FEATURE(subscriptions)
    enum SubscriptionFilterRelation
    {
        RELATION_AND = 0,
        RELATION_OR = 1
    };

    struct SubscriptionConfigObject
    {
        QString address;
        QString type = "sip008";
        float updateInterval = 10;
        QList<QString> includeKeywords;
        QList<QString> excludeKeywords;
        SubscriptionFilterRelation includeRelation = RELATION_OR;
        SubscriptionFilterRelation excludeRelation = RELATION_AND;
    };
#endif

#if QV2RAYBASE_FEATURE(statistics)
    enum StatisticsType
    {
        API_ALL_INBOUND = 0,
        API_OUTBOUND_PROXY = 1,
        API_OUTBOUND_DIRECT = 2,
        API_OUTBOUND_BLACKHOLE = 3,
    };

    struct StatisticsObject
    {
        // clang-format off
        struct StatsEntry { quint64 up; quint64 down; };
        StatsEntry &operator[](StatisticsType i) { while (entries.count() <= i) entries.append(StatsEntry{}); return entries[i]; }
        StatsEntry &get(StatisticsType i)        { while (entries.count() <= i) entries.append(StatsEntry{}); return entries[i]; }
        void Clear() { entries.clear(); }
        // clang-format on

      private:
        QList<StatsEntry> entries;
    };
#endif

#if QV2RAYBASE_FEATURE(latency)
    constexpr unsigned int LATENCY_TEST_VALUE_ERROR = 99999;
    constexpr unsigned int LATENCY_TEST_VALUE_NODATA = LATENCY_TEST_VALUE_ERROR - 1;
#endif

} // namespace Qv2rayBase::Models
