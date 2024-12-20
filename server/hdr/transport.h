#pragma once

#include "tcpsession.h"

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>

#include <memory>
#include <string>
#include <cstdint>
#include <functional>

using std::string;
using boost::asio::ip::tcp;
using boost::system::error_code;
using std::function;

class TransportServer
{
public:
    using ConnectedCallback = std::function<void(TcpSession::Ptr)>;
    using SessionErrorCallback = std::function<void(const boost::system::error_code &)>;

    struct Configuration
    {
        string interface;
        uint16_t port;

        Configuration() { port = 0; }
    };

    TransportServer(const string &interface, const uint16_t &port, SessionErrorCallback cb);
    TransportServer(const Configuration &&conf, SessionErrorCallback cb);
    ~TransportServer();

    void setConnectedCallback(ConnectedCallback fp);
    bool listen(bool needListen);
    void acceptNewConnection();

private:
    using ThreadPool = boost::asio::thread_pool;
    using IoService = boost::asio::io_service;
    using IoServicePtr = std::unique_ptr<IoService>;
    using IoServiceWork = boost::asio::io_service::work;
    using IoServiceWorkPtr = std::unique_ptr<IoServiceWork>;

    Configuration m_conf;
    std::shared_ptr<tcp::acceptor> m_acceptor;
    ThreadPool m_threadPool;
    IoServicePtr m_ioService;
    IoServiceWorkPtr m_ioServiceWork;

    ConnectedCallback m_cbConnected;
    SessionErrorCallback m_cbSessionError;

    void handlerAccept(const error_code &erCode, TcpSession::Ptr session);
};
