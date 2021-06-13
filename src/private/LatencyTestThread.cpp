#include "LatencyTestThread.hpp"

#include "Common/ProfileHelpers.hpp"
#include "Plugin/LatencyTestHost.hpp"
#include "Plugin/PluginAPIHost.hpp"
#include "Qv2rayBaseLibrary.hpp"

#include <uvw.hpp>

namespace Qv2rayBase::_private
{
    class LatencyTestThreadPrivate
    {
      public:
        std::shared_ptr<uvw::Loop> loop;
        bool isStop = false;
        std::shared_ptr<uvw::TimerHandle> stopTimer;
        std::vector<Qv2rayPlugin::Latency::LatencyTestRequest> requests;
        std::mutex m;

        void doTest(Qv2rayBase::Plugin::LatencyTestHost *parent, uvw::TimerHandle &handle)
        {
            if (isStop)
            {
                if (!requests.empty())
                    requests.clear();
                int timer_count = 0;
                uv_walk(
                    loop->raw(),
                    [](uv_handle_t *handle, void *arg)
                    {
                        int &counter = *static_cast<int *>(arg);
                        if (uv_is_closing(handle) == 0)
                            counter++;
                    },
                    &timer_count);
                if (timer_count == 1) // only current timer
                {
                    handle.stop();
                    handle.close();
                    loop->clear();
                    loop->close();
                    loop->stop();
                }
            }
            else
            {
                if (requests.empty())
                    return;
                std::unique_lock<std::mutex> lockGuard{ m };
                for (auto &req : requests)
                {
                    const auto engineInfo = Qv2rayBaseLibrary::PluginAPIHost()->Latency_GetEngine(req.engine);
                    // This is a blocking call
                    Qv2rayPlugin::LatencyTestResponse resp;
                    if (engineInfo.isAsync)
                        resp = engineInfo.Create()->TestLatencyAsync(loop, req);
                    else
                        resp = engineInfo.Create()->TestLatency(req);
                    parent->OnLatencyTestCompleted(req.id, resp);
                }
                requests.clear();
            }
        };
    };

    LatencyTestThread::LatencyTestThread(QObject *parent) : QThread(parent)
    {
    }

    void LatencyTestThread::stopLatencyTest()
    {
        Q_D(LatencyTestThread);
        d->isStop = true;
    }

    void LatencyTestThread::pushRequest(const ConnectionId &id, const LatencyTestEngineId &engine)
    {
        Q_D(LatencyTestThread);
        if (d->isStop)
            return;
        std::unique_lock<std::mutex> lockGuard{ d->m };
        const auto &[protocol, host, port] = GetOutboundInfoTuple(GetOutbound(id, 0));
        d->requests.emplace_back(Qv2rayPlugin::Latency::LatencyTestRequest{ engine, id, host, port });
    }

    void LatencyTestThread::run()
    {
        Q_D(LatencyTestThread);
        d->loop = uvw::Loop::create();
        d->stopTimer = d->loop->resource<uvw::TimerHandle>();
        d->stopTimer->on<uvw::TimerEvent>([this, d](auto &, auto &handle) { d->doTest(qobject_cast<Qv2rayBase::Plugin::LatencyTestHost *>(parent()), handle); });
        d->stopTimer->start(uvw::TimerHandle::Time{ 500 }, uvw::TimerHandle::Time{ 500 });
        d->loop->run();
    }

} // namespace Qv2rayBase::_private
