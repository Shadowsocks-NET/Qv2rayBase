#include "Plugin/LatencyTestHost.hpp"

#include "private/Plugin/LatencyTestHost_p.hpp"
#include "private/Plugin/LatencyTestThread_p.hpp"

namespace Qv2rayBase::Plugin
{
    LatencyTestHost::LatencyTestHost(QObject *parent) : QObject(parent)
    {
        d_ptr.reset(new LatencyTestHostPrivate);
        Q_D(LatencyTestHost);
        d->latencyThread = new LatencyTestThread(this);
        d->latencyThread->start();
    }

    LatencyTestHost::~LatencyTestHost()
    {
        Q_D(LatencyTestHost);
        d->latencyThread->stopLatencyTest();
        d->latencyThread->wait();
    }

    void LatencyTestHost::TestLatency(const ConnectionId &connectionId, const LatencyTestEngineId &engine)
    {
        Q_D(LatencyTestHost);
        d->latencyThread->pushRequest(connectionId, engine);
    }

    void LatencyTestHost::StopAllLatencyTest()
    {
        Q_D(LatencyTestHost);
        d->latencyThread->stopLatencyTest();
        d->latencyThread->wait();
        d->latencyThread->start();
    }

} // namespace Qv2rayBase::Plugin
