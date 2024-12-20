#pragma once
#include <ispecificprocessor.h>

#include "capabilities.pb.h"
#include "sensors.pb.h"

#include <smlog.h>

#include <boost/any.hpp>

#include <functional>
#include <map>


class EmserverProcessor : public ISpecificProcessor
{
public:
    using callbackCapabilityRequest = std::function<void(const emserver::RequestCapabilities&, const int64_t&)>;
    using callbackCapabilityResponse = std::function<void(const emserver::Capabilities&, const int64_t&)>;
    using callbackServerRuntimeNotification = std::function<void(const emserver::ServerRuntimeNotification, const int64_t&)>;
    using callbackSensorControl = std::function<void(const emserver::SensorsControl, const int64_t&)>;
    using callbackSensorActivated = std::function<void(const emserver::SensorActivated, const int64_t&)>;

    enum MethodCode
    {
        RequestCapabilites = 1,
        ResponseCapabilities,
        ServerRuntimeNotification,
        SensorsConfigurationControl,
        SensorControl,
        SensorActivated
    };

    // TODO : REFACTOR THIS!
    explicit EmserverProcessor(uint32_t workModeCode);
    virtual bool callbackRecievedCommonMessage(const common_base::CommonMessage &commonBaseMessage) override;

    std::string makeMessage(const MethodCode code, const google::protobuf::Message *pMessage, const int64_t id = -1);

    template<typename T> bool registerCallback(const MethodCode code, const T& pCallback)
    {
        m_callbacks[code] = pCallback;
        return true;
    }

private:
    using CallbackContainer = std::map<MethodCode, boost::any>;
    CallbackContainer m_callbacks;

    uint32_t m_workModeCode;

    // M - proto message type
    // C - callback type
    template<typename M, typename C> bool commonCallbackProcessing(MethodCode code, const std::string &responce, const int64_t &mcode)
    {
        bool ret = false;
        M msg;
        bool isOk = msg.ParseFromString(responce);
        if(!isOk)
            return ret;

        auto it = m_callbacks.find(code);
        if(it != m_callbacks.end())
        {
            auto funcPtr = boost::any_cast<C>(it->second);
            funcPtr(msg, mcode);
            ret = true;
        }

        return ret;
    }
};
