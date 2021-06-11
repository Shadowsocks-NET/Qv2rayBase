#pragma once

#include "Connections/ConnectionsBaseTypes.hpp"
#include "Qv2rayBaseLibrary.hpp"

namespace Qv2rayBase::Profile
{
    class IConfigurationGenerator
    {
      public:
        IConfigurationGenerator() = default;
        virtual ~IConfigurationGenerator() = default;

        virtual ProfileContent ApplyRouting(const ProfileContent &profile, const RoutingObject &route) = 0;

        //        void SaveRoutes() const;
        //        //
        //        std::tuple<bool, DNSConfig, FakeDNSConfig> GetDNSSettings(const GroupRoutingId &id) const
        //        {
        //            return { configs[id].overrideDNS, configs[id].dnsConfig, configs[id].fakeDNSConfig };
        //        }
        //        std::pair<bool, RouteConfig> GetAdvancedRoutingSettings(const GroupRoutingId &id) const
        //        {
        //            return { configs[id].overrideRoute, configs[id].routeConfig };
        //        }
        //        //
        //        bool SetDNSSettings(const GroupRoutingId &id, bool overrideGlobal, const DNSConfig &dns, const FakeDNSConfig &fakeDNS);
        //        bool SetAdvancedRouteSettings(const GroupRoutingId &id, bool overrideGlobal, const RouteConfig &dns);
        //        //
        //        OUTBOUNDS ExpandExternalConnection(const OUTBOUNDS &outbounds) const;
        //        //
        //        // Final Config Generation
        //        ProfileContent GenerateFinalConfig(const ConnectionGroupPair &pair, bool hasAPI = true) const;
        //        ProfileContent GenerateFinalConfig(ProfileContent root, const GroupRoutingId &routingId, bool hasAPI = true) const;
        //        //
        //        bool ExpandChainedOutbounds(ProfileContent &) const;
        //      private:
        //        QHash<GroupRoutingId, GroupRoutingConfig> configs;
    };
} // namespace Qv2rayBase::Profile
