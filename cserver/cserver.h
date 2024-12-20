#ifndef CSERVER_H
#define CSERVER_H

#include "api_server.h"


#if defined __cplusplus

#include "processoremserver.h"
#include "transport.h"

#include <memory>

class CServer
{
public:
    using Ptr = std::shared_ptr<CServer>;
    CServer();
    int listen(const CNIpAddress *ip, int port);
    void close();
    bool setLoggingLevel(const ServerLoggingLevel &lvl);
    void setLoggingPath(const CNString *cnPath);
    void runtimeError(const int32_t &code, ErrorDescription &err);

    void setClientConnectedCallback(const cbClientConnected &cb);
    void setCapsRequestCallback(cbCapabilitiesRequest cb);
    void setSensorsControlCallback(const cbSensorControlrequest &cb);

    void setLoggingCallback(const cbLog &cb);

    int sendCapabilities(const requestid &id, const Capabilities &caps);
    int sendActivatedSensorState(const requestid &id, const SensorBase *pActivatedSensorConfig);


private:
    std::unique_ptr<TransportServer> m_server;
    std::unique_ptr<EmserverProcessor> m_processor;
    TcpSession::Ptr m_session;

    // extern callback functions
    cbLog onLog_;
    cbClientConnected onClientConnected_;
    cbCapabilitiesRequest onClientCapsRequest_;
    cbSensorControlrequest onSensorControlRequest_;



#if defined(USE_COUNTERS_OF_CLIENTCALLBACK)
    std::map<uint16_t, uint64_t> m_clientCounters;
#endif
    void printStatistics();
    void resetCounters();
    void appendCounter(int code);

    void transportClientConnectedCallback(TcpSession::Ptr ptrSession);
    void callbackSessionError(const boost::system::error_code &code);
    void callbackEmserverCapsRequest(const emserver::RequestCapabilities &req, const int64_t &id);
    void callbackEmserverSensorControlRequest(const emserver::SensorsControl &req, const int64_t &id);

    Sensors* nativeSensors(const emserver::SensorsControl &protoSensors);
    emserver::Capabilities protoSensors(const Sensors* pSensors);
    void freeSensors(Sensors* sensors);

    emserver::SensorActivated activeSensorFromNative(const SensorBase *pSensor);


    void sendMethodCallResult(const uint32_t &callID, const int32_t &code);
    void logging(const string &logString);
};

// using servermap = std::map<int, CServer::Ptr>;
// servermap *servers;
//int counter = 0;
#endif

#endif
