#include "transport_client.h"
#include "smlog.h"

using boost::asio::ip::tcp;
using namespace emserverproxy;

constexpr uint32_t reconnect_timeout_sec = 5;

TransportClient::TransportClient(const string &interface, const uint16_t &port)
    : m_ptrResolver{nullptr}
    , m_callbackConnected(nullptr)
    , m_callbackError(nullptr)
{
    m_conf.interface = interface;
    m_conf.port = port;

    m_ioService = std::make_unique<IoService>();
    m_ioServiceWork = std::make_unique<IoServiceWork>(*m_ioService.get());

    auto binder = boost::bind(&boost::asio::io_service::run, m_ioService.get());
    boost::asio::post(m_threadPool, binder);
}

TransportClient::TransportClient(const ConnectionPoint &&conf)
    : m_conf{conf}
    , m_ptrResolver{nullptr}
    , m_callbackConnected(nullptr)
    , m_callbackError(nullptr)
{
    m_ioService = std::make_unique<IoService>();
    m_ioServiceWork = std::make_unique<IoServiceWork>(*m_ioService.get());

    auto binder = boost::bind(&boost::asio::io_service::run, m_ioService.get());
    boost::asio::post(m_threadPool, binder);
}

TransportClient::TransportClient(const ConnectedCallback &&clb, const SessionErrorCallback &&errCb) :
    m_ptrResolver{nullptr}
    , m_callbackConnected{clb}
    , m_callbackError{errCb}
{
    m_ioService = std::make_unique<IoService>();
    m_ioServiceWork = std::make_unique<IoServiceWork>(*m_ioService.get());

    auto binder = boost::bind(&boost::asio::io_service::run, m_ioService.get());
    boost::asio::post(m_threadPool, binder);
}

TransportClient::~TransportClient()
{
    if(m_ioServiceWork)
        m_ioServiceWork.reset();

    m_threadPool.join();
    m_ioService->reset();
}

bool TransportClient::openConnection(const ConnectionPoint *pDestPoint)
{
    if(pDestPoint != nullptr)
    {
        m_conf = *pDestPoint;
    }

    auto tmp = m_ioService.get();

    auto session = TcpSession::create(*tmp, m_callbackError);

    if (!m_ptrResolver)
    {
        m_ptrResolver = std::make_unique<tcp::resolver>( *m_ioService.get() );
    }

    auto resolve_handler = std::bind(&TransportClient::onHandlerResolved, this, std::placeholders::_1, std::placeholders::_2, session);
    m_ptrResolver->async_resolve(m_conf.interface, std::to_string(m_conf.port), resolve_handler );

    if(!m_reconnectTimer)
        m_reconnectTimer = std::unique_ptr<boost::asio::deadline_timer>( new boost::asio::deadline_timer(*m_ioService.get() ));

    m_reconnectTimer->expires_from_now(boost::posix_time::seconds(reconnect_timeout_sec));
    m_reconnectTimer->async_wait(std::bind(&TransportClient::onReconnectTimerTimeout, this, std::placeholders::_1));

    return true;
}

void TransportClient::closeConnection()
{
    if(m_reconnectTimer)
        m_reconnectTimer->cancel();
}

void TransportClient::onReconnectTimerTimeout(const error_code &c)
{
    if(c == boost::asio::error::operation_aborted)
    {
        LOG_INFO << "Reconnect timer stoped: operation aborted";
        return;
    }
    else if(c)
    {
        LOG_ERROR << "ReconnectTimer has unhandled error detected. Code " << c << ": " << c.message();
        return;
    }

    openConnection();
}

void TransportClient::onHandlerResolved(const error_code &erCode, boost::asio::ip::tcp::resolver::results_type results, TcpSession::Ptr session)
{
    if(erCode)
    {
        LOG_ERROR << "Network node resolving error. Code " << erCode << ": " << erCode.message();
        if(m_callbackError)
            m_callbackError(erCode);

    }
    else
    {
        LOG_DEBUG << "Network node resolved. Try to connect.";
        auto connect_handler = std::bind(&TransportClient::onHandlerAccepted, this, std::placeholders::_1, session);
        boost::asio::async_connect(session->socket(), results.begin(), connect_handler);
    }
}

void TransportClient::onHandlerAccepted(const error_code &erCode, TcpSession::Ptr session)
{
    if(erCode)
    {
        LOG_ERROR << "Client connector acuquired error " << erCode << ": " << erCode.message();
        return;
    }

    if(m_reconnectTimer)
        m_reconnectTimer->cancel();

    if (m_callbackConnected)
        m_callbackConnected(session);
}
