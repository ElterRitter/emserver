#include "dataconverter.h"

using namespace emserverproxy;




Sensors DataConverter::fromCaps(const emserver::Capabilities &caps)
{
    Sensors ret;
    auto &conf = caps.systemconfiguration();
    for(const auto &protoSensor : conf)
    {
        auto ns = nativeSensor(protoSensor);
        if(ns)
            ret.push_back(ns);
    }

    return ret;
}

emserverproxy::Sensors DataConverter::sensorsFromProto(const emserver::SensorsControl &protoSensors)
{
    Sensors ret;
    auto &config = protoSensors.config();
    for(const auto &protoSensor : config)
    {
        auto ns = nativeSensor(protoSensor);
        if(ns)
            ret.push_back(ns);
    }

    return ret;
}

emserver::SensorsControl DataConverter::sensorsToProto(const emserverproxy::Sensors &nativeSensors)
{
    emserver::SensorsControl ret;

    for(const auto &sensor : nativeSensors)
    {
        emserver::SensorConfiguration conf;
        if(sensor->type() == SensorBase::TypeDryContact)
        {
            auto dcSensor = std::static_pointer_cast<SensorDryContact>(sensor);
            emserver::ConfigurationDryContact dcConf;
            dcConf.set_state(dcSensor->triggeredState());
            conf.set_type(emserver::stDryContact);
            conf.set_id(sensor->sensorId());
            conf.set_specificconfiguration(dcConf.SerializeAsString());
        }
        else if(sensor->type() == SensorBase::TypeTemperature)
        {
            auto tempSens = std::static_pointer_cast<SensorTemperature>(sensor);
            emserver::ConfigurationTemperature teConf;
            teConf.set_level(tempSens->tresholdLevel());
            conf.set_type(emserver::stTemperature);
            conf.set_id(sensor->sensorId());
            conf.set_specificconfiguration(teConf.SerializeAsString());
        }
    }

    return ret;
}

ISensorState::Ptr DataConverter::nativeActivatedSensor(const emserver::SensorActivated &protoSensor)
{
    ISensorState::Ptr ret;
    auto st = protoSensor.type();
    if(st == emserver::SensorsType::stDryContact)
    {
        emserver::SensorDryContactState dcState;
        bool isOk = dcState.ParseFromString(protoSensor.sensorvalue());
        if(!isOk)
            return ret;

        ret = std::make_shared<SensorDryContact::State>(protoSensor.sensorid(), dcState.state());
    }
    else if(st == emserver::SensorsType::stTemperature)
    {
        emserver::SensorTemperatureState tempState;
        bool isOk = tempState.ParseFromString(protoSensor.sensorvalue());
        if(!isOk)
            return ret;

        ret = std::make_shared<SensorTemperature::State>(protoSensor.sensorid(), tempState.value());
    }

    return ret;
}

SensorBase::Ptr DataConverter::nativeSensor(const emserver::SensorConfiguration &protoSensor)
{
    SensorBase::Ptr ret;
    auto senType = protoSensor.type();
    if(senType == emserver::stDryContact)
    {
        emserver::ConfigurationDryContact sensConf;
        sensConf.ParseFromString(protoSensor.specificconfiguration());
        ret = std::make_shared<SensorDryContact>(protoSensor.id(), sensConf.state());
    }
    else if(senType == emserver::stTemperature)
    {
        emserver::ConfigurationTemperature sensTemp;
        sensTemp.ParseFromString(protoSensor.specificconfiguration());
        ret = std::make_shared<SensorTemperature>(protoSensor.id(), sensTemp.level());
    }

    return ret;
}
