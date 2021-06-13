
/* Author: Maciek Muszkowski
 *
 * This is a simple ping implementation for Linux.
 * It will work ONLY on kernels 3.x+ and you need
 * to set allowed groups in /proc/sys/net/ipv4/ping_group_range */
#include "ICMPLatencyTester.hpp"

namespace Qv2rayBase::StaticPlugin
{
#ifdef Q_OS_UNIX
#include "ICMPLatencyTester.hpp"

#include <netinet/in.h>
#include <netinet/ip.h> //macos need that
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#ifdef Q_OS_MAC
#define SOL_IP 0
#endif

    /// 1s complementary checksum
    uint16_t ping_checksum(const char *buf, size_t size)
    {
        size_t i;
        uint64_t sum = 0;

        for (i = 0; i < size; i += 2)
        {
            sum += *(uint16_t *) buf;
            buf += 2;
        }
        if (size - i > 0)
        {
            sum += *(uint8_t *) buf;
        }

        while ((sum >> 16) != 0)
        {
            sum = (sum & 0xffff) + (sum >> 16);
        }

        return (uint16_t) ~sum;
    }

    void Static_ICMP_LatencyTestEngine::deinit()
    {
        if (socketId >= 0)
        {
            close(socketId);
            socketId = -1;
        }
    }

    bool Static_ICMP_LatencyTestEngine::checkAndFinalize()
    {
        if (response.failed + successCount == TOTAL_TEST_COUNT)
        {
            if (timeoutTimer)
            {
                timeoutTimer->stop();
                timeoutTimer->clear();
                timeoutTimer->close();
            }
            if (pollHandle)
            {
                if (!pollHandle->closing())
                    pollHandle->stop();
                pollHandle->clear();
                pollHandle->close();
            }
            return true;
        }
        return false;
    }

    Static_ICMP_LatencyTestEngine::~Static_ICMP_LatencyTestEngine()
    {
        deinit();
    }

