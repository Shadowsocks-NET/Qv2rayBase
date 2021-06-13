#pragma once
#include "Handlers/LatencyTestHandler.hpp"

#include <QThread>

namespace Qv2rayBase::_private
{
    class LatencyTestThreadPrivate;
    class LatencyTestThread : public QThread
    {
        Q_OBJECT
      public:
        explicit LatencyTestThread(QObject *parent = nullptr);
        void stopLatencyTest();
        void pushRequest(const ConnectionId &id, const LatencyTestEngineId &engine);

      protected:
        void run() override;

      private:
        QScopedPointer<LatencyTestThread> d_ptr;
        Q_DECLARE_PRIVATE(LatencyTestThread)
    };
} // namespace Qv2rayBase::_private
