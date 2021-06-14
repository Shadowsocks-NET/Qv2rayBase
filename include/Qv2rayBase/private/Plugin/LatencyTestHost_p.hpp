#pragma once

namespace Qv2rayBase::Plugin
{
    class LatencyTestThread;
    class LatencyTestHostPrivate
    {
      public:
        // we're not introduce multi latency test thread for now,
        // cause it's easy to use a scheduler like round-robin scheme
        // and libuv event loop is fast.
        LatencyTestThread *latencyThread;
    };
} // namespace Qv2rayBase::Plugin
