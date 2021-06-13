#include "TCPLatencyTester.hpp"

namespace Qv2rayBase::BuiltinPlugins::Latency
{
    constexpr int TCP_CONNECTION_TIMEOUT_SECS = 5;

    int getSocket(int af, int socktype, int proto)
    {
        uv_os_sock_t fd;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif
        if ((fd = socket(af, socktype, proto)) == INVALID_SOCKET)
        {
            return 0;
        }
        // Set TCP connection timeout per-socket level.
        // See [https://github.com/libuv/help/issues/54] for details.
#if defined(_WIN32) && !defined(__SYMBIAN32__)
#ifndef TCP_MAXRT
#define TCP_MAXRT 5
#endif
        setsockopt(fd, IPPROTO_TCP, TCP_MAXRT, (char *) &TCP_CONNECTION_TIMEOUT_SECS, sizeof(TCP_CONNECTION_TIMEOUT_SECS));
#elif defined(__APPLE__)
        // (billhoo) MacOS uses TCP_CONNECTIONTIMEOUT to do so.
        setsockopt(fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, (char *) &TCP_CONNECTION_TIMEOUT_SECS, sizeof(TCP_CONNECTION_TIMEOUT_SECS));
#else // Linux like systems
        uint32_t conn_timeout_ms = TCP_CONNECTION_TIMEOUT_SECS * 1000;
        setsockopt(fd, IPPROTO_TCP, TCP_USER_TIMEOUT, (char *) &conn_timeout_ms, sizeof(conn_timeout_ms));
#endif
        return (int) fd;
    }

    void Static_TCP_LatencyTestEngine::Prepare(std::shared_ptr<uvw::Loop>)
    {
    }

    void Static_TCP_LatencyTestEngine::StartTest(std::shared_ptr<uvw::Loop> loop)
    {
        for (; response.total < TOTAL_TEST_COUNT; response.total++)
        {
            auto tcpClient = loop->resource<uvw::TCPHandle>();
            tcpClient->open(getSocket(af, SOCK_STREAM, IPPROTO_TCP));
            tcpClient->once<uvw::ErrorEvent>(
                [ptr = shared_from_this(), this](const uvw::ErrorEvent e, uvw::TCPHandle &h)
                {
                    response.failed++;
                    response.error = e.what();
                    h.clear();
                    h.close();
                });
            tcpClient->once<uvw::ConnectEvent>(
                [ptr = shared_from_this(), start = system_clock::now(), this](auto &, auto &h)
                {
                    successCount++;
                    system_clock::time_point end = system_clock::now();
                    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    long ms = milliseconds.count();
                    response.avg += ms;
                    response.worst = std::max(response.worst, ms);
                    response.best = std::min(response.best, ms);
                    h.clear();
                    h.close();
                });
            tcpClient->connect(reinterpret_cast<const sockaddr &>(storage));
        }
    }

} // namespace Qv2rayBase::StaticPlugin
