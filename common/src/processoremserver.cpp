#include "processoremserver.h"

#include "emserver.pb.h"

#include <string>

EmserverProcessor::EmserverProcessor(uint32_t workModeCode) :
    m_workModeCode{workModeCode}
{

}

std::string EmserverProcessor::makeMessage(const MethodCode code, const google::protobuf::Message *pMessage, const int64_t id)
{
    if(pMessage == nullptr)
        return std::string();

    if(!pMessage->IsInitialized())
        return std::string();

    emserver::ServerCommonMessage msg;
    msg.set_type(code);
    msg.set_id(id);
    msg.set_payload(pMessage->SerializeAsString());
    return msg.SerializeAsString();
}

bool EmserverProcessor::callbackRecievedCommonMessage(const common_base::CommonMessage &commonBaseMessage)
{
    bool ret = false;
    if(!commonBaseMessage.has_code())
        return ret;

    if(commonBaseMessage.code() != m_workModeCode)
        return ret;

    emserver::ServerCommonMessage emServerMessage;
    if(!emServerMessage.ParseFromString(commonBaseMessage.payload()))
        return ret;

    const int64_t code = emServerMessage.has_id() ? emServerMessage.id() : -1;
    const std::string &payload = emServerMessage.payload();
    switch(emServerMessage.type())
    {
    case RequestCapabilites:
        ret = commonCallbackProcessing<emserver::RequestCapabilities, callbackCapabilityRequest>(RequestCapabilites, payload, code);
        break;
    case ResponseCapabilities:
        ret = commonCallbackProcessing<emserver::Capabilities, callbackCapabilityResponse>(ResponseCapabilities, payload, code);
        break;
    case ServerRuntimeNotification:
        ret = commonCallbackProcessing<emserver::ServerRuntimeNotification, callbackServerRuntimeNotification>(ServerRuntimeNotification, payload, code);
        break;
    case SensorControl:
        ret = commonCallbackProcessing<emserver::SensorsControl, callbackSensorControl>(SensorControl, payload, code);
        break;
    case SensorActivated:
        ret = commonCallbackProcessing<emserver::SensorActivated, callbackSensorActivated>(SensorActivated, payload, code);
        break;
    }

    // TODO : add ret check

    return ret;
}
