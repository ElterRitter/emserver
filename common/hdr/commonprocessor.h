#pragma once
#include <common.pb.h>

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <list>
#include <functional>

class CommonProcessor
{
public:
    using DataChunk = std::vector<uint8_t>;
    using ChunksBuffer = std::list<DataChunk>;
    using CommonMessageRecievedCallback = std::function<void(const common_base::CommonMessage&)>;

    CommonProcessor();
    ~CommonProcessor();

    void registerCommonMessageIncomingCallback(CommonMessageRecievedCallback cb) { m_readCallback = cb; }

    void onReadedChunk(const char *pData, const size_t &dataSize);
    DataChunk prepareData(const std::string &s);

private:
    CommonMessageRecievedCallback m_readCallback;
    ChunksBuffer m_acceptorBuffer;
    std::thread m_threadProcessing;
    std::mutex m_mutexSync;
    std::condition_variable m_cvNeedProcessing;
    bool m_needExit;

    void bufferProcessingThreadFunc();
};

