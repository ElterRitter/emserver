#include "interop_sensors.h"

using namespace emserverproxy;



SensorDryContact::SensorDryContact(const sensorid id, const bool triggeredState) :
    SensorBase(id)
    , m_triggeredState(triggeredState)
{

}

void SensorDryContact::updateState(ISensorState::Ptr &state)
{
    if(!state)
        return;

    auto ptr = std::static_pointer_cast<State>(state);
    if(!ptr)
        return;

    m_currentState = ptr;
}

bool SensorDryContact::triggeredState() const
{
    return m_triggeredState;
}

SensorTemperature::SensorTemperature(const sensorid id, int32_t tresholdLevel) :
    SensorBase(id)
    , m_tresholdLevel(tresholdLevel)
{

}

void SensorTemperature::updateState(ISensorState::Ptr &state)
{
    if(!state)
        return;

    auto ptr = std::static_pointer_cast<State>(state);
    if(!ptr)
        return;

    m_currentState = ptr;
}

int32_t SensorTemperature::tresholdLevel() const
{
    return m_tresholdLevel;
}
