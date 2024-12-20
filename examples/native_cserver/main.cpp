#include <QCoreApplication>

#include <iostream>
#include <cstdint>
#include <string>

#include <cserver.h>

#include <vector>

using std::cout;
using std::endl;
using std::vector;

Sensors systemSensors;

void initialSystem()
{
    systemSensors.sensorsCount = 2;

    SensorDryContact *senDC = new SensorDryContact();
    senDC->currentState = false;
    senDC->watchState = true;
    senDC->id = 1;

    SensorTemperature *temp = new SensorTemperature();
    temp->currentValue = 0;
    temp->watchLevel = 30;
    temp->id = 2;

    systemSensors.hardware = new SensorBase*[2];
    systemSensors.hardware[0] = senDC;
    systemSensors.hardware[1] = temp;
}


void onClientConnected(CNIpAddress *ip, uint16_t port)
{
    if((ip != nullptr) && (ip->pAddress != nullptr))
    {
        string strIp(ip->pAddress, ip->size);
        cout << "client connected ip " << strIp << endl;
        cout << "client connected port " << port << endl;
    }
    else
    {
        cout << "client disconnected" << endl;
    }
}

requestid sendId = 0;
int onClientCapabilitiesRequest(const requestid id)
{
    cout << "[onClientCapabilitiesRequest] requestid " << id;
    Capabilities caps;
    caps.sensors = systemSensors;
    auto ret = send_capabilities(id, caps);
    if(ret < 0)
        cout << "[onClientCapabilitiesRequest] Send capabilities error. Code " << ret << endl;

    SensorTemperature *temp = static_cast<SensorTemperature*>(systemSensors.hardware[1]);
    temp->currentValue = 30;
    send_activesensor(sendId++, temp);

    return ret;
}

int onServerSensorsControlRequest(requestid id, const Sensors* pSensors)
{
    if(!pSensors)
        return -1;

    cout << "[onServerSensorsControlRequest] requestid " << id << "; sensors count " << pSensors->sensorsCount;
    for(uint32_t i = 0; i < pSensors->sensorsCount; i++)
    {
        cout << "Sensor " << i << " :" << endl;
        auto sens = pSensors->hardware[i];
        if(sens->type == SensorType::sensorDryContact)
        {
            SensorDryContact *pSens = static_cast<SensorDryContact*>(sens);
            cout << " - type DryContact;" << endl << " - triggerState: " << pSens->watchState << endl;
        }
        else if(sens->type == SensorType::sensorTemperature)
        {
            SensorTemperature *pSens = static_cast<SensorTemperature*>(sens);
            cout << " - type Temperature;" << endl << " - trigger level " << pSens->watchLevel << endl;
        }
    }

    return 0;
}

void onLogging(CNString *str)
{
    std::string logs(str->pStringData, str->size);

    std::cout << "[onLoging] " << logs << std::endl;
}


int main(int argc, char ** argv)
{
    cout << "Hello world! Try to init native server" << endl;

    QCoreApplication a(argc, argv);

    string ip("127.0.0.1");
    uint16_t port = 35452;
    initialSystem();
    server_create();
    cout << "Updating logging level to debug" << endl;
    server_update_loglevel(ServerLoggingLevel::SrvDebug);

    string logPath{"C:/CodeProjects"};
    CNString cnLogPath;
    cnLogPath.size = logPath.size();
    cnLogPath.pStringData = new char[cnLogPath.size];
    memset(cnLogPath.pStringData, 0, cnLogPath.size);
    memcpy(cnLogPath.pStringData, logPath.c_str(), cnLogPath.size);
    server_update_logpath(&cnLogPath);


    cout << "Registering callbacks" << endl;
    server_register_callback_ClientConnected(&onClientConnected);
    server_register_callback_CapabilitiesRequest(&onClientCapabilitiesRequest);
    server_register_callback_Logging(&onLogging);


    CNIpAddress addr;
    addr.size = ip.size();
    addr.pAddress = new char[ip.size()];
    memcpy(addr.pAddress, ip.c_str(), ip.size());

    int ret = 0;
    int err = server_listen(&addr, port);
    if(err != 0)
    {
        cout << "Can't listen interfave";
        ret = -1;
    }
    else
    {
        cout << "Waiting for client on " << ip << ":" << std::to_string(port) << "..." << endl;
        ret = a.exec();

        system("pause");

        cout << "Closing server..." << endl;
        server_close();

        cout << "Bye..." << endl;
        system("pause");
    }

    return ret;
}
