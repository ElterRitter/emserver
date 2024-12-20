#include "test_commonprocessor.h"
#include "baseprotocol.h"

#include <string>

using std::string;

Test_CommonProcessor::Test_CommonProcessor()
{

}

void Test_CommonProcessor::SetUp()
{
    m_proc = std::make_unique<CommonProcessor>();
}

void Test_CommonProcessor::TearDown()
{
    m_proc.reset();
}

TEST_F(Test_CommonProcessor, ReadSingleMessage)
{
    string payload("testingOne");
    auto proto = BaseProtocol::create(payload);
    auto data = proto->makeProtocolMessage();

    m_proc->onReadedChunk((char*)data.data(), data.size());

}
