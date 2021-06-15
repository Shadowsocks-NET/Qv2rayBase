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

#include "Profile/Serialization.hpp"

#include "Common/ProfileHelpers.hpp"
#include "Common/Utils.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/ProfileManager.hpp"
#include "Qv2rayBaseLibrary.hpp"

namespace Qv2rayBase::Profile
{
    std::optional<std::pair<QString, ProfileContent>> ConvertConfigFromString(const QString &link)
    {
        const auto optConf = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_Deserialize(link);
        if (!optConf)
            return std::nullopt;

        const auto &[name, outbound] = *optConf;
        ProfileContent root;
        root.outbounds.append(outbound);
        return std::pair{ name, root };
    }

    const QString ConvertConfigToString(const ConnectionId &id)
    {
        auto alias = GetDisplayName(id);
        auto server = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        return ConvertConfigToString(alias, server);
    }

    const QString ConvertConfigToString(const QString &alias, const ProfileContent &server)
    {
        const auto outbound = server.outbounds.first();
        return *Qv2rayBaseLibrary::PluginAPIHost()->Outbound_Serialize(alias, outbound);
    }
} // namespace Qv2rayBase::Profile
