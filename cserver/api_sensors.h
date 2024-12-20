#ifndef API_SENSORS_H
#define API_SENSORS_H

#include "api_common.h"
#pragma pack(push,1)
enum SensorType
{
    sensorUnknown = 0,
    sensorDryContact = 1,
    sensorTemperature = 2
};

struct SensorBase
{
    int32_t id;
    SensorType type;
    SensorBase() : id(0), type(sensorUnknown) { };
    SensorBase(SensorType t) : id(0), type{t} { };
};

struct SensorDryContact : SensorBase
{
    GeoPosition *position;
    bool watchState;
    bool currentState;
    SensorDryContact() : SensorBase(SensorType::sensorDryContact), position{nullptr}, watchState{false}, currentState{false}
    { };
};

struct SensorTemperature : SensorBase
{
    GeoPosition *position;
    int32_t watchLevel;
    int32_t currentValue;
    SensorTemperature() : SensorBase(SensorType::sensorTemperature), position{nullptr}, watchLevel(0), currentValue(0)
    { }
};

struct Sensors
{
    uint32_t sensorsCount;
    SensorBase** hardware;
    Sensors() : sensorsCount(0), hardware(nullptr)
    { };
};

#pragma pack(pop)

#endif // API_SENSORS_H
