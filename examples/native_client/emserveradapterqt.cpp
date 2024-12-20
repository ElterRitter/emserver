#include "emserveradapterqt.h"

#include <emserverfacade.h>

#include <QDebug>

#include <QDateTime>

Q_LOGGING_CATEGORY(emserveradapterqt, "emserveradapterqt")
#define Debug qDebug(emserveradapterqt)
#define Warn qWarning(emserveradapterqt)
#define Info qInfo(emserveradapterqt)

using emserverproxy::EmserverFacade;

EmserverAdapterQt::EmserverAdapterQt(QObject *parent)
    : QObject{parent}
     , m_spectrCounter(0)
{
    qRegisterMetaType<emserverproxy::ConnectedState>();
    qRegisterMetaType<emserverproxy::Sensors>();

    m_server = std::make_shared<ProxyFacade>();

    EmserverFacade::emserverConnectedStateChanging conHandler = std::bind(&EmserverAdapterQt::onConnectionStateChanged, this, std::placeholders::_1, std::placeholders::_2);
    m_server->registerCallbackConnectedStateChanged(conHandler);

    EmserverFacade::emserverResponseCapabilities handler = std::bind(&EmserverAdapterQt::onCapabilitiesResponse, this,
                                                                   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_server->registerCallbackCapabilities(handler);

    EmserverFacade::emserverResponseSensorActivation cbSensAct = std::bind(&EmserverAdapterQt::onActivatedSensor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    m_server->registerCallbackSensorActivated(cbSensAct);

    auto handlerError = std::bind(&EmserverAdapterQt::onServerRuntimeNotification, this, std::placeholders::_1, std::placeholders::_2);
    m_server->registerCallbackServerRuntimeNotifications(handlerError);
}

void EmserverAdapterQt::connectionOpen(const QString addr, const quint16 port)
{
    if(!m_server)
        return;

    m_server->connectionOpen(addr.toStdString(), port);
}

void EmserverAdapterQt::connectionClose()
{
    if(!m_server)
        return;

    m_server->connectionClose();
}

void EmserverAdapterQt::sendRequestCapabilities()
{
    if(!m_server)
        return;

    int ret = m_server->sendRequestCapabilities();
    if(ret >= 0)
        m_calls[ret] = "EmserverAdapterQt::sendRequestCapabilities";
}


void EmserverAdapterQt::onConnectionStateChanged(const ProxyFacade *ptr, const emserverproxy::ConnectedState connState)
{
    Q_UNUSED(ptr)
    emit clientConnectedStateChanged(connState);
}

void EmserverAdapterQt::onCapabilitiesResponse(const ProxyFacade *ptr, const emserverproxy::Sensors sensors, const emserverproxy::requestid &id)
{
    Q_UNUSED(ptr);
    Q_UNUSED(id);
    Debug << "[EmserverAdapterQt::onCapabilitiesResponse] caps responce:" << endl;
    uint32_t sitem = 0;
    m_sensors = sensors;
    for(const auto &sen : m_sensors)
    {
        if(!sen)
            continue;

        Debug << "Sen[" << sitem++ << "]: id " << sen->sensorId();
        Debug << "type: " << (sen->type() == emserverproxy::SensorBase::TypeDryContact ? "Dry Contacts" : "Temperature" );
    }

    emit clientSensorsUpdate(m_sensors);
}

void EmserverAdapterQt::onActivatedSensor(const ProxyFacade *ptr, emserverproxy::ISensorState::Ptr sensorState, const emserverproxy::requestid &id)
{
    Q_UNUSED(ptr);
    Q_UNUSED(id);
    auto it = std::find_if(m_sensors.begin(), m_sensors.end(),
                           [&sensorState](const emserverproxy::SensorBase::Ptr &sb)
                           {
                               return sb->sensorId() == sensorState->sensorId();
                           });
    if(it == m_sensors.end())
        return;

    (*it)->updateState(sensorState);
    auto state = (*it)->state();
    if((*it)->type() == emserverproxy::SensorBase::TypeDryContact)
        Debug << "[EmserverAdapterQt::onActivatedSensor] Dry contact sensor activated for level " << std::static_pointer_cast<emserverproxy::SensorDryContact::State>(state)->state();
    else
        Debug << "[EmserverAdapterQt::onActivatedSensor] Temperature sensor activated for level " << std::static_pointer_cast<emserverproxy::SensorTemperature::State>(state)->value();
}

void EmserverAdapterQt::onServerRuntimeNotification(const ProxyFacade *ptr, const emserverproxy::ErrorDescriptor error)
{
    Q_UNUSED(ptr)

    if((error.methodCode != 0) && (error.errorCode != 0))
    {
        std::string mname{"undefined"};
        auto it = m_calls.find(error.methodCode);
        if(it != m_calls.end())
        {
            mname = it->second;
            m_calls.erase(it);
        }
        Debug << "Detected a method validation error: number of name is " << error.methodCode << " . Error code " << error.errorCode;
    }
    else if((error.methodCode == 0) && (error.errorCode != 0))
    {
        Debug << "Detected a server runtime error: code " << error.methodCode << " with description " << error.description.c_str();
    }
    else if((error.methodCode != 0) && (error.errorCode == 0))
    {
        Debug << "Detected normal notification. Error code is 0";
        auto it = m_calls.find(error.methodCode);
        if(it != m_calls.end())
        {
            m_calls.erase(it);
        }
    }
}



