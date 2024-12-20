#include "transport.h"

#include <smlog.h>

#include <boost/bind/bind.hpp>

using boost::asio::ip::address;


TransportServer::TransportServer(const string &interface, const uint16_t &port, SessionErrorCallback cb) :
    m_cbConnected(nullptr)
    , m_cbSessionError(cb)
{
    m_conf.interface = interface;
    m_conf.port = port;

    m_ioService = std::make_unique<IoService>();
    m_ioServiceWork = std::make_unique<IoServiceWork>(*m_ioService.get());

    auto binder = boost::bind(&boost::asio::io_service::run, m_ioService.get());
    boost::asio::post(m_threadPool, binder);
}

TransportServer::TransportServer(const Configuration &&conf, SessionErrorCallback cb) :
    m_conf{conf}
    , m_cbConnected(nullptr)
    , m_cbSessionError(cb)
{
    m_ioService = std::make_unique<IoService>();
    m_ioServiceWork = std::make_unique<IoServiceWork>(*m_ioService.get());

    auto binder = boost::bind(&boost::asio::io_service::run, m_ioService.get());
    boost::asio::post(m_threadPool, binder);
}

TransportServer::~TransportServer()
{
    m_cbSessionError = nullptr;

    if(m_acceptor)
    {
        boost::system::error_code er;
        m_acceptor->cancel(er);
        m_acceptor.reset();
    }

    if(m_ioServiceWork)
        m_ioServiceWork.reset();

    m_threadPool.join();
    m_ioService->reset();
}

void TransportServer::setConnectedCallback(ConnectedCallback fp)
{
    m_cbConnected = fp;
}

bool TransportServer::listen(bool needListen)
{
    bool ret = false;
    boost::system::error_code ec;
    if(needListen)
    {
        auto iface = boost::asio::ip::make_address(m_conf.interface, ec);
        if(ec)
        {
            LOG_INFO << "Invalid listen interface ( " << m_conf.interface << " ). Error code: " << ec.value();
            return ret;
        }

        try
        {
            tcp::endpoint ep{iface, m_conf.port};
            m_acceptor = std::shared_ptr<tcp::acceptor>( new tcp::acceptor(*m_ioService.get(), ep, false) );
        }
        catch(boost::system::system_error &se)
        {
            LOG_INFO << "Acceptor exeption. Code " << se.code();
            return ret;
        }

        LOG_INFO << "listening ...";
        ret = true;
    }
    else
    {
        if(m_acceptor)
        {
            m_acceptor->cancel(ec);
            m_acceptor.reset();
        }

        if(ec)
            LOG_ERROR << "[TransportServer::listen] closing listen port with error " << ec;
    }

    return ret;
}

void TransportServer::acceptNewConnection()
{
    if(m_acceptor)
    {
        TcpSession::Ptr session = TcpSession::create(*m_ioService.get(), m_cbSessionError);
        auto accepted_handler = std::bind(&TransportServer::handlerAccept, this, std::placeholders::_1, session);
        m_acceptor->async_accept(session->socket(), accepted_handler);
    }
}

void TransportServer::handlerAccept( const error_code &erCode, TcpSession::Ptr session )
{
    if(erCode)
    {
        LOG_ERROR << "[TransportServer::handlerAccept] error occuqared. Code " << erCode.value() << " ; message " << erCode.message();
        if(erCode == boost::asio::error::operation_aborted)
            return;
    }

    if(m_cbConnected != nullptr)
        m_cbConnected(session);
}
