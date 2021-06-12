#pragma once

#include <QDir>
#include <QFileInfo>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QSaveFile>

namespace Qv2rayBase::Utils
{
    inline std::optional<QString> VerifyJsonString(const QString &source)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(source.toUtf8(), &error);
        Q_UNUSED(doc)
        return error.error == QJsonParseError::NoError ? std::nullopt : std::optional{ error.errorString() };
    }

    inline QString JsonToString(const QJsonValue &json, QJsonDocument::JsonFormat format = QJsonDocument::Indented)
    {
        if (json.isArray())
            return JsonToString(json.toArray(), format);
        if (json.isObject())
            return JsonToString(json.toObject(), format);
        return json.toVariant().toString();
    }

    inline QString JsonToString(const QJsonObject &json, QJsonDocument::JsonFormat format = QJsonDocument::Indented)
    {
        QJsonDocument doc;
        doc.setObject(json);
        return doc.toJson(format);
    }

    inline QString JsonToString(const QJsonArray &array, QJsonDocument::JsonFormat format = QJsonDocument::Indented)
    {
        QJsonDocument doc;
        doc.setArray(array);
        return doc.toJson(format);
    }

    inline QByteArray ReadFile(const QString &filePath)
    {
        QFile f(filePath);
        f.open(QFile::ReadOnly);
        const auto data = f.readAll();
        f.close();
        return data;
    }

    inline bool WriteFile(const QByteArray &content, const QString &targetpath)
    {
        bool override = false;
        {
            QFileInfo info(targetpath);
            override = info.exists();
            if (!override && !info.dir().exists())
                info.dir().mkpath(info.dir().path());
        }
        QSaveFile f{ targetpath };
        f.open(QIODevice::WriteOnly);
        f.write(content);
        f.commit();
        return override;
    }

    inline QJsonObject JsonFromString(const QString &string)
    {
        QJsonDocument doc = QJsonDocument::fromJson(string.toUtf8());
        return doc.object();
    }

    inline QString GenerateRandomString(int len = 12)
    {
        static const QString possibleCharacters("abcdefghijklmnopqrstuvwxyz");
        QString randomString;

        for (int i = 0; i < len; ++i)
        {
            uint rand = QRandomGenerator::system()->generate();
            uint max = static_cast<uint>(possibleCharacters.length());
            QChar nextChar = possibleCharacters[rand % max];
            randomString.append(nextChar);
        }

        return randomString.toLower();
    }

    inline QString Base64Encode(QString string)
    {
        return string.toUtf8().toBase64();
    }

    inline QString Base64Decode(QString string)
    {
        return QByteArray::fromBase64(string.toUtf8());
    }

    inline QString SafeBase64Encode(QString string)
    {
        return Base64Encode(string).replace(QChar('+'), QChar('-')).replace(QChar('/'), QChar('_'));
    }

    inline QString SafeBase64Decode(QString string)
    {
        return Base64Decode(string.replace(QChar('-'), QChar('+')).replace(QChar('_'), QChar('/')));
    }

    inline QString FormatBytes(const int64_t b)
    {
        const static char *sizes[5] = { "B", "KB", "MB", "GB", "TB" };
        auto _bytes = b;
        char str[64];
        int i;

        double dblByte = _bytes;
        for (i = 0; i < 5 && _bytes >= 1000; i++, _bytes /= 1000)
            dblByte = _bytes / 1000.0;

        sprintf(str, "%.2f", dblByte);
        return QString(str) + " " + QString(sizes[i]);
    }

    inline QStringList SplitLines(const QString &_string)
    {
        return _string.split(QRegularExpression{ "[\r\n]" }, Qt::SkipEmptyParts);
    }

    inline QString TimeToString(const time_t &t)
    {
        return QDateTime::fromSecsSinceEpoch(t).toString();
    }

    inline bool IsIPv4Address(const QString &addr)
    {
        return QHostAddress(addr).protocol() == QAbstractSocket::IPv4Protocol;
    }

    inline bool IsIPv6Address(const QString &addr)
    {
        return QHostAddress(addr).protocol() == QAbstractSocket::IPv6Protocol;
    }

    inline bool IsValidIPAddress(const QString &addr)
    {
        return !addr.isEmpty() && (IsIPv4Address(addr) || IsIPv6Address(addr));
    }

    inline bool IsValidV2RayDNSServer(const QString &addr)
    {
        return IsIPv4Address(addr)                  //
               || IsIPv6Address(addr)               //
               || addr.startsWith("https://")       //
               || addr.startsWith("https+local://") //
               || addr.startsWith("quic+local://")  //
               || addr == "localhost"               //
               || addr == "fakedns";
    }
} // namespace Qv2rayBase::Utils

using namespace Qv2rayBase::Utils;
