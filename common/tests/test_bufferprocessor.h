#include <gtest/gtest.h>

#include <baseprotocol.h>
#include <string>

class Test_BufferProcessor : public ::testing::Test
{
public:
    Test_BufferProcessor() = default;

protected:
    BaseProtocol::Bytes makeProtocolMessage(const std::string &payload);
};

