#ifndef API_EMSERVER_CAPABILITY
#define API_EMSERVER_CAPABILITY

#include "api_common.h"
#include "api_sensors.h"

#pragma pack(push, 1)

struct Capabilities
{
    Sensors sensors;
};

#pragma pack(pop)

#endif
