#pragma once
#include <baseprotocol.h>

#include <vector>
#include <cstdint>
#include <list>

class BufferProcessor
{
public:
    using DataChunk = std::vector<uint8_t>;
    using DataChunks = std::list<DataChunk>;
    using BaseMessages = std::vector<BaseProtocol::Ptr>;

    BufferProcessor() = default;
    ~BufferProcessor();

    void appendBuffer(const DataChunk &data);
    void appendBuffers(const DataChunks &&datas);
    const BaseMessages messages();
    size_t bufferSize() const { return m_processingBuffer.size(); }
    void flushBuffer();

private:
    DataChunk m_processingBuffer;
};
