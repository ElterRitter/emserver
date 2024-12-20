#include "test_bufferprocessor.h"
#include "capabilities.pb.h"

#include <common.pb.h>
#include <emserver.pb.h>
#include <sensors.pb.h>

#include <bufferprocessor.h>

#include <string>

using std::string;

TEST_F(Test_BufferProcessor, SimpleMessages)
{
    string in{"hello"};
    BaseProtocol::Ptr p = BaseProtocol::create(in);
    auto bytes = p->makeProtocolMessage();

    ASSERT_TRUE(bytes.size() > 0);

    BufferProcessor proc;
    proc.appendBuffer(bytes);
    auto msgs = proc.messages();
    ASSERT_TRUE(msgs.size() > 0);

    string out = msgs.front()->payload();
    ASSERT_EQ(in, out);
}

TEST_F(Test_BufferProcessor, SimpleBucksMessage)
{
    string inOne{"he$$o"};
    auto bytesOne = makeProtocolMessage(inOne);
    ASSERT_TRUE(bytesOne.size() > 0);

    string inTwo{"$ello"};
    auto bytesTwo = makeProtocolMessage(inTwo);
    ASSERT_TRUE(bytesTwo.size() > 0);

    BufferProcessor proc;
    proc.appendBuffer(bytesOne);
    proc.appendBuffer(bytesTwo);
    ASSERT_TRUE(proc.bufferSize() == bytesOne.size() + bytesTwo.size() );

    auto msgs = proc.messages();
    ASSERT_TRUE(msgs.size() == 2);

    ASSERT_EQ(msgs.at(0)->payload(), inOne);
    ASSERT_EQ(msgs.at(1)->payload(), inTwo);
}

TEST_F(Test_BufferProcessor, SimpleMessageWithPrefix)
{
    string in{"testing_string_hello"};
    auto bytes = makeProtocolMessage(in);
    ASSERT_TRUE(bytes.size() > 0) << " valid meessage is empty after gneration";
    BaseProtocol::Bytes wrongChunk{'$', 't', 'e', 's', 't', '$' };

    BufferProcessor proc;
    proc.appendBuffer(wrongChunk);
    proc.appendBuffer(bytes);
    ASSERT_TRUE(proc.bufferSize() > bytes.size()) << "proc.bufferSize is equal or lower than original message size";
    ASSERT_TRUE(proc.bufferSize() == bytes.size() + wrongChunk.size()) << "proc.bufferSize is now equal as chunks size";

    auto msgs = proc.messages();
    ASSERT_TRUE(msgs.size() == 1) << "extracted unexpected messages from buffer. Expected value is 1";

    ASSERT_EQ(msgs.at(0)->payload(), in) << "extracted payload is diffrent than original";
    ASSERT_TRUE(proc.bufferSize() == 0) << "buffer not empty after message extraction";
}

TEST_F(Test_BufferProcessor, SimpleMessageWithPostfix)
{
    string in{"testing_string_hello"};
    auto bytes = makeProtocolMessage(in);
    ASSERT_TRUE(bytes.size() > 0) << " valid meessage is empty after generation";
    BaseProtocol::Bytes wrongChunk{'$', 't', 'e', 's', 't', '$' };

    BufferProcessor proc;
    proc.appendBuffer(bytes);
    proc.appendBuffer(wrongChunk);
    ASSERT_TRUE(proc.bufferSize() > bytes.size()) << "proc.bufferSize is equal or lower than original message size";
    ASSERT_TRUE(proc.bufferSize() == bytes.size() + wrongChunk.size()) << "proc.bufferSize is now equal as chunks size";

    auto msgs = proc.messages();
    ASSERT_TRUE(msgs.size() == 1) << "extracted unexpected messages from buffer. Expected value is 1";

    ASSERT_EQ(msgs.at(0)->payload(), in) << "extracted payload is diffrent than original: in " << in << " out " << msgs.at(0)->payload();
    ASSERT_TRUE(proc.bufferSize() == wrongChunk.size()) << "buffer size not equal as a wrongChunk.size after single message extraction";
}

