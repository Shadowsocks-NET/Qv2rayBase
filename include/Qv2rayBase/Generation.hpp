#pragma once
#include "Qv2rayBase.hpp"

static const inline QStringList V2RayLogLevel = { "none", "debug", "info", "warning", "error" };

namespace Qv2ray::core::connection::generation
{
    namespace routing
    {
        enum RuleType
        {
            RULE_DOMAIN,
            RULE_IP
        };
        QV2RAYBASE_EXPORT ROUTERULE GenerateSingleRouteRule(RuleType t, const QString &str, const QString &outboundTag, const QString &type = "field");
        QV2RAYBASE_EXPORT ROUTERULE GenerateSingleRouteRule(RuleType t, const QStringList &list, const QString &outboundTag, const QString &type = "field");
        QV2RAYBASE_EXPORT QJsonObject GenerateDNS(const DNSConfig &dnsServer);
        QV2RAYBASE_EXPORT ROUTING GenerateRoutes(bool enableProxy, bool bypassCN, bool bypassLAN, const QString &outboundTag, const RouteConfig &routeConfig);
    } // namespace routing

    namespace misc
    {
        QJsonObject GenerateAPIEntry(const QString &tag, bool withHandler = true, bool withLogger = true, bool withStats = true);
    } // namespace misc

    namespace inbounds
    {
        QV2RAYBASE_EXPORT INBOUNDSETTING GenerateDokodemoIN(const QString &address, int port, const QString &network, int timeout = 0, bool followRedirect = false);
        QV2RAYBASE_EXPORT INBOUNDSETTING GenerateHTTPIN(bool auth, const QList<AccountObject> &accounts, int timeout = 300, bool allowTransparent = false);
        QV2RAYBASE_EXPORT INBOUNDSETTING GenerateSocksIN(const QString &auth, const QList<AccountObject> &_accounts, bool udp = false, const QString &ip = "127.0.0.1");
        QV2RAYBASE_EXPORT INBOUNDS GenerateDefaultInbounds();
        QV2RAYBASE_EXPORT QJsonObject GenerateSniffingObject(bool enabled, QList<QString> destOverride, bool metadataOnly = false);
        QV2RAYBASE_EXPORT INBOUND GenerateInboundEntry(const QString &tag,               //
                                                       const QString &protocol,          //
                                                       const QString &listen,            //
                                                       int port,                         //
                                                       const INBOUNDSETTING &settings,   //
                                                       const QJsonObject &sniffing = {}, //
                                                       const QJsonObject &allocate = {});
    } // namespace inbounds

    namespace outbounds
    {
        QV2RAYBASE_EXPORT OUTBOUNDSETTING GenerateFreedomOUT(const QString &domainStrategy, const QString &redirect);
        QV2RAYBASE_EXPORT OUTBOUNDSETTING GenerateBlackHoleOUT(bool useHTTP);
        QV2RAYBASE_EXPORT OUTBOUNDSETTING GenerateShadowSocksOUT(const QList<ShadowSocksServerObject> &servers);
        QV2RAYBASE_EXPORT OUTBOUNDSETTING GenerateShadowSocksServerOUT(const QString &address, int port, const QString &method, const QString &password);
        QV2RAYBASE_EXPORT OUTBOUNDSETTING GenerateHTTPSOCKSOut(const QString &address, int port, bool useAuth, const QString &username, const QString &password);
        QV2RAYBASE_EXPORT OUTBOUND GenerateOutboundEntry(const QString &tag,                //
                                                         const QString &protocol,           //
                                                         const OUTBOUNDSETTING &settings,   //
                                                         const QJsonObject &streamSettings, //
                                                         const QJsonObject &mux = {},       //
                                                         const QString &sendThrough = "0.0.0.0");
    } // namespace outbounds

    namespace filters
    {
        // mark all outbound
        QV2RAYBASE_EXPORT void OutboundMarkSettingFilter(CONFIGROOT &root, const int mark);
        QV2RAYBASE_EXPORT void RemoveEmptyMuxFilter(CONFIGROOT &root);
        QV2RAYBASE_EXPORT void DNSInterceptFilter(CONFIGROOT &root, const bool have_tproxy, const bool have_tproxy_v6, const bool have_socks);
        QV2RAYBASE_EXPORT void BypassBTFilter(CONFIGROOT &root);
        QV2RAYBASE_EXPORT void mKCPSeedFilter(CONFIGROOT &root);
        QV2RAYBASE_EXPORT void FillupTagsFilter(CONFIGROOT &root, const QString &subKey);
    } // namespace filters

} // namespace Qv2ray::core::connection::generation

using namespace Qv2ray::core;
using namespace Qv2ray::core::connection;
using namespace Qv2ray::core::connection::generation;
using namespace Qv2ray::core::connection::generation::filters;
using namespace Qv2ray::core::connection::generation::inbounds;
using namespace Qv2ray::core::connection::generation::outbounds;
using namespace Qv2ray::core::connection::generation::routing;
using namespace Qv2ray::core::connection::generation::misc;
