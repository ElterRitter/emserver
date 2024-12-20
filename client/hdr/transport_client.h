#pragma once
#include <tcpsession.h>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <string>
#include <memory>
#include <cstdint>

using std::string;
using boost::asio::ip::tcp;
using boost::system::error_code;

namespace emserverproxy {

class TransportClient
{
public:
    using ConnectedCallback = std::function<void(TcpSession::Ptr)>;
    using SessionErrorCallback = std::function<void(const boost::system::error_code &)>;

    struct ConnectionPoint
    {
        string interface;
        uint32_t port;

        ConnectionPoint() { port = 0; }
        ConnectionPoint(const std::string &destIP, const uint32_t destPort) : interface(destIP), port(destPort) { }
    };

    TransportClient(const string &interface, const uint16_t &port);
    TransportClient(const ConnectionPoint && conf);
    TransportClient(const ConnectedCallback &&clb, const SessionErrorCallback &&errCb);
    ~TransportClient();

    bool openConnection(const ConnectionPoint *pDestPoint = nullptr);
    void closeConnection();

private:
    using ThreadPool = boost::asio::thread_pool;
    using IoService = boost::asio::io_service;
    using IoServicePtr = std::unique_ptr<IoService>;
    using IoServiceWork = boost::asio::io_service::work;
    using IoServiceWorkPtr = std::unique_ptr<IoServiceWork>;
    using TcpResolver = std::unique_ptr<boost::asio::ip::tcp::resolver>;
    using DlTimer = boost::asio::deadline_timer;
    using DlTimerPtr = std::unique_ptr<DlTimer>;

    ConnectionPoint m_conf;
    TcpResolver m_ptrResolver;
    ThreadPool m_threadPool;
    IoServicePtr m_ioService;
    IoServiceWorkPtr m_ioServiceWork;

    ConnectedCallback m_callbackConnected;
    SessionErrorCallback m_callbackError;

    DlTimerPtr m_reconnectTimer;

    void onReconnectTimerTimeout(const boost::system::error_code &c);
    void onHandlerAccepted(const error_code &erCode, TcpSession::Ptr session);
    void onHandlerResolved(const boost::system::error_code &erCode, boost::asio::ip::tcp::resolver::results_type results, TcpSession::Ptr session);
};

}   // namespace emserverproxy