TEST_F(Test_BufferProcessor, HalfMessages)
{
    string inOne{"some_first_string_hello"};
    auto bytesOne = makeProtocolMessage(inOne);
    ASSERT_TRUE(bytesOne.size() > 0) << "valid message is empty after generation";

    string inTwo{"second_string_larger_than_first_one"};
    auto bytesTwo = makeProtocolMessage(inTwo);
    ASSERT_TRUE(bytesTwo.size() > 0) << "valid second message is empty after generation";

    {
        BufferProcessor proc;
        proc.appendBuffer({bytesOne.begin(), bytesOne.begin() + bytesOne.size() / 2});
        proc.appendBuffer({bytesTwo.rbegin() + bytesTwo.size() / 2, bytesTwo.rend()});
        auto msgs = proc.messages();
        ASSERT_TRUE(msgs.empty());
        ASSERT_TRUE(proc.bufferSize() == bytesOne.size() / 2 + bytesTwo.size() / 2);
    }

    BufferProcessor proc;
    auto itOneHalf = bytesOne.begin() + bytesOne.size() / 2;
    proc.appendBuffer({bytesOne.begin(), itOneHalf});

    ASSERT_TRUE(proc.bufferSize() == bytesOne.size() / 2);

    auto msgs = proc.messages();
    ASSERT_TRUE(msgs.empty());

    proc.appendBuffer({itOneHalf, bytesOne.end()});
    ASSERT_TRUE(proc.bufferSize() == bytesOne.size());

    auto itTwoHalf = bytesTwo.begin() + bytesTwo.size() / 2;
    proc.appendBuffer({bytesTwo.begin(), itTwoHalf});
    ASSERT_EQ(proc.bufferSize(), bytesOne.size() + bytesTwo.size() / 2);

    auto msgs2 = proc.messages();
    ASSERT_TRUE(proc.bufferSize() == bytesTwo.size() / 2);
    ASSERT_TRUE(msgs2.size() == 1);
    ASSERT_EQ(msgs2.at(0)->payload(), inOne);

    auto msgs3 = proc.messages();
    ASSERT_TRUE(msgs3.empty());
    ASSERT_TRUE(proc.bufferSize() == bytesTwo.size() / 2);

    proc.appendBuffer({itTwoHalf, bytesTwo.end()});
    ASSERT_TRUE(proc.bufferSize() == bytesTwo.size());

    auto msgs4 = proc.messages();
    ASSERT_TRUE(msgs4.size() == 1);
    ASSERT_EQ(msgs4.at(0)->payload(), inTwo);
}

TEST_F(Test_BufferProcessor, TwoSplitMessages)
{
    string inOne{"some_first_string_hello"};
    auto bytesOne = makeProtocolMessage(inOne);
    ASSERT_TRUE(bytesOne.size() > 0) << "valid message is empty after generation";

    string inTwo{"second_string_larger_than_first_one"};
    auto bytesTwo = makeProtocolMessage(inTwo);
    ASSERT_TRUE(bytesTwo.size() > 0) << "valid second message is empty after generation";

    BufferProcessor proc;
    proc.appendBuffer(bytesOne);
    BaseProtocol::Bytes splitter{'$', 'd', 'a', '$', 'a'};
    proc.appendBuffer(splitter);
    proc.appendBuffer(bytesTwo);

    auto msgs = proc.messages();
    ASSERT_EQ(msgs.size(), 2);
    ASSERT_TRUE(proc.bufferSize() == 0);
    ASSERT_EQ(msgs.at(0)->payload(), inOne);
    ASSERT_EQ(msgs.at(1)->payload(), inTwo);
    ASSERT_TRUE(proc.bufferSize() == 0);
}

