#pragma once
#include "Qv2rayBase.hpp"
#include "QvHelpers.hpp"

namespace Qv2ray::core::connection::serialization
{
    QV2RAYBASE_EXPORT const inline auto QV2RAY_SERIALIZATION_COMPLEX_CONFIG_PLACEHOLDER = "(N/A)";
    /**
     * pattern for the nodes in ssd links.
     * %1: airport name
     * %2: node name
     * %3: rate
     */
    QV2RAYBASE_EXPORT const inline QString QV2RAY_SSD_DEFAULT_NAME_PATTERN = "%1 - %2 (rate %3)";
    QV2RAYBASE_EXPORT QList<std::pair<QString, CONFIGROOT>> ConvertConfigFromString(const QString &link, QString *aliasPrefix, QString *errMessage,
                                                                                    QString *newGroupName = nullptr);
    QV2RAYBASE_EXPORT const QString ConvertConfigToString(const ConnectionGroupPair &id);
    QV2RAYBASE_EXPORT const QString ConvertConfigToString(const QString &alias, const QString &groupName, const CONFIGROOT &server);

    namespace vmess
    {
        QV2RAYBASE_EXPORT CONFIGROOT Deserialize(const QString &vmess, QString *alias, QString *errMessage);
        QV2RAYBASE_EXPORT const QString Serialize(const StreamSettingsObject &transfer, const VMessServerObject &server, const QString &alias);
    } // namespace vmess

    namespace vmess_new
    {
        QV2RAYBASE_EXPORT CONFIGROOT Deserialize(const QString &vmess, QString *alias, QString *errMessage);
        QV2RAYBASE_EXPORT const QString Serialize(const StreamSettingsObject &transfer, const VMessServerObject &server, const QString &alias);
    } // namespace vmess_new

    namespace vless
    {
        QV2RAYBASE_EXPORT CONFIGROOT Deserialize(const QString &vless, QString *alias, QString *errMessage);
    } // namespace vless

    namespace ss
    {
        QV2RAYBASE_EXPORT CONFIGROOT Deserialize(const QString &ss, QString *alias, QString *errMessage);
        QV2RAYBASE_EXPORT const QString Serialize(const ShadowSocksServerObject &server, const QString &alias);
    } // namespace ss

    namespace ssd
    {
        QV2RAYBASE_EXPORT QList<std::pair<QString, CONFIGROOT>> Deserialize(const QString &uri, QString *groupName, QStringList *logList);
    } // namespace ssd

} // namespace Qv2ray::core::connection::serialization

using namespace Qv2ray::core;
using namespace Qv2ray::core::connection;
using namespace Qv2ray::core::connection::serialization;
