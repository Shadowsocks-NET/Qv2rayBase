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
