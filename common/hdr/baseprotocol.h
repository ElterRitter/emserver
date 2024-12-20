#pragma once

#include <common.pb.h>

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

class BaseProtocol
{
public:
    using Ptr = std::unique_ptr<BaseProtocol>;
    using Bytes = std::vector<uint8_t>;

    enum ExtractionFailure { None, NeedMoreData, InvalidFirstByte, InvalidChecksum };

    static Ptr extractFromRawData(const uint8_t *pData, const std::size_t avaliableDataSize, ExtractionFailure &reason);
    static Ptr create(const std::string &payload);

    Bytes makeProtocolMessage() const;
    std::string payload() const;
    uint32_t fullSize() const;
    uint32_t payloadSize() const { return m_payload.size(); }

private:
    Bytes m_payload;

    BaseProtocol(const std::string &payload);
    BaseProtocol(const uint8_t *pData, size_t dataSize);

    static uint32_t calculateChecksum(const uint8_t *pData, const std::size_t &dataSize);
};

