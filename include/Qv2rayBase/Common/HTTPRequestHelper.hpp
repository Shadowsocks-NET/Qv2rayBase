//  Qv2rayBase, the modular feature-rich infrastructure library for Qv2ray.
//  Copyright (C) 2021 Moody and relavent Qv2ray contributors.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <QByteArray>
#include <QNetworkReply>
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
        static void AsyncHttpGet(const QString &url, QObject *context, const std::function<void(const QByteArray &)> &funcPtr);
        static std::tuple<QNetworkReply::NetworkError, QString, QByteArray> HttpGet(const QUrl &url);

      private:
        static void setAccessManagerAttributes(QNetworkRequest &request, QNetworkAccessManager &accessManager);
        static void setHeader(QNetworkRequest &request, const QByteArray &key, const QByteArray &value);
    };
} // namespace Qv2rayBase::Utils
