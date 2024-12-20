#include "bufferprocessor.h"

#include <smlog.h>

const uint8_t magic = '$';

BufferProcessor::~BufferProcessor()
{
    m_processingBuffer.clear();
}

void BufferProcessor::appendBuffer(const DataChunk &data)
{
    m_processingBuffer.insert(m_processingBuffer.end(), data.begin(), data.end());
}

void BufferProcessor::appendBuffers(const DataChunks &&datas)
{
    for(const auto &data : datas)
    {
        appendBuffer(data);
    }
    //LOG_DEBUG << "After appending buffer size " << m_processingBuffer.size() << " bytes";
}

const BufferProcessor::BaseMessages BufferProcessor::messages()
{
    BaseMessages ret;
    auto searcherMagic = [](const uint8_t &item) { return item == magic; };
    auto markerPos = std::find_if(m_processingBuffer.begin(), m_processingBuffer.end(), searcherMagic);
    bool isChecksumErrorDetected = false;
    BaseProtocol::Ptr item;
    BaseProtocol::ExtractionFailure reason;
    while(markerPos != m_processingBuffer.end())
    {
        auto dst = std::distance(m_processingBuffer.begin(), markerPos);
        item = BaseProtocol::extractFromRawData(&(*markerPos), m_processingBuffer.size() - dst, reason);
        if((reason == BaseProtocol::ExtractionFailure::InvalidChecksum) && (!isChecksumErrorDetected))
        {
            isChecksumErrorDetected = true;
            LOG_WARN << "--------------------Detected wrong checksum parser error-------------------------- ( " << m_processingBuffer.size() << "; " << dst << " )";
        }

        if(item)
        {
            m_processingBuffer.erase(m_processingBuffer.begin(), markerPos + item->fullSize());
            ret.push_back(std::move(item));
            markerPos = m_processingBuffer.begin();
        }
        else
        {
            ++markerPos;
        }

        markerPos = std::find_if(markerPos, m_processingBuffer.end(), searcherMagic);
    }

    return ret;
}

void BufferProcessor::flushBuffer()
{
    m_processingBuffer.clear();
}
