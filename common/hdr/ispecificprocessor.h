#pragma once

#include <common.pb.h>

class ISpecificProcessor
{
public:
    enum WorkModeCodes
    {
        emserver = 0x01
    };
    ISpecificProcessor() = default;
    virtual ~ISpecificProcessor() = default;

    virtual bool callbackRecievedCommonMessage(const common_base::CommonMessage& message) = 0;
};
