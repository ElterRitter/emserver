#pragma once
#include "interop.h"

#include <QObject>
#include <QLoggingCategory>

#include <memory>
#include <map>
#include <string>

Q_DECLARE_LOGGING_CATEGORY(emserveradapterqt)

namespace emserverproxy {
class EmserverFacade;
}

class EmserverAdapterQt : public QObject
{
    Q_OBJECT
public:
    explicit EmserverAdapterQt(QObject *parent = nullptr);

    void connectionOpen(const QString addr, const quint16 port);
    void connectionClose();

    void sendRequestCapabilities();

signals:
    void clientConnectedStateChanged(emserverproxy::ConnectedState state);
    void clientSensorsUpdate(emserverproxy::Sensors sensors);

private:
    using callmap = std::map<int, std::string>;
    callmap m_calls;
    std::shared_ptr<emserverproxy::EmserverFacade> m_server;
    emserverproxy::Sensors m_sensors;

    quint32 m_spectrCounter;

    using ProxyFacade = emserverproxy::EmserverFacade;

    // callbacks
    void onConnectionStateChanged(const ProxyFacade* ptr, const emserverproxy::ConnectedState connState);
    void onCapabilitiesResponse(const ProxyFacade* ptr, const emserverproxy::Sensors sensors, const emserverproxy::requestid& id);
    void onActivatedSensor(const ProxyFacade* ptr, emserverproxy::ISensorState::Ptr sensorState, const emserverproxy::requestid& id);
    void onServerRuntimeNotification(const ProxyFacade* ptr, const emserverproxy::ErrorDescriptor error);
};


Q_DECLARE_METATYPE(emserverproxy::NetworkEndpoint)
Q_DECLARE_METATYPE(emserverproxy::ConnectedState)
Q_DECLARE_METATYPE(emserverproxy::Sensors)
