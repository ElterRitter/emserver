#include "tcpsession.h"

#include <smlog.h>

#include <chrono>

using namespace std::chrono_literals;

TcpSession::TcpSession(io_context &ctx, ErrorCallback cb) :
    m_socket{ctx}
    , onSessionError_{cb}
    , m_protocol{ new CommonProcessor{} }
    , m_inSendMode(false)
{
}

TcpSession::Ptr TcpSession::create(io_context &ctx, ErrorCallback cbErr)
{
    return Ptr( new TcpSession(ctx, cbErr) );
}

void TcpSession::registerReadCallback(ISpecificProcessor *processor)
{
    m_protocol->registerCommonMessageIncomingCallback(std::bind(&ISpecificProcessor::callbackRecievedCommonMessage, processor, std::placeholders::_1));
}

TcpSession::~TcpSession()
{
    if(!m_socket.is_open())
        return;

    try
    {
        onSessionError_ = nullptr;

        LOG_DEBUG << "destructor of session";
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        m_socket.close();
    }
    catch(boost::system::system_error &error)
    {
        LOG_DEBUG << "session closed with exception code " << error.code() << "; what is " << error.what();
    }
}

void TcpSession::waitReading()
{
    LOG_DEBUG << "[TcpSession::waitReading] wait reading";
    auto readHandler = std::bind(&TcpSession::handlerRead, this, std::placeholders::_1, std::placeholders::_2);
    boost::asio::async_read(m_socket, boost::asio::dynamic_buffer(m_buffer), boost::asio::transfer_exactly(1), readHandler);
}

void TcpSession::sendData(const std::string &s)
{
    if(!m_socket.is_open())
        return;

    auto chunk = m_protocol->prepareData(s);
    auto handler = std::bind(&TcpSession::handlerWrite, this, std::placeholders::_1, std::placeholders::_2);

    std::unique_lock<std::mutex> canSend(m_sendSyncMutex);
    while(m_inSendMode)
    {
        std::unique_lock<std::mutex> lg(m_sendAvaliableMutex);
        if(m_cv.wait_for(lg, 500ms) == std::cv_status::no_timeout)
            break;
    }

    m_inSendMode = true;
    std::ostream os(&m_streamSend);
    os.write((char*)chunk.data(), chunk.size());

    boost::asio::async_write(m_socket, m_streamSend, handler);
    //m_streamSend.consume(chunk.size());
}

TcpSession::SocketParameters TcpSession::connectedSocketParameters() const
{
    SocketParameters param{"", 0};
    if(!m_socket.is_open())
        return param;

    try
    {
        auto point = m_socket.remote_endpoint();
        auto address = point.address();
        param.address = address.is_unspecified() ? "" : address.to_string();
        param.port = point.port();
    }
    catch(boost::system::system_error &er)
    {
        param.address.clear();
        param.port = 0;
    }

    return param;
}

TcpSession::SocketParameters TcpSession::localSocketParameters() const
{
    SocketParameters param{"", 0};
    if(!m_socket.is_open())
        return param;

    try
    {
        auto point = m_socket.local_endpoint();
        auto address = point.address();
        param.address = address.is_unspecified() ? "" : address.to_string();
        param.port = point.port();
    }
    catch(boost::system::system_error &er)
    {
        param.address.clear();
        param.port = 0;
    }

    return param;
}

void TcpSession::handlerRead(const boost::system::error_code &code, size_t bytes_recieved)
{
//    LOG_DEBUG() << "[TcpSession::handlerRead] bytes " << bytes_recieved;
    if(code)
    {
        if (code == boost::asio::error::operation_aborted)
            return;

        LOG_ERROR << "[TcpSession::handlerRead] detected error. Code " << code.value() << "; message " << code.message();
        if(onSessionError_)
            onSessionError_(code);

#if defined(_MSC_VER)
        if (code.value() == ERROR_CONNECTION_ABORTED)
        {
            LOG_DEBUG << "[TcpSession::onSessionError] windows connection aborted code";
            return;
        }
#endif

        return;
    }

    auto recived_handler = std::bind(&TcpSession::handlerRead, this, std::placeholders::_1, std::placeholders::_2);
    auto avaliable_bytes = m_socket.available();
    //LOG_DEBUG << "[TransportServer::handlerRead] recieved " << bytes_recieved << " bytes. Avaliable " << avaliable_bytes;
    if(avaliable_bytes == 0)
    {
        avaliable_bytes = 1;
        m_protocol->onReadedChunk(m_buffer.data(), m_buffer.size());
        m_buffer.clear();
    }

    boost::asio::async_read(m_socket, boost::asio::dynamic_buffer(m_buffer), boost::asio::transfer_exactly(avaliable_bytes), recived_handler);
}

void TcpSession::handlerWrite(const boost::system::error_code &code, size_t bytes_transmited)
{
    LOG_DEBUG << "[TcpSession::handlerWrite] bytes written " << bytes_transmited;

    std::unique_lock<std::mutex> lg(m_sendAvaliableMutex);
    m_streamSend.consume(bytes_transmited);
    m_inSendMode = false;
    m_cv.notify_one();
    lg.unlock();

    if(code)
    {
        if (code == boost::asio::error::operation_aborted)
            return;

        if(onSessionError_)
            onSessionError_(code);

        return;
    }
}

