#pragma once

#include "Common/CommonTypes.hpp"
#include "Qv2rayBase_export.h"

#include <QString>

namespace Qv2rayBase::Profile
{
    QV2RAYBASE_EXPORT std::optional<std::pair<QString, ProfileContent>> ConvertConfigFromString(const QString &link);
    QV2RAYBASE_EXPORT const QString ConvertConfigToString(const ConnectionId &id);
    QV2RAYBASE_EXPORT const QString ConvertConfigToString(const QString &alias, const ProfileContent &root);
} // namespace Qv2rayBase::Profile
