#pragma once

#include "transport_client.h"
#include <processoremserver.h>
#include "interop.h"

#include <boost/any.hpp>

#include <memory>
#include <string>
#include <cstdint>
#include <map>

namespace emserverproxy {

class TransportClient;
class TcpSession;
class EmserverFacade;

class EmserverAdapter
{
public:
    enum MethodCallbackCode
    {
        ConnectedStateChanged,
        ResponseServerError,
        ResponseCapabilities,
        SensorActivated
    };
    explicit EmserverAdapter(EmserverFacade *parent = nullptr);
    ~EmserverAdapter();

    void connectionOpen(const std::string &remoteIP, const uint16_t port);
    void connectionClose();

    template<typename T>
    void registerCallback(const MethodCallbackCode code, const T callbackFuncPtr)
    { m_clientCallbacks[code] = callbackFuncPtr; }

    requestid sendRequestCapabilities();
    requestid sendSensorsControl(const Sensors& sensors);


private:
    using TransportPtr = std::unique_ptr<TransportClient>;
    using ParserPtr = std::unique_ptr<EmserverProcessor>;
    using CallbackContainer = std::map<MethodCallbackCode, boost::any>;

    static requestid methodCounter;

    TransportPtr m_transport;
    ::TcpSession::Ptr m_session;
    ParserPtr m_parserProtocol;
    CallbackContainer m_clientCallbacks;
    EmserverFacade *m_facade;
#if defined(USE_CALLBACK_COUNTERS)
    std::map<MethodCallbackCode, uint64_t> m_callbackCounters;
    void printCounters();
#endif
    void appendCounter(const MethodCallbackCode code);
    void resetCounters();

    // callback functions
    void onClientConnected(::TcpSession::Ptr session);
    void onSessionError(const boost::system::error_code code);

    // callback protocol processing
    void onCapabilitiesResponse(const emserver::Capabilities &caps, const int64_t &id);
    void onSensorActivatedResponse(const emserver::SensorActivated &sensor, const int64_t &id);
    void onServerRuntimeNotification(const emserver::ServerRuntimeNotification &error, const int64_t &id);

    requestid callCode() const { return methodCounter++; };
    const std::string codeToString(const MethodCallbackCode code) const;
};

}
