#include "Common/HTTPRequestHelper.hpp"

#include "Common/Settings.hpp"
#include "Qv2rayBaseLibrary.hpp"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>

#define QV_MODULE_NAME "NetworkCore"

namespace Qv2rayBase::Utils
{
    void NetworkRequestHelper::setHeader(QNetworkRequest &request, const QByteArray &key, const QByteArray &value)
    {
        QvDebug() << "Adding HTTP request header:" << key << ":" << value;
        request.setRawHeader(key, value);
    }

    void NetworkRequestHelper::setAccessManagerAttributes(QNetworkRequest &request, QNetworkAccessManager &accessManager)
    {
        const auto &netconf = Qv2rayBaseLibrary::GetConfig()->network_config;
        switch (netconf.type)
        {
            case Models::NetworkProxyConfig::PROXY_NONE:
            {
                QvDebug() << "Get without proxy.";
                accessManager.setProxy(QNetworkProxy(QNetworkProxy::ProxyType::NoProxy));
                break;
            }
            case Models::NetworkProxyConfig::PROXY_SYSTEM:
            {
                accessManager.setProxy(QNetworkProxyFactory::systemProxyForQuery().first());
                break;
            }
            case Models::NetworkProxyConfig::PROXY_HTTP:
            {
                QNetworkProxy p{ QNetworkProxy::HttpProxy, netconf.address, quint16(netconf.port) };
                accessManager.setProxy(p);
                break;
            }
            case Models::NetworkProxyConfig::PROXY_SOCKS5:
            {
                QNetworkProxy p{ QNetworkProxy::Socks5Proxy, netconf.address, quint16(netconf.port) };
                p.setCapabilities(p.capabilities() | QNetworkProxy::HostNameLookupCapability);
                accessManager.setProxy(p);
                break;
            }
            default: Q_UNREACHABLE();
        }

        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        auto ua = netconf.ua;
        ua.replace("$VERSION", QV2RAY_BASELIB_VERSION);
        request.setHeader(QNetworkRequest::KnownHeaders::UserAgentHeader, ua);
    }

    QByteArray NetworkRequestHelper::HttpGet(const QUrl &url)
    {
        QNetworkRequest request;
        QNetworkAccessManager accessManager;
        request.setUrl(url);
        setAccessManagerAttributes(request, accessManager);
        auto _reply = accessManager.get(request);
        //
        {
            QEventLoop loop;
            QObject::connect(&accessManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
            loop.exec();
        }
        //
        // Data or timeout?
        QvLog() << "QNetworkReply:" << _reply->errorString();
        auto data = _reply->readAll();
        return data;
    }

    void NetworkRequestHelper::AsyncHttpGet(const QString &url, QObject *context, std::function<void(const QByteArray &)> funcPtr)
    {
        QNetworkRequest request;
        request.setUrl(url);
        auto accessManagerPtr = new QNetworkAccessManager();
        setAccessManagerAttributes(request, *accessManagerPtr);
        auto reply = accessManagerPtr->get(request);
        QObject::connect(reply, &QNetworkReply::finished, context,
                         [=]()
                         {
                             QvLog() << QMetaEnum::fromType<QNetworkReply::NetworkError>().key(reply->error());
                             funcPtr(reply->readAll());
                             accessManagerPtr->deleteLater();
                         });
    }

} // namespace Qv2rayBase::Utils
