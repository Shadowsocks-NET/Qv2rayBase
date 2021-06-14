#pragma once

#include <QByteArray>
#include <QUrl>
#include <functional>

class QNetworkRequest;
class QNetworkAccessManager;

namespace Qv2rayBase::Utils
{
    class NetworkRequestHelper
    {
        explicit NetworkRequestHelper() = default;
        ~NetworkRequestHelper() = default;

      public:
        static void AsyncHttpGet(const QString &url, QObject *context, std::function<void(const QByteArray &)> funcPtr);
        static QByteArray HttpGet(const QUrl &url);

      private:
        static void setAccessManagerAttributes(QNetworkRequest &request, QNetworkAccessManager &accessManager);
        static void setHeader(QNetworkRequest &request, const QByteArray &key, const QByteArray &value);
    };
} // namespace Qv2rayBase::Utils
