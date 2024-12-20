#pragma once
#include "ispecificprocessor.h"
#include "commonprocessor.h"

#include <boost/asio.hpp>
#include <mutex>

#include <memory>
#include <vector>
#include <string>

#include <iostream>

using std::cout;
using std::endl;

using namespace boost::asio;
using namespace boost::asio::ip;

class TcpSession
{
public:
    using Ptr = std::shared_ptr<TcpSession>;
    using WPtr = std::weak_ptr<TcpSession>;
    using ErrorCallback = std::function<void(const boost::system::error_code &code)>;
    using callbackReaded = std::function<void(const char* pData, const size_t dataSize)>;

    struct SocketParameters
    {
        std::string address;
        uint16_t port;
    };

    static Ptr create(io_context &ctx, ErrorCallback cbErr);
    void registerReadCallback(ISpecificProcessor *processor);
    tcp::socket &socket() { return m_socket; }

    ~TcpSession();

    void waitReading();
    void sendData(const std::string &s);

    SocketParameters connectedSocketParameters() const;
    SocketParameters localSocketParameters() const;

private:
    tcp::socket m_socket;
    std::vector<char> m_buffer;
    ErrorCallback onSessionError_;
    boost::asio::streambuf m_streamSend;
    std::unique_ptr<CommonProcessor> m_protocol;
    std::mutex m_sendSyncMutex;
    std::mutex m_sendAvaliableMutex;
    std::condition_variable m_cv;
    bool m_inSendMode;

    explicit TcpSession(io_context &ctx, ErrorCallback cb);
    void handlerRead(const boost::system::error_code &code, size_t bytes_recieved);
    void handlerWrite(const boost::system::error_code &code, size_t bytes_transmited);
};

