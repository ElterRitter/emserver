#include "test_protocol.h"

#include "baseprotocol.h"

#include <boost/crc.hpp>

#include <string>

using std::string;

void Test_BaseProtocol::SetUp()
{

}

void Test_BaseProtocol::TearDown()
{

}

TEST_F(Test_BaseProtocol, SimpleCreateMessage)
{
    string payload = "test";
    auto proto = BaseProtocol::create(payload);
    ASSERT_TRUE(proto != nullptr);
    ASSERT_EQ(payload, proto->payload());
    ASSERT_EQ(payload.size(), proto->payloadSize());

    auto bytes = proto->makeProtocolMessage();
    ASSERT_TRUE(bytes.size() > payload.size());
    ASSERT_TRUE(bytes.size() == proto->fullSize());
    ASSERT_TRUE(bytes.front() == '$');

    uint32_t field = 0;
    memcpy(&field, bytes.data()+1, sizeof(field));  // start symbol
    ASSERT_TRUE(field == payload.size());

    boost::crc_32_type crc;
    crc.process_bytes(bytes.data(), bytes.size() - sizeof(field));
    memcpy(&field, &bytes.data()[bytes.size()-sizeof(field)], sizeof(field));
    ASSERT_TRUE(crc.checksum() == field);
}

TEST_F(Test_BaseProtocol, SimpleExtractionMessage)
{
    string payload = "test";
    auto proto = BaseProtocol::create(payload);
    ASSERT_TRUE(proto != nullptr);

    auto bytes = proto->makeProtocolMessage();
    ASSERT_TRUE(bytes.size() > 0);

    BaseProtocol::ExtractionFailure reason;
    auto extractedProto = BaseProtocol::extractFromRawData(bytes.data(), bytes.size(), reason);
    ASSERT_EQ(payload, extractedProto->payload());
}

TEST_F(Test_BaseProtocol, MessageWithBucks)
{
    string payload = "te$$ting";
    auto proto = BaseProtocol::create(payload);
    ASSERT_TRUE(proto != nullptr);

    auto bytes = proto->makeProtocolMessage();
    ASSERT_TRUE(bytes.size() > 0);

    BaseProtocol::ExtractionFailure reason;
    auto extractedProto = BaseProtocol::extractFromRawData(bytes.data(), bytes.size(), reason);
    ASSERT_EQ(payload, extractedProto->payload());
}

TEST_F(Test_BaseProtocol, TwoMessages)
{
    string payload_one = "testing_one", payload_two = "testing_two";
    auto protoOne = BaseProtocol::create(payload_one);
    auto protoTwo = BaseProtocol::create(payload_two);

    auto bytesOne = protoOne->makeProtocolMessage();
    auto bytesTwo = protoTwo->makeProtocolMessage();
    bytesOne.insert(bytesOne.end(), bytesTwo.begin(), bytesTwo.end());

    BaseProtocol::ExtractionFailure reason;
    auto extractedProtoOne = BaseProtocol::extractFromRawData(bytesOne.data(), bytesOne.size(), reason);
    ASSERT_TRUE(extractedProtoOne != nullptr);

    auto extractedPayloadOne = extractedProtoOne->payload();
    ASSERT_EQ(extractedPayloadOne, payload_one);

    bytesOne.erase(bytesOne.begin(), bytesOne.begin() + extractedProtoOne->fullSize());
    auto extractedProtoTwo = BaseProtocol::extractFromRawData(bytesOne.data(), bytesOne.size(), reason);
    ASSERT_TRUE(extractedProtoTwo != nullptr);

    auto extractedPayloadTwo = extractedProtoTwo->payload();
    ASSERT_EQ(extractedPayloadTwo, payload_two);
    ASSERT_EQ(extractedProtoTwo->fullSize(), bytesOne.size());
}

TEST_F(Test_BaseProtocol, TwoHalfMessages)
{
    string payloadOne = "testingOne", payloadTwo = "testingTwo";
    auto protoOne = BaseProtocol::create(payloadOne);
    ASSERT_TRUE(protoOne != nullptr);

    auto protoTwo = BaseProtocol::create(payloadTwo);
    ASSERT_TRUE(protoTwo);

    auto bytesOne = protoOne->makeProtocolMessage();
    ASSERT_TRUE(bytesOne.size() > 0);

    auto bytesTwo = protoTwo->makeProtocolMessage();
    ASSERT_TRUE(bytesTwo.size() > 0);

    BaseProtocol::Bytes splittedData;
    auto it = bytesOne.begin() + 3;
    std::copy(bytesOne.begin(), it, std::back_inserter(splittedData));
    ASSERT_TRUE(splittedData.size() > 0);

    BaseProtocol::ExtractionFailure reason;
    auto extractedOne = BaseProtocol::extractFromRawData(splittedData.data(), splittedData.size(), reason);
    ASSERT_TRUE(extractedOne == nullptr);

    std::copy(it, bytesOne.end(), std::back_inserter(splittedData));
    extractedOne = BaseProtocol::extractFromRawData(splittedData.data(), splittedData.size(), reason);
    ASSERT_TRUE(extractedOne != nullptr);

    auto extractedPayload = extractedOne->payload();
    ASSERT_EQ(extractedPayload, payloadOne);

    splittedData.erase(splittedData.begin(), splittedData.begin()+3);
    it = bytesTwo.begin() + 3;
    std::copy(bytesTwo.begin(), it, std::back_inserter(splittedData));
    extractedOne = BaseProtocol::extractFromRawData(splittedData.data(), splittedData.size(), reason);
    ASSERT_TRUE(extractedOne == nullptr);

    std::copy(it, bytesTwo.end(), std::back_inserter(splittedData));
    extractedOne = BaseProtocol::extractFromRawData(splittedData.data(), splittedData.size(), reason);
    ASSERT_TRUE(extractedOne == nullptr);

    splittedData.erase(splittedData.begin(), splittedData.begin() + protoOne->fullSize() - 3);
    extractedOne = BaseProtocol::extractFromRawData(splittedData.data(), splittedData.size(), reason);
    ASSERT_TRUE(extractedOne != nullptr);

    extractedPayload = extractedOne->payload();
    ASSERT_EQ(payloadTwo, extractedPayload);
}
