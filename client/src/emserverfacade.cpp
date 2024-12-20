#include "emserverfacade.h"

#include "emserveradapter.h"

using namespace emserverproxy;

EmserverFacade::EmserverFacade()
{
    m_pAdapter = std::shared_ptr<EmserverAdapter>( new EmserverAdapter(this) );
}

void EmserverFacade::connectionOpen(const std::string &remoteIP, const uint16_t port)
{
    m_pAdapter->connectionOpen(remoteIP, port);
}

void EmserverFacade::connectionClose()
{
    m_pAdapter->connectionClose();
}

requestid EmserverFacade::sendRequestCapabilities()
{
    return m_pAdapter->sendRequestCapabilities();
}

requestid EmserverFacade::sendSensorsControl(const Sensors &sensorsParameters)
{
    return m_pAdapter->sendSensorsControl(sensorsParameters);
}


void EmserverFacade::registerCallbackConnectedStateChanged(const emserverConnectedStateChanging &ptrConnected)
{
    m_pAdapter->registerCallback<emserverConnectedStateChanging>(EmserverAdapter::ConnectedStateChanged, ptrConnected);
}

void EmserverFacade::registerCallbackCapabilities(const emserverResponseCapabilities &ptrCaps)
{
    m_pAdapter->registerCallback<emserverResponseCapabilities>(EmserverAdapter::ResponseCapabilities, ptrCaps);
}

void EmserverFacade::registerCallbackSensorActivated(const emserverResponseSensorActivation &ptrSensActive)
{
    m_pAdapter->registerCallback<emserverResponseSensorActivation>(EmserverAdapter::SensorActivated, ptrSensActive);
}

void EmserverFacade::registerCallbackServerRuntimeNotifications(const emserverServerRuntimeNotification &ptrErrorHandler)
{
    m_pAdapter->registerCallback<emserverServerRuntimeNotification>(EmserverAdapter::ResponseServerError, ptrErrorHandler);
}
