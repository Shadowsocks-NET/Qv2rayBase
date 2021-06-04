#pragma once

#include "Qv2rayBaseApplication.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QJsonDocument>
#include <QRegularExpression>

#define REGEX_IPV6_ADDR                                                                                                                              \
    R"(\[\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*\])"
#define REGEX_IPV4_ADDR                                                                                                                              \
    R"((\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5]))"
#define REGEX_PORT_NUMBER R"(([0-9]|[1-9]\d{1,3}|[1-5]\d{4}|6[0-5]{2}[0-3][0-5])*)"

namespace Qv2ray::common
{
    QV2RAYBASE_EXPORT QString SafeBase64Decode(QString string);
    QV2RAYBASE_EXPORT QString SafeBase64Encode(const QString &string, bool trim);
    QV2RAYBASE_EXPORT QString Base64Encode(const QString &string);
    QV2RAYBASE_EXPORT QString Base64Decode(const QString &string);
    //
    //
    QV2RAYBASE_EXPORT QJsonObject JsonFromString(const QString &string);
    QV2RAYBASE_EXPORT QString JsonToString(const QJsonValue &json, QJsonDocument::JsonFormat format = QJsonDocument::JsonFormat::Indented);
    QV2RAYBASE_EXPORT QString JsonToString(const QJsonObject &json, QJsonDocument::JsonFormat format = QJsonDocument::JsonFormat::Indented);
    QV2RAYBASE_EXPORT QString JsonToString(const QJsonArray &array, QJsonDocument::JsonFormat format = QJsonDocument::JsonFormat::Indented);
    QV2RAYBASE_EXPORT QString VerifyJsonString(const QString &source);
    //
    //
    QV2RAYBASE_EXPORT QStringList SplitLines(const QString &str);
    QV2RAYBASE_EXPORT const QString GenerateRandomString(int len = 12);
    inline QString GenerateUuid()
    {
        return GenerateRandomString().toLower();
        // return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    inline QString TruncateString(const QString &str, int limit = -1, const QString &suffix = "...")
    {
        QString t = str;
        t.truncate(limit);
        return (limit == -1 || str.length() < limit) ? str : (t + suffix);
    }
    inline QString timeToString(const time_t &t)
    {
        QDateTime timestamp;
        timestamp.setSecsSinceEpoch(t);
        return timestamp.toString();
    }
    //
    //
    QV2RAYBASE_EXPORT QString FormatBytes(const int64_t bytes);
    QV2RAYBASE_EXPORT bool FileExistsIn(const QDir &dir, const QString &fileName);
    QV2RAYBASE_EXPORT bool IsValidFileName(const QString &fileName);
    QV2RAYBASE_EXPORT bool StringToFile(const QString &text, const QString &targetpath);
    QV2RAYBASE_EXPORT QStringList GetFileList(const QDir &dir);
    QV2RAYBASE_EXPORT QString RemoveInvalidFileName(const QString &fileName);
    QV2RAYBASE_EXPORT QString StringFromFile(const QString &filePath);
    QV2RAYBASE_EXPORT QString StringFromFile(QFile &source);
    QV2RAYBASE_EXPORT void DeducePossibleFileName(const QString &baseDir, QString *fileName, const QString &extension);
    //
    //
    namespace validation
    {
        const inline QRegularExpression __regex_ipv4_full(REGEX_IPV4_ADDR "$");

        inline bool IsIPv4Address(const QString &addr)
        {
            return __regex_ipv4_full.match(addr).hasMatch();
        }

        inline bool IsIPv6Address(const QString &addr)
        {
            QHostAddress address(addr);
            return QAbstractSocket::IPv6Protocol == address.protocol();
        }

        inline bool IsValidIPAddress(const QString &addr)
        {
            return !addr.isEmpty() && (IsIPv4Address(addr) || IsIPv6Address(addr));
        }

        inline bool IsValidDNSServer(const QString &addr)
        {
            return IsIPv4Address(addr)                  //
                   || IsIPv6Address(addr)               //
                   || addr.startsWith("https://")       //
                   || addr.startsWith("https+local://") //
                   || addr.startsWith("quic+local://")  //
                   || addr == "localhost"               //
                   || addr == "fakedns";
        }
    } // namespace validation

    QV2RAYBASE_EXPORT void QvMessageBoxWarn(QWidget *parent, const QString &title, const QString &text);
    QV2RAYBASE_EXPORT void QvMessageBoxInfo(QWidget *parent, const QString &title, const QString &text);
    QV2RAYBASE_EXPORT MessageOpt QvMessageBoxAsk(QWidget *parent, const QString &title, const QString &text, const QList<MessageOpt> &opt = { Yes, No });
} // namespace Qv2ray::common

using namespace Qv2ray::common;
using namespace Qv2ray::common::validation;
