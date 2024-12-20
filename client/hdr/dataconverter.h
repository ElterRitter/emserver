#pragma once

#include "interop.h"
#include "capabilities.pb.h"
#include "sensors.pb.h"

class DataConverter
{
public:
    DataConverter() = default;

    static emserverproxy::Sensors fromCaps(const emserver::Capabilities &caps);
    static emserverproxy::Sensors sensorsFromProto(const emserver::SensorsControl &protoSensors);
    static emserver::SensorsControl sensorsToProto(const emserverproxy::Sensors &nativeSensors);
    static emserverproxy::ISensorState::Ptr nativeActivatedSensor(const emserver::SensorActivated &protoSensor);
private:
    static emserverproxy::SensorBase::Ptr nativeSensor(const emserver::SensorConfiguration &protoSensor);
};