TEST_F(Test_BufferProcessor, TwoMessages)
{
    string inOne{"test1"};
    auto bytesOne = makeProtocolMessage(inOne);
    ASSERT_TRUE(bytesOne.size() > 0);

    string inTwo{"test2"};
    auto bytesTwo = makeProtocolMessage(inTwo);
    ASSERT_TRUE(bytesTwo.size() > 0);

    {
        BufferProcessor::DataChunks chunks;
        chunks.push_back(bytesOne);
        chunks.push_back(bytesTwo);

        BufferProcessor proc;
        proc.appendBuffers(std::move(chunks));
        const auto & msgs = proc.messages();
        ASSERT_TRUE(msgs.size() == 2);

        auto &msg = msgs.at(0);
        EXPECT_EQ(msg->payload(), inOne);
        EXPECT_EQ(msgs.at(1)->payload(), inTwo);
    }

    {
        BufferProcessor::DataChunks chunks;
        chunks.push_back(bytesOne);
        chunks.push_back({bytesTwo.begin(), bytesTwo.begin() + bytesTwo.size() / 2});
        BufferProcessor proc;
        proc.appendBuffers(std::move(chunks));
        const auto & msgs = proc.messages();
        ASSERT_TRUE(msgs.size() == 1);
        ASSERT_TRUE(proc.bufferSize() == bytesTwo.size() / 2);

        proc.appendBuffer({bytesTwo.begin() + bytesTwo.size() / 2, bytesTwo.end()});
        const auto &msgs2 = proc.messages();
        ASSERT_TRUE(msgs2.size() == 1);
        ASSERT_TRUE(proc.bufferSize() == 0);
        EXPECT_EQ(msgs2.at(0)->payload(), inTwo);
    }
}

TEST_F(Test_BufferProcessor, SimpleBinaryMessage)
{
    emserver::ConfigurationTemperature ss;
    ss.set_level(30);

    emserver::ServerCommonMessage cmsg;
    cmsg.set_type(5);   // dummy
    cmsg.set_id(1);
    cmsg.set_payload(ss.SerializeAsString());

    common_base::CommonMessage msg;
    msg.set_code(1);
    msg.set_payload(cmsg.SerializeAsString());

    common_base::TransportContainer trMsg;
    trMsg.set_contaiterversion(1);
    trMsg.set_payload(msg.SerializeAsString());

    auto p = BaseProtocol::create(trMsg.SerializeAsString());
    auto bytes = p->makeProtocolMessage();

    ASSERT_TRUE(bytes.size() > 0);

    BufferProcessor proc;
    proc.appendBuffer(bytes);
    auto msgs = proc.messages();
    ASSERT_TRUE(msgs.size() == 1);
    ASSERT_TRUE(proc.bufferSize() == 0);

    bool isExtracted = trMsg.ParseFromString(msgs.at(0)->payload());
    ASSERT_TRUE(isExtracted);
    ASSERT_TRUE(trMsg.IsInitialized());
    ASSERT_TRUE(trMsg.has_payload());

    isExtracted = msg.ParseFromString(trMsg.payload());
    ASSERT_TRUE(isExtracted);
    ASSERT_TRUE(msg.IsInitialized());
    ASSERT_TRUE(msg.has_code() && msg.code() == 0x01);

    isExtracted = cmsg.ParseFromString(msg.payload());
    ASSERT_TRUE(isExtracted);
    ASSERT_TRUE(cmsg.IsInitialized() && cmsg.has_payload());

    isExtracted = ss.ParseFromString(cmsg.payload());
    ASSERT_TRUE(isExtracted);
    ASSERT_TRUE(ss.has_level() && (ss.level() != 0));

    const auto level = ss.level();
    ASSERT_TRUE(level == 30);
}


BaseProtocol::Bytes Test_BufferProcessor::makeProtocolMessage(const std::string &payload)
{
    auto bp = BaseProtocol::create(payload);
    auto bytes = bp->makeProtocolMessage();
    return bytes;
}
