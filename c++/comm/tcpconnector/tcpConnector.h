#pragma once
#include "iConnector.h"
#include "tcpinit.h"
#include <string>
#include <atomic>
#include <mutex>
#include <ws2tcpip.h> // inet_pton

namespace __ENF::nsconnector_v2
{
    class tcpConnector :
        public iConnector
        , public tcpInit
    {
    public:
        tcpConnector(const char* ip, int port, int connectiontimeoutMilisec = 3000);
        virtual ~tcpConnector();

        bool connect() override;
        void shutdownconnection(); // 핸들은 유지하되, 송수신 중단.
        void disconnect() override;
        bool isconnect() override;

		size_t			send(std::vector<unsigned char> vdata, int param = 0) override;
		std::int64_t	recv(unsigned char* buffer, size_t buflen) override;
        bool            waitrecvsignal(int timeoutMs = 0) override final;

        void setOption(SOCKET& s);
    protected:
        bool waitsocketConnected(const SOCKET& s, long ntimeoutsec, long ntimeoutMilisec);

    private:
        std::mutex m_closelock;
        std::atomic<SOCKET> m_socket;
        std::atomic_bool m_closed;
        std::string m_ip;
        int m_port;
        int m_timeoutMs;
    };
}