#include "commonprocessor.h"
#include "baseprotocol.h"
#include "bufferprocessor.h"

#include <smlog.h>

#include <chrono>

using namespace std::chrono_literals;
using std::vector;

CommonProcessor::CommonProcessor() :
    m_needExit(false)
{
    m_threadProcessing = std::thread(&CommonProcessor::bufferProcessingThreadFunc, this);
}

CommonProcessor::~CommonProcessor()
{
    m_needExit = true;
    m_cvNeedProcessing.notify_one();
    m_threadProcessing.join();
}

void CommonProcessor::onReadedChunk(const char *pData, const size_t &dataSize)
{
    // copy data from incoming buffer
    DataChunk dataChunk(dataSize);
    //dataChunk.assign(pData, pData + dataSize);
    memcpy(dataChunk.data(), pData, dataSize);

    std::lock_guard<std::mutex> lg(m_mutexSync);
    m_acceptorBuffer.push_back(dataChunk);
    m_cvNeedProcessing.notify_one();
}

CommonProcessor::DataChunk CommonProcessor::prepareData(const std::string &s)
{
    common_base::CommonMessage msg;
    msg.set_code(1);
    msg.set_payload(s);

    common_base::TransportContainer trMsg;
    trMsg.set_contaiterversion(1);
    trMsg.set_payload(msg.SerializeAsString());

    BaseProtocol::Ptr p = BaseProtocol::create(trMsg.SerializeAsString());
    return p->makeProtocolMessage();
}

void CommonProcessor::bufferProcessingThreadFunc()
{
    LOG_DEBUG << "[CommonProcessor::bufferProcessingThreadFunc] starting base protocol processing thread";
    //std::cv_status stat = std::cv_status::timeout;

    BufferProcessor processor;

    while(!m_needExit)
    {
        std::unique_lock<std::mutex> lg(m_mutexSync);
        if(m_acceptorBuffer.empty() && (m_cvNeedProcessing.wait_for(lg, 200ms) == std::cv_status::timeout))
            continue;

        ChunksBuffer acceptedChunks;
        acceptedChunks.swap(m_acceptorBuffer);
//        LOG_DEBUG << "acceptedChunks has incoming size " << acceptedChunks.size() << " items";
        lg.unlock();

        if(!m_readCallback)
        {
            acceptedChunks.clear();
            continue;
        }

        processor.appendBuffers(std::move(acceptedChunks));
        const auto & messages = processor.messages();
        if(messages.empty())
        {
            //LOG_DEBUG << "No messages extracted";
            continue;
        }

//        LOG_DEBUG << "[CommonProcessor::bufferProcessingThreadFunc] extracted " << messages.size() << " messages";
        common_base::TransportContainer trMsg;
        for(const auto &message : messages)
        {
            bool isExtracted = trMsg.ParseFromString(message->payload());
            if(isExtracted && trMsg.IsInitialized() && trMsg.has_payload())
            {
                common_base::CommonMessage cmsg;
                cmsg.ParseFromString(trMsg.payload());
                if(!m_needExit)
                    m_readCallback(cmsg);
                else
                    break;
            }
        }

//        LOG_DEBUG << "[CommonProcessor::bufferProcessingThreadFunc] Wait next incoming chunks. Not processed data: " << processor.bufferSize() << " bytes";
    }

    LOG_DEBUG << "[CommonProcessor::bufferProcessingThreadFunc] ending base protocol processing thread";
}
