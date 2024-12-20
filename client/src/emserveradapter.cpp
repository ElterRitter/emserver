#include "interop.h"

#include "emserveradapter.h"
#include "emserverfacade.h"

#include "dataconverter.h"

#include <smlog.h>

#include <assert.h>

using namespace emserverproxy;

requestid EmserverAdapter::methodCounter = 1;

EmserverAdapter::EmserverAdapter(EmserverFacade *parent)
    : m_facade(parent)
{
    m_parserProtocol = std::unique_ptr<EmserverProcessor>( new EmserverProcessor(ISpecificProcessor::WorkModeCodes::emserver) );
    
    EmserverProcessor::callbackCapabilityResponse respCap = std::bind(&EmserverAdapter::onCapabilitiesResponse, this, std::placeholders::_1, std::placeholders::_2);
    m_parserProtocol->registerCallback<EmserverProcessor::callbackCapabilityResponse>(EmserverProcessor::ResponseCapabilities, respCap);

    EmserverProcessor::callbackSensorActivated respActivated = std::bind(&EmserverAdapter::onSensorActivatedResponse, this, std::placeholders::_1, std::placeholders::_2);
    m_parserProtocol->registerCallback<EmserverProcessor::callbackSensorActivated>(EmserverProcessor::SensorActivated, respActivated);

    EmserverProcessor::callbackServerRuntimeNotification errHandler = std::bind(&EmserverAdapter::onServerRuntimeNotification, this, std::placeholders::_1, std::placeholders::_2);
    m_parserProtocol->registerCallback<EmserverProcessor::callbackServerRuntimeNotification>(EmserverProcessor::ServerRuntimeNotification, errHandler);

    auto connectedHandler = std::bind(&EmserverAdapter::onClientConnected, this, std::placeholders::_1);
    auto errorHandler = std::bind(&EmserverAdapter::onSessionError, this, std::placeholders::_1);
    m_transport = std::make_unique<TransportClient>( connectedHandler, errorHandler );

    resetCounters();
}

EmserverAdapter::~EmserverAdapter()
{
    m_clientCallbacks.clear();
    connectionClose();
#if defined(USE_CALLBACK_COUNTERS)
    printCounters();
#endif
}

void EmserverAdapter::connectionOpen(const std::string &remoteIP, const uint16_t port)
{
#if defined(USE_CALLBACK_COUNTERS)
    printCounters();
    resetCounters();
#endif

    TransportClient::ConnectionPoint point(remoteIP, port);
    m_transport->openConnection(&point);
}

void EmserverAdapter::connectionClose()
{
#if defined(USE_CALLBACK_COUNTERS)
    printCounters();
    resetCounters();
#endif

    if(m_transport)
    {
        m_transport->closeConnection();
    }

    m_session.reset();
    auto it = m_clientCallbacks.find(MethodCallbackCode::ConnectedStateChanged);
    if (it != m_clientCallbacks.end())
    {
        emserverproxy::ConnectedState st;
        auto callback = boost::any_cast<EmserverFacade::emserverConnectedStateChanging>(it->second);
        callback(m_facade, st);
    }
}

requestid EmserverAdapter::sendRequestCapabilities()
{
    int32_t ret = callCode();
    emserver::RequestCapabilities req;
    auto msg = m_parserProtocol->makeMessage(EmserverProcessor::RequestCapabilites, &req, ret);
    if(msg.empty())
    {
        LOG_ERROR << "[EmserverAdapter::sendRequestCapabilities] empty request";
        ret = EmserverFacade::WrongCallCode;
    }

    if(m_session)
    {
        m_session->sendData(msg);
    }
    else
    {
        LOG_ERROR << "[EmserverAdapter::sendRequestCapabilities] No active session";
        ret = EmserverFacade::InvalidSessionError;
    }

    return ret;
}

requestid EmserverAdapter::sendSensorsControl(const Sensors &sensors)
{
    int32_t ret = callCode();
    emserver::SensorsControl req = DataConverter::sensorsToProto(sensors);
    auto msg = m_parserProtocol->makeMessage(EmserverProcessor::SensorsConfigurationControl, &req, ret);
    if(msg.empty())
    {
        LOG_ERROR << "[EmserverAdapter::sendSensorsControl] empty request";
        ret = EmserverFacade::WrongCallCode;
    }

    if(m_session)
    {
        m_session->sendData(msg);
    }
    else
    {
        LOG_ERROR << "[EmserverAdapter::sendSensorsControl] No active session";
        ret = EmserverFacade::InvalidSessionError;
    }

    return ret;
}


#if defined(USE_CALLBACK_COUNTERS)
void EmserverAdapter::printCounters()
{
    LOG_INFO << "Callbacks statistic:";
    for(const auto &item : m_callbackCounters)
    {
        LOG_INFO << " - callback " << codeToString(item.first) << " called " << item.second << " times";
    }
    LOG_INFO << "End callbacks statistic";
}
#endif

