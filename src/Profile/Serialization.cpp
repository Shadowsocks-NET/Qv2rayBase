#include "Profile/Serialization.hpp"

#include "Common/ProfileHelper.hpp"
#include "Common/Utils.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Profile/ProfileManager.hpp"
#include "Qv2rayBaseLibrary.hpp"

#include <QJsonIO.hpp>

namespace Qv2rayBase::Profile
{
    std::optional<std::pair<QString, ProfileContent>> ConvertConfigFromString(const QString &link)
    {
        const auto config = Qv2rayBaseLibrary::PluginAPIHost()->Outbound_Deserialize(link);
        if (!config)
            return std::nullopt;

        const auto &[_alias, _protocol, _outbound, _stream] = *config;
        ProfileContent root;
        const auto outbound = QJsonObject{ { "tag", "OUTBOUND_TAG_PROXY" }, { "protocol", _protocol }, { "settings", _outbound }, { "streamSettings", _stream } };
        QJsonIO::SetValue(root, outbound, "outbounds", 0);
        return std::pair{ _alias, root };
    }

    const QString ConvertConfigToString(const ConnectionId &id)
    {
        auto alias = GetDisplayName(id);
        if (IsComplexConfig(id))
        {
            return "QV2RAY_SERIALIZATION_COMPLEX_CONFIG_PLACEHOLDER";
        }
        auto server = Qv2rayBaseLibrary::ProfileManager()->GetConnection(id);
        return ConvertConfigToString(alias, server);
    }

    const QString ConvertConfigToString(const QString &alias, const ProfileContent &server)
    {
        const auto outbound = OUTBOUND(server["outbounds"].toArray().first().toObject());
        const auto type = outbound["protocol"].toString();
        const auto settings = outbound["settings"].toObject();
        const auto streamSettings = outbound["streamSettings"].toObject();
        return *Qv2rayBaseLibrary::PluginAPIHost()->Outbound_Serialize(Qv2rayPlugin::PluginOutboundDescriptor{ alias, type, settings, streamSettings });
    }
} // namespace Qv2rayBase::Profile