    void Static_ICMP_LatencyTestEngine::Prepare(std::shared_ptr<uvw::Loop>)
    {
        int ttl = 30;
        if (((socketId = socket(PF_INET, SOCK_DGRAM, IPPROTO_ICMP)) < 0))
        {
            response.error = "EPING_SOCK: " + QObject::tr("Socket creation failed");
            response.avg = Qv2rayPlugin::Latency::LATENCY_TEST_VALUE_ERROR;
            errored = true;
            return;
        }
        // set TTL
        if (setsockopt(socketId, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0)
        {
            response.error = "EPING_TTL: " + QObject::tr("Failed to setup TTL value");
            response.avg = Qv2rayPlugin::Latency::LATENCY_TEST_VALUE_ERROR;
            errored = true;
            return;
            return;
        }
    }

    void Static_ICMP_LatencyTestEngine::StartTest(std::shared_ptr<uvw::Loop> loop)
    {
        timeoutTimer = loop->resource<uvw::TimerHandle>();
        uvw::OSSocketHandle osSocketHandle{ socketId };
        pollHandle = loop->resource<uvw::PollHandle>(osSocketHandle);
        timeoutTimer->once<uvw::TimerEvent>(
            [this, ptr = std::weak_ptr<Static_ICMP_LatencyTestEngine>{ shared_from_this() }](auto &, uvw::TimerHandle &)
            {
                if (ptr.expired())
                    return;
                else
                {
                    auto p = ptr.lock();
                    pollHandle->clear();
                    if (!pollHandle->closing())
                        pollHandle->stop();
                    pollHandle->close();
                    successCount = 0;
                    response.failed = response.total = TOTAL_TEST_COUNT;
                    checkAndFinalize();
                }
            });

        timeoutTimer->start(uvw::TimerHandle::Time{ 10000 }, uvw::TimerHandle::Time{ 0 });
        auto pollEvent = uvw::Flags<uvw::PollHandle::Event>::from<uvw::PollHandle::Event::READABLE>();
        pollHandle->on<uvw::PollEvent>(
            [this, ptr = shared_from_this()](uvw::PollEvent &, uvw::PollHandle &h)
            {
                timeval end;
                sockaddr_in addr;
                socklen_t slen = sizeof(sockaddr_in);
                int rlen = 0;
                char buf[1024];
                do
                {
                    do
                    {
                        rlen = recvfrom(socketId, buf, 1024, 0, (struct sockaddr *) &addr, &slen);
                    } while (rlen == -1 && errno == EINTR);

                // skip malformed
#ifdef Q_OS_MAC
                    if (rlen < sizeof(icmp) + 20)
#else
                    if (rlen < sizeof(icmp))
#endif
                        continue;

#ifdef Q_OS_MAC
                    auto &resp = *reinterpret_cast<icmp *>(buf + 20);
#else
                    auto &resp = *reinterpret_cast<icmp *>(buf);
#endif
                    // skip the ones we didn't send
                    auto cur_seq = resp.icmp_hun.ih_idseq.icd_seq;
                    if (cur_seq >= seq)
                        continue;

                    switch (resp.icmp_type)
                    {
                        case ICMP_ECHOREPLY:
                            gettimeofday(&end, nullptr);
                            response.avg += 1000 * (end.tv_sec - startTimevals[cur_seq - 1].tv_sec) + (end.tv_usec - startTimevals[cur_seq - 1].tv_usec) / 1000;
                            successCount++;
                            checkAndFinalize();
                            continue;
                        case ICMP_UNREACH:
                            response.error = "EPING_DST: " + QObject::tr("Destination unreachable");
                            response.failed++;
                            if (checkAndFinalize())
                            {
                                h.clear();
                                h.close();
                                return;
                            }
                            continue;
                        case ICMP_TIMXCEED:
                            response.error = "EPING_TIME: " + QObject::tr("Timeout");
                            response.failed++;
                            if (checkAndFinalize())
                            {
                                h.clear();
                                h.close();
                                return;
                            }
                            continue;
                        default:
                            response.error = "EPING_UNK: " + QObject::tr("Unknown error");
                            response.failed++;
                            if (checkAndFinalize())
                            {
                                h.clear();
                                h.close();
                                return;
                            }
                            continue;
                    }
                } while (rlen > 0);
            });

        pollHandle->start(pollEvent);

        for (int i = 0; i < TOTAL_TEST_COUNT; ++i)
        {
            // prepare echo request packet
            icmp _icmp_request;
            memset(&_icmp_request, 0, sizeof(_icmp_request));
            _icmp_request.icmp_type = ICMP_ECHO;
            _icmp_request.icmp_hun.ih_idseq.icd_id = 0; // SOCK_DGRAM & 0 => id will be set by kernel
            _icmp_request.icmp_hun.ih_idseq.icd_seq = seq++;
            _icmp_request.icmp_cksum = ping_checksum(reinterpret_cast<char *>(&_icmp_request), sizeof(_icmp_request));
            int n;
            timeval start;
            gettimeofday(&start, nullptr);
            startTimevals.push_back(start);
            do
            {
                n = ::sendto(socketId, &_icmp_request, sizeof(icmp), 0, (struct sockaddr *) &storage, sizeof(struct sockaddr));
            } while (n < 0 && errno == EINTR);
        }
    }
#elif defined(Q_OS_WIN)

    typedef struct _IO_STATUS_BLOCK
    {
        union
        {
            NTSTATUS Status;
            PVOID Pointer;
        } DUMMYUNIONNAME;
        ULONG_PTR Information;
    } IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
    typedef VOID(NTAPI *PIO_APC_ROUTINE)(IN PVOID ApcContext, IN PIO_STATUS_BLOCK IoStatusBlock, IN ULONG Reserved);
#define PIO_APC_ROUTINE_DEFINED
#include <WS2tcpip.h>
//
#include <Windows.h>
//
#include <iphlpapi.h>
//
#include <IcmpAPI.h>
//
#include <QString>

    Static_ICMP_LatencyTestEngine ::~Static_ICMP_LatencyTestEngine()
    {
    }
    void Static_ICMP_LatencyTestEngine::ping()
    {
        waitHandleTimer = loop->resource<uvw::TimerHandle>();
        waitHandleTimer->on<uvw::TimerEvent>(
            [ptr = shared_from_this(), this](auto &&, auto &&)
            {
                SleepEx(0, TRUE);
                if (data.failedCount + successCount == data.totalCount)
                {
                    waitHandleTimer->stop();
                    waitHandleTimer->close();
                    waitHandleTimer->clear();
                }
            });
        for (; data.totalCount < req.totalCount; ++data.totalCount)
        {
            pingImpl();
        }
        waitHandleTimer->start(uvw::TimerHandle::Time{ 500 }, uvw::TimerHandle::Time{ 500 });
    }
    void Static_ICMP_LatencyTestEngine::pingImpl()
    {
        constexpr WORD payload_size = 1;
        constexpr DWORD reply_buf_size = sizeof(ICMP_ECHO_REPLY) + payload_size + 8;
        struct ICMPReply
        {
            ICMPReply(std::function<void(bool, long, const QString &, HANDLE)> f) : whenIcmpFailed(std::move(f))
            {
                hIcmpFile = IcmpCreateFile();
            }
            unsigned char reply_buf[reply_buf_size]{};
            unsigned char payload[payload_size]{ 42 };
            HANDLE hIcmpFile;
            std::function<void(bool, long, const QString &, HANDLE)> whenIcmpFailed;
            ~ICMPReply()
            {
                if (hIcmpFile != INVALID_HANDLE_VALUE)
                    IcmpCloseHandle(hIcmpFile);
            }
        };
        auto icmpReply = new ICMPReply{ [this, id = req.id](bool isSuccess, long res, const QString &message, HANDLE h)
                                        {
                                            if (!isSuccess)
                                            {
                                                data.errorMessage = message;
                                                data.failedCount++;
                                            }
                                            else
                                            {
                                                data.avg += res;
                                                data.best = std::min(res, data.best);
                                                data.worst = std::max(res, data.worst);
                                                successCount++;
                                            }
                                            notifyTestHost(testHost, id);
                                        } };
        if (icmpReply->hIcmpFile == INVALID_HANDLE_VALUE)
        {
            data.errorMessage = "IcmpCreateFile failed";
            data.failedCount++;
            notifyTestHost(testHost, req.id);
            delete icmpReply;
            return;
        }
        IcmpSendEcho2(
            icmpReply->hIcmpFile, NULL,
            [](PVOID ctx, PIO_STATUS_BLOCK b, ULONG r)
            {
                static int i = 1;
                LOG("hit" + QSTRN(i++));
                auto replyPtr = reinterpret_cast<ICMPReply *>(ctx);
                auto isSuccess = (NTSTATUS(b->Status)) >= 0;
                long res = 0;
                QString message;
                if (isSuccess)
                {
                    const ICMP_ECHO_REPLY *r = (const ICMP_ECHO_REPLY *) replyPtr->reply_buf;
                    res = r->RoundTripTime * 1000;
                }
                else
                {
                    auto e = GetLastError();
                    DWORD buf_size = 1000;
                    TCHAR buf[1000];
                    GetIpErrorString(e, buf, &buf_size);
                    message = "IcmpSendEcho returned error (" + QString::fromStdWString(buf) + ")";
                }
                replyPtr->whenIcmpFailed(isSuccess, res, message, replyPtr->hIcmpFile);
                delete replyPtr;
            },
            icmpReply, reinterpret_cast<IPAddr &>(reinterpret_cast<sockaddr_in &>(storage).sin_addr), icmpReply->payload, payload_size, NULL, icmpReply->reply_buf,
            reply_buf_size, 10000);
    }
    bool Static_ICMP_LatencyTestEngine::notifyTestHost(LatencyTestHost *testHost, const ::Qv2ray::base::ConnectionId &id)
    {
        if (data.failedCount + successCount == data.totalCount)
        {
            if (data.failedCount == data.totalCount)
                data.avg = LATENCY_TEST_VALUE_ERROR;
            else
                data.errorMessage.clear(), data.avg = data.avg / successCount / 1000;
            testHost->OnLatencyTestCompleted(id, data);
            return true;
        }
        return false;
    }
    void Static_ICMP_LatencyTestEngine::start()
    {
        data.totalCount = 0;
        data.failedCount = 0;
        data.worst = 0;
        data.avg = 0;
        af = isAddr();
        if (af == -1)
        {
            getAddrHandle = loop->resource<uvw::GetAddrInfoReq>();
            sprintf(digitBuffer, "%d", req.port);
        }
        async_DNS_lookup(0, 0);
    }
#endif
} // namespace Qv2rayBase::StaticPlugin
