#include "BuiltinLatencyTesterPlugin.hpp"

#include "latency/TCPLatencyTester.hpp"

namespace Qv2rayBase::BuiltinPlugins::Latency
{
    class LatencyHandler : public ILatencyHandler
    {
        const LatencyTestEngineId TCPEngineId{ "qv2ray_tcp__" };
        const LatencyTestEngineId ICMPEngineId{ "qv2ray_icmp_" };
        virtual QList<LatencyTestEngineInfo> PluginLatencyTestEngines() const
        {
            return { LatencyTestEngineInfo{ TCPEngineId, true, "TCP Connect Latency", "", { []() { return std::make_unique<Static_TCP_LatencyTestEngine>(); } } },
                     LatencyTestEngineInfo{ ICMPEngineId, true, "ICMP Ping Latency", "", { []() { return std::make_unique<Static_TCP_LatencyTestEngine>(); } } } };
        };
    };

    const QvPluginMetadata BuiltinLatencyTesterPlugin::GetMetadata() const
    {
        return { "Qv2rayBase Builtin Latency Test Engine Provider",                                        //
                 "Qv2rayBase Development Team",                                                            //
                 "qvplugin_builtin_latency_tester",                                                        //
                 "Builtin Latency Test Engine Provider, with TCP and ICMP latency test engine supported.", //
                 "",                                                                                       //
                 { COMPONENT_LATENCY_TEST_ENGINE } };
    }

    BuiltinLatencyTesterPlugin::~BuiltinLatencyTesterPlugin()
    {
    }

    bool BuiltinLatencyTesterPlugin::InitializePlugin()
    {
        m_LatencyTestHandler = std::make_shared<LatencyHandler>();
        return true;
    }

    void BuiltinLatencyTesterPlugin::SettingsUpdated()
    {
    }

} // namespace Qv2rayBase::StaticPlugin
