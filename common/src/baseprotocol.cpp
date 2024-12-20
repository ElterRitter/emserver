#include "baseprotocol.h"

#include <boost/crc.hpp>

#include <smlog.h>

constexpr char magic = '$';

static_assert(sizeof(magic) == 1, "Invalid magic size");

BaseProtocol::Ptr BaseProtocol::extractFromRawData(const uint8_t *pData, const size_t avaliableDataSize, ExtractionFailure &reason)
{
    if(!pData)
    {
        reason = InvalidFirstByte;
        return nullptr;
    }

    if(pData[0] != magic)
    {
        reason = InvalidFirstByte;
        return nullptr;
    }

    uint32_t msize = 0;
    uint32_t totalMessageSize = sizeof(msize) + 1;
    if(avaliableDataSize <= totalMessageSize)
    {
        reason = NeedMoreData;
        return nullptr;
    }

    memcpy(&msize, &pData[1], sizeof(msize));

    uint32_t chSum = 0;
    totalMessageSize += msize + sizeof(chSum);
    if(avaliableDataSize < totalMessageSize)
    {
        reason = NeedMoreData;
        return nullptr;
    }

    memcpy(&chSum, &pData[totalMessageSize - sizeof(chSum)], sizeof(chSum));

    uint32_t calcSum = calculateChecksum(pData, totalMessageSize - sizeof(chSum));
    if(calcSum == chSum)
    {
        reason = None;
        int32_t payloadSz = totalMessageSize - sizeof(chSum) * 2 - sizeof(magic);
        const uint8_t *payloadData = pData + sizeof(magic) + sizeof(msize);
        return BaseProtocol::Ptr( new BaseProtocol(payloadData, payloadSz) );
    }
    else
        reason = InvalidChecksum;

    return nullptr;
}

BaseProtocol::Ptr BaseProtocol::create(const std::string &payload)
{
    return std::unique_ptr<BaseProtocol>(new BaseProtocol(payload));
}

BaseProtocol::Bytes BaseProtocol::makeProtocolMessage() const
{
    uint32_t payload_size = static_cast<uint32_t>(m_payload.size());
    uint32_t message_size = m_payload.size() + 2 * sizeof(uint32_t) + sizeof(magic);
    
    Bytes ret;
    ret.reserve(message_size);
    ret.resize(sizeof(magic) + sizeof(message_size));
    
    ret[0] = magic;
    memcpy(&ret[1], &payload_size, sizeof(payload_size));
    std::copy(m_payload.begin(), m_payload.end(), std::back_inserter(ret));

    uint32_t chPos = 1 + sizeof(uint32_t) + payload_size;
    uint32_t chSum = calculateChecksum(ret.data(), ret.size());
    ret.resize(ret.size() + sizeof(chSum));
	memcpy(&ret[chPos], &chSum, sizeof(chSum));

    return ret;
}

std::string BaseProtocol::payload() const
{
    std::string ret;
    ret.reserve(m_payload.size());
    std::copy(m_payload.begin(), m_payload.end(), std::back_inserter(ret));
    return ret;
}

uint32_t BaseProtocol::fullSize() const
{
    return m_payload.size() + sizeof(magic) + sizeof(uint32_t)*2;
}

BaseProtocol::BaseProtocol(const std::string &payload)
{
    std::copy(payload.begin(), payload.end(), std::back_inserter(m_payload));
}

BaseProtocol::BaseProtocol(const uint8_t *pData, std::size_t dataSize)
{
    m_payload.resize(dataSize);

    memcpy(m_payload.data(), pData, dataSize);
}


uint32_t BaseProtocol::calculateChecksum(const uint8_t *pData, const std::size_t &dataSize)
{
    boost::crc_32_type ret;
    ret.process_bytes(pData, dataSize);
    return ret.checksum();
}
