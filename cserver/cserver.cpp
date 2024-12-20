#include "cserver.h"
#include <chrono>

CServer::Ptr server;

bool server_create()
{
    server = CServer::Ptr( new CServer() );
    return true;
}

void server_update_logpath(CNString *path)
{
    if(!server)
        return;

    if(!path)
        return;

    if((path->pStringData == nullptr) || (path->size == 0))
        return;

    server->setLoggingPath(path);
}

bool server_update_loglevel(uint32_t lvl)
{
    bool ret = false;
    if(lvl >= ServerLoggingLevel::MaxValue)
        return ret;

    if(server)
        ret = server->setLoggingLevel(static_cast<ServerLoggingLevel>(lvl));

    return ret;
}

void server_register_callback_Logging(cbLog cb)
{
    if(server)
    {
        server->setLoggingCallback(cb);
    }
}

int server_register_callback_ClientConnected(cbClientConnected cb)
{
    int ret = 0;
    if(server)
    {
        server->setClientConnectedCallback(cb);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int server_register_callback_CapabilitiesRequest(cbCapabilitiesRequest cb)
{
    int ret = 0;
    if(server)
    {
        server->setCapsRequestCallback(cb);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int server_register_callback_SensorsControlRequest(cbSensorControlrequest &cb)
{
    int ret = 0;
    if(server)
    {
        server->setSensorsControlCallback(cb);
    }
    else
    {
        ret = -1;
    }

    return ret;
}

int server_listen(CNIpAddress *address, int port)
{
    int ret = -1;
    if(server)
        ret = server->listen(address, port);

    return ret;
}

void server_runtime_error(int32_t code, ErrorDescription errorDescription)
{
    if(server)
        server->runtimeError(code, errorDescription);
}

void server_close()
{
    if(server)
        server->close();
}

void server_release()
{
    if(server)
        server.reset();
}

int send_capabilities(const requestid id, const Capabilities caps)
{
    int ret = -1;
    if(server)
        ret = server->sendCapabilities(id, caps);

    return ret;
}

int send_activesensor(const requestid id, const SensorBase *pSensor)
{
    int ret = -1;
    if(server)
        ret = server->sendActivatedSensorState(id, pSensor);

    return ret;
}


#if defined __cplusplus
#include <string>
#include <functional>

CServer::CServer() :
    m_processor{ new EmserverProcessor(1) }
    , onLog_(nullptr)
    , onClientConnected_(nullptr)
    , onClientCapsRequest_(nullptr)
    , onSensorControlRequest_(nullptr)

{
    auto handlerCaps = std::bind(&CServer::callbackEmserverCapsRequest, this, std::placeholders::_1, std::placeholders::_2);
    m_processor->registerCallback<EmserverProcessor::callbackCapabilityRequest>(EmserverProcessor::RequestCapabilites, handlerCaps);

    auto handlerSensorControl = std::bind(&CServer::callbackEmserverSensorControlRequest, this, std::placeholders::_1, std::placeholders::_2);
    m_processor->registerCallback<EmserverProcessor::callbackSensorControl>(EmserverProcessor::SensorControl, handlerSensorControl);
}

int CServer::listen(const CNIpAddress *ip, int port)
{
    int ret = 0;
    std::string strIp(ip->pAddress, ip->size);
    logging(std::string("Try to create listen server at interface ") + strIp + ":" + std::to_string(port));

    TransportServer::SessionErrorCallback cbErr = std::bind(&CServer::callbackSessionError, this, std::placeholders::_1);
    m_server = std::unique_ptr<TransportServer>( new TransportServer(strIp, port, cbErr) );
    m_server->setConnectedCallback(std::bind(&CServer::transportClientConnectedCallback, this, std::placeholders::_1));
    bool isListen = m_server->listen(true);
    if(isListen)
    {
        m_server->acceptNewConnection();
        logging("Server ready to accept new connection");
    }
    else
        ret = -2;

    return ret;
}

void CServer::close()
{
    logging("CServer close");

    printStatistics();
    if(m_server)
    {
        logging("Server not listening for incoming connections");
        m_server->listen(false);
    }

    if(m_session)
    {
        m_session.reset();
        logging("Session resetted");
    }

    logging("CServer closed");
}

bool CServer::setLoggingLevel(const ServerLoggingLevel &lvl)
{
    bool ret = true;
#if defined USE_LIB_LOGGING
    log4cpp::Priority::PriorityLevel logLvl = log4cpp::Priority::INFO;
    switch(lvl)
    {
    case SrvDebug: logLvl = log4cpp::Priority::DEBUG; break;
    case SrvInfo: logLvl = log4cpp::Priority::INFO; break;
    case SrvWarning: logLvl = log4cpp::Priority::WARN; break;
    case SrvError: logLvl = log4cpp::Priority::ERROR; break;
    case SrvFatal: logLvl = log4cpp::Priority::FATAL; break;
    default: ret = false;
    }

    SimpleSmallLogger::instance().setLoggingLevel(logLvl);
#endif
    return ret;
}

void CServer::setLoggingPath(const CNString *cnPath)
{
#if defined USE_LIB_LOGGING
    std::string path(cnPath->pStringData, cnPath->size);
    path.append("/cnserver.log");
    SimpleSmallLogger::instance().addFileAppender(path);
#endif
}

void CServer::runtimeError(const int32_t &code, ErrorDescription &err)
{
    if(!m_session)
    {
        logging("[CServer::runtimeError] invalid session");
        return;
    }

    emserver::ServerRuntimeNotification msg;
    msg.set_methodid(0);
    msg.set_code(code);

    std::string str(err.pError, err.size);
    msg.set_description(str);
    m_session->sendData(m_processor->makeMessage(EmserverProcessor::ServerRuntimeNotification, &msg));
    logging("[CServer::runtimeError] code " + std::to_string(code) + "; Description: " + str);
}

void CServer::setClientConnectedCallback(const cbClientConnected &cb)
{
    onClientConnected_ = cb;
}

void CServer::setCapsRequestCallback(cbCapabilitiesRequest cb)
{
    onClientCapsRequest_ = cb;
}

void CServer::setSensorsControlCallback(const cbSensorControlrequest &cb)
{
    onSensorControlRequest_ = cb;
}


void CServer::setLoggingCallback(const cbLog &cb)
{
    onLog_ = cb;
}


int CServer::sendCapabilities(const requestid &id, const Capabilities &caps)
{
    int ret = -1;
    auto strId = std::to_string(id);
    emserver::Capabilities sCaps = protoSensors(&caps.sensors);
    if(sCaps.IsInitialized() && m_session)
    {
        logging("[CServer::sendCapabilities] send current caps to reqid " + strId);
        m_session->sendData(m_processor->makeMessage(EmserverProcessor::ResponseCapabilities, &sCaps, id));
        appendCounter(EmserverProcessor::ResponseCapabilities);
        ret = 0;
    }
    else
    {
        logging("[CServer::sendCapabilities] caps is invalid or invalid session. Request id " + strId + "; Send error code " + std::to_string(ret));
        if(m_session)
            sendMethodCallResult(id, ret);
    }

    return ret;
}

int CServer::sendActivatedSensorState(const requestid &id, const SensorBase *pActivatedSensorConfig)
{
    int ret = -1;
    auto strId = std::to_string(id);

    emserver::SensorActivated msg = activeSensorFromNative(pActivatedSensorConfig);
    if(msg.IsInitialized() && m_session)
    {
        logging("[CServer::sendActivatedSensorState] send current value of sensor to client");
        m_session->sendData(m_processor->makeMessage(EmserverProcessor::SensorActivated, &msg, id));
        appendCounter(EmserverProcessor::SensorActivated);
        ret = 0;
    }
    else
    {
        logging("[CServer::sendActivatedSensorState] sensor state is invalid or invalid session. Request id is " + strId + "; Send code " + std::to_string(ret));
        if(m_session)
            sendMethodCallResult(id, ret);
    }

    return ret;
}


void CServer::printStatistics()
{
#if defined(USE_COUNTERS_OF_CLIENTCALLBACK)
    logging("[CServer::printStatistics] of clients callbacks");
    logging("[CServer::printStatistics] ResponseCapabilities count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseCapabilities]));
    logging("[CServer::printStatistics] ResponseDiagnostic count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseDiagnostic]));
    logging("[CServer::printStatistics] ResponseTasks count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseTasks]));
    logging("[CServer::printStatistics] SessionData count " + std::to_string(m_clientCounters[EmserverProcessor::SessionData]));
    logging("[CServer::printStatistics] ResponseSpectr count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseSpectr]));
    logging("[CServer::printStatistics] ResponseBearing count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseBearing]));
    logging("[CServer::printStatistics] ResponseSoundChannelsState count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseSoundChannelsState]));
    logging("[CServer::printStatistics] ResponseTasksFrequencies count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseTasksFrequencies]));
    logging("[CServer::printStatistics] ResponseFTDS count " + std::to_string(m_clientCounters[EmserverProcessor::ResponseFTDS]));
    logging("[CServer::printStatistics] ServerRuntimeNotification count " + std::to_string(m_clientCounters[EmserverProcessor::ServerRuntimeNotification]));
#endif
}

void CServer::resetCounters()
{
#if defined(USE_COUNTERS_OF_CLIENTCALLBACK)
    m_clientCounters[EmserverProcessor::ResponseCapabilities] = 0;
    m_clientCounters[EmserverProcessor::ResponseDiagnostic] = 0;
    m_clientCounters[EmserverProcessor::ResponseTasks] = 0;
    m_clientCounters[EmserverProcessor::SessionData] = 0;
    m_clientCounters[EmserverProcessor::ResponseSpectr] = 0;
    m_clientCounters[EmserverProcessor::ResponseBearing] = 0;
    m_clientCounters[EmserverProcessor::ResponseSoundChannelsState] = 0;
    m_clientCounters[EmserverProcessor::ResponseTasksFrequencies] = 0;
    m_clientCounters[EmserverProcessor::ResponseFTDS] = 0;
    m_clientCounters[EmserverProcessor::ServerRuntimeNotification] = 0;
#endif
}


void CServer::appendCounter(int code)
{
#if defined(USE_COUNTERS_OF_CLIENTCALLBACK)
    m_clientCounters[code] += 1;
#endif
}


void CServer::transportClientConnectedCallback(TcpSession::Ptr ptrSession)
{
    logging("[CServer::transportClientConnectedCallback] is new session " + std::to_string(ptrSession != nullptr));
    m_session = ptrSession;
    m_server->listen(false);

    auto params = m_session->connectedSocketParameters();
    m_session->registerReadCallback(m_processor.get());
    m_session->waitReading();
    if(onClientConnected_)
    {
        CNIpAddress *ip = new CNIpAddress();
        ip->size = params.address.size();
        ip->pAddress = new char[ip->size+1];
        memset(ip->pAddress, 0, ip->size+1);
        memcpy(ip->pAddress, params.address.c_str(), ip->size);

        onClientConnected_(ip, params.port);

        delete [] ip->pAddress;
        delete ip;
    }
}

void CServer::callbackSessionError(const error_code &code)
{
    m_session.reset();
    if(code == boost::asio::error::operation_aborted)
    {
        logging("[CServer::callbackSessionError] Session aborted");
        return;
    }

    if(code == boost::asio::error::operation_not_supported)
    {
        logging("[CServer::callbackSessionError] Session error operation_not_supported");
        return;
    }

    if(code == boost::asio::error::connection_aborted)
    {
        logging("[CServer::callbackSessionError] connection aborted");
        return;
    }

    printStatistics();
    resetCounters();

    logging("[CServer::callbackSessionError] detected session error. Code " + std::to_string(code.value()));
    if(onClientConnected_)
    {
        CNIpAddress *ip = new CNIpAddress;
        onClientConnected_(ip, 0);
        delete ip;
    }

    if(m_server->listen(true))
        m_server->acceptNewConnection();
}

void CServer::callbackEmserverCapsRequest(const emserver::RequestCapabilities &req, const int64_t &id)
{
    auto strId = std::to_string(id);
    int32_t callbackRetCode = -2;
    if(!req.IsInitialized())
    {
        logging("[CServer::callbackEmserverCapsRequest] request not initialized. By reqid " + strId);
        sendMethodCallResult(id, callbackRetCode);
        return;
    }

    if(onClientCapsRequest_)
    {
        logging("[CServer::callbackEmserverCapsRequest] call callback by reqid " + strId);
        callbackRetCode = onClientCapsRequest_(id);
    }
    else
    {
        callbackRetCode = -1;
        logging("no server callback registered for caps request by reqid " + strId);
    }

    sendMethodCallResult(id, callbackRetCode);
}

void CServer::callbackEmserverSensorControlRequest(const emserver::SensorsControl &req, const int64_t &id)
{
    auto strId = std::to_string(id);
    int32_t callbackCode = -2;
    if(!req.IsInitialized())
    {
        logging("[CServer::callbackEmserverSensorControlRequest] request not initialized. By reqid " + strId);
        sendMethodCallResult(id, callbackCode);
        return;
    }

    if(onSensorControlRequest_)
    {
        logging("[CServer::callbackEmserverSensorControlRequest] call callback by reqid " + strId);
        auto nativeConfig = nativeSensors(req);
        callbackCode = onSensorControlRequest_(id, nativeConfig);
        freeSensors(nativeConfig);
    }

    sendMethodCallResult(id, callbackCode);
}


Sensors* CServer::nativeSensors(const emserver::SensorsControl &protoSensors)
{
    Sensors *pSensors = new Sensors();
    auto &protoSens = protoSensors.config();
    pSensors->sensorsCount = protoSensors.config_size();
    for(uint32_t i = 0; i < pSensors->sensorsCount; i++)
    {
        auto &protoSen = protoSens[i];
        if(!protoSens[i].IsInitialized())
            continue;

        int32_t sensId = protoSen.id();
        auto type = protoSen.type();
        if(type == emserver::stTemperature)
        {
            auto *pSens = new SensorTemperature();
            emserver::ConfigurationTemperature conf;
            bool isOk = conf.ParseFromString(protoSen.specificconfiguration());
            if(isOk)
            {
                pSens->watchLevel = conf.level();
                pSens->id = sensId;
                pSensors->hardware[i] = pSens;
            }
            else
                delete pSens;
        }
        else if(type == emserver::stDryContact)
        {
            auto pSens = new SensorDryContact();
            emserver::ConfigurationDryContact conf;
            bool isOk = conf.ParseFromString(protoSen.specificconfiguration());
            if(isOk)
            {
                pSens->watchState = conf.state();
                pSens->id = sensId;
                pSensors->hardware[i] = pSens;
            }
            else
                delete pSens;
        }
    }

    return pSensors;
}

emserver::Capabilities CServer::protoSensors(const Sensors *pSensors)
{
    emserver::Capabilities caps;
    for(uint32_t i = 0; i < pSensors->sensorsCount; i++)
    {
        auto protoSens = caps.add_systemconfiguration();
        auto &nativeSens = pSensors->hardware[i];
        if(nativeSens->type == SensorType::sensorTemperature)
        {
            auto tempSens = static_cast<SensorTemperature*>(nativeSens);
            emserver::ConfigurationTemperature conf;
            conf.set_level(tempSens->watchLevel);

            protoSens->set_id(tempSens->id);
            protoSens->set_type(emserver::stTemperature);
            protoSens->set_specificconfiguration(conf.SerializeAsString());
        }
        else if(nativeSens->type == SensorType::sensorDryContact)
        {
            auto sens = static_cast<SensorDryContact*>(nativeSens);
            emserver::ConfigurationDryContact conf;
            conf.set_state(sens->watchState);

            protoSens->set_id(sens->id);
            protoSens->set_type(emserver::stDryContact);
            protoSens->set_specificconfiguration(conf.SerializeAsString());
        }
    }
    return caps;
}

void CServer::freeSensors(Sensors *sensors)
{
    if(!sensors)
        return;

    if(sensors->sensorsCount == 0)
    {
        delete sensors;
        return;
    }

    for(uint32_t i = 0; i < sensors->sensorsCount; i++)
    {
        delete sensors->hardware[i];
    }

    delete sensors;
}

emserver::SensorActivated CServer::activeSensorFromNative(const SensorBase *pSensor)
{
    emserver::SensorActivated ret;
    if(!pSensor)
        return ret;

    if(pSensor->type == SensorType::sensorTemperature)
    {
        const SensorTemperature* pTempSens = static_cast<const SensorTemperature*>(pSensor);
        ret.set_sensorid(pTempSens->id);
        ret.set_type(emserver::stTemperature);

        emserver::SensorTemperatureState st;
        st.set_value(pTempSens->currentValue);

        ret.set_sensorvalue(st.SerializeAsString());
    }
    else if(pSensor->type == SensorType::sensorDryContact)
    {
        const SensorDryContact* pDCSens = static_cast<const SensorDryContact*>(pSensor);
        ret.set_sensorid(pDCSens->id);
        ret.set_type(emserver::stDryContact);

        emserver::SensorDryContactState st;
        st.set_state(pDCSens->currentState);

        ret.set_sensorvalue(st.SerializeAsString());
    }

    return ret;
}



void CServer::sendMethodCallResult(const uint32_t &callID, const int32_t &code)
{
    appendCounter(EmserverProcessor::ServerRuntimeNotification);
    emserver::ServerRuntimeNotification srn;
    srn.set_methodid(callID);
    srn.set_code(code);
    auto resp = m_processor->makeMessage(EmserverProcessor::ServerRuntimeNotification, &srn);
    if(resp.empty())
        return;

    if(m_session)
    {
        m_session->sendData(resp);
    }
}

void CServer::logging(const std::string &logString)
{
    if(!onLog_)
        return;

    if(logString.empty())
        return;

    CNString str;
    str.size = logString.size() + 1;
    str.pStringData = new char[str.size];
    memset(str.pStringData, 0, str.size);
    memcpy(str.pStringData, logString.c_str(), logString.size());
    onLog_(&str);
}


#endif