void EmserverAdapter::onClientConnected(::TcpSession::Ptr session)
{
    m_session = session;
    auto it = m_clientCallbacks.find(MethodCallbackCode::ConnectedStateChanged);
    if(it != m_clientCallbacks.end())
    {
        auto callback = boost::any_cast<EmserverFacade::emserverConnectedStateChanging>(it->second);
        if(m_session)
        {
            emserverproxy::ConnectedState st;
            auto params = m_session->localSocketParameters();
            st.localPoint.address = params.address;
            st.localPoint.port = params.port;

            params = m_session->connectedSocketParameters();
            st.remotePoint.address = params.address;
            st.remotePoint.port = params.port;

            callback(m_facade, st);
            appendCounter(MethodCallbackCode::ConnectedStateChanged);
        }
    }

    if (m_session)
    {
        m_session->registerReadCallback(m_parserProtocol.get());
        m_session->waitReading();
    }
}

void EmserverAdapter::onSessionError(const error_code code)
{
    m_session.reset();
    if(code == boost::asio::error::operation_aborted)
        return;

    if(code == boost::asio::error::operation_not_supported)
        return;

    if(code == boost::asio::error::connection_aborted)
        return;

    auto it = m_clientCallbacks.find(MethodCallbackCode::ConnectedStateChanged);
    if(it != m_clientCallbacks.end())
    {
        auto callback = boost::any_cast<EmserverFacade::emserverConnectedStateChanging>(it->second);

        emserverproxy::ConnectedState st;
        callback(m_facade, st);
    }

#if defined(USE_CALLBACK_COUNTERS)
    printCounters();
    resetCounters();
#endif

    if(m_transport)
    {
        m_transport->openConnection();
    }
}

void EmserverAdapter::onCapabilitiesResponse(const emserver::Capabilities &caps, const int64_t &id)
{
    LOG_DEBUG << "[ClientProcessor::onCapabilitiesResponse]";
    auto it = m_clientCallbacks.find(MethodCallbackCode::ResponseCapabilities);
    if(it != m_clientCallbacks.end())
    {
        auto ptr = boost::any_cast<EmserverFacade::emserverResponseCapabilities>(it->second);
        auto proxyCaps = DataConverter::fromCaps(caps);
        ptr(m_facade, proxyCaps, id);
        appendCounter(MethodCallbackCode::ResponseCapabilities);
    }
}

void EmserverAdapter::onSensorActivatedResponse(const emserver::SensorActivated &sensor, const int64_t &id)
{
    LOG_DEBUG << "[EmserverAdapter::onSensorActivatedResponse]";
    auto it = m_clientCallbacks.find(MethodCallbackCode::SensorActivated);
    if(it != m_clientCallbacks.end())
    {
        auto ptr = boost::any_cast<EmserverFacade::emserverResponseSensorActivation>(it->second);
        auto proxySensor = DataConverter::nativeActivatedSensor(sensor);
        ptr(m_facade, proxySensor, id);
        appendCounter(MethodCallbackCode::SensorActivated);
    }
}


void EmserverAdapter::onServerRuntimeNotification(const emserver::ServerRuntimeNotification &error, const int64_t &id)
{
    LOG_DEBUG << "[EmserverAdapter::onServerRuntimeNotification]";
    auto it = m_clientCallbacks.find(MethodCallbackCode::ResponseServerError);
    if(it != m_clientCallbacks.end())
    {
        auto ptr = boost::any_cast<EmserverFacade::emserverServerRuntimeNotification>(it->second);

        ErrorDescriptor erDescr;
        erDescr.description = error.has_description() ? error.description() : "";
        erDescr.errorCode = error.code();
        erDescr.methodCode = error.methodid();
        ptr(m_facade, erDescr);
        appendCounter(MethodCallbackCode::ResponseServerError);
    }
}

void EmserverAdapter::appendCounter(const MethodCallbackCode code)
{
#if defined(USE_CALLBACK_COUNTERS)
    m_callbackCounters[code] += 1;
#endif
}

const string EmserverAdapter::codeToString(const MethodCallbackCode code) const
{
    std::string ret;
    switch(code)
    {
    case ConnectedStateChanged: ret.append("ConnectedStateChanged"); break;
    case ResponseCapabilities: ret.append("ResponseCapabilities"); break;
    case ResponseServerError: ret.append("ResponseServerError"); break;
    case SensorActivated: ret.append("SensorActivated"); break;
    }

    return ret;
}

void EmserverAdapter::resetCounters()
{
#if defined(USE_CALLBACK_COUNTERS)
    m_callbackCounters[ConnectedStateChanged] = 0;
    m_callbackCounters[ResponseCapabilities] = 0;
    m_callbackCounters[SensorActivated] = 0;
    m_callbackCounters[ResponseServerError] = 0;
#endif
}
