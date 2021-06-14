#pragma once
#include <QJsonObject>

namespace Qv2rayBase::_private
{
    QJsonObject UpgradeConfigInc(int fromVersion, const QJsonObject &original);
    QJsonObject MigrateSettings(const QJsonObject &original);
}
