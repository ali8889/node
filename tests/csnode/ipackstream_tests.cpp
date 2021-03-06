
#include <gtest/gtest.h>
#include "packstream.hpp"

// commented tests require modification of IPackStream
// because they check IPackStream's internal state and
// it is filled with garbage before call to init()

TEST(IPackStream, IsNotGoodWithoutInitialization) {
    cs::IPackStream stream;
    ASSERT_FALSE(stream.good());
}

TEST(IPackStream, IsGoodAfterProperInitialization) {
    cs::IPackStream stream;
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    ASSERT_TRUE(stream.good());
}

TEST(IPackStream, OperatorBoolReturnsFalseWithoutInitialization) {
    cs::IPackStream stream;
    ASSERT_FALSE(static_cast<bool>(stream));
}

TEST(IPackStream, OperatorBoolReturnsTrueAfterInitialization) {
    cs::IPackStream stream;
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    ASSERT_TRUE(static_cast<bool>(stream));
}

/*
TEST(IPackStream, IsAtEndWithoutInitialization) {
  cs::IPackStream stream;
  ASSERT_TRUE(stream.end());
}//*/

TEST(IPackStream, IsNotAtEndAfterProperInitialization) {
    cs::IPackStream stream;
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    ASSERT_FALSE(stream.end());
}

/*
TEST(IPackStream, CanNotPeekWithoutInitialization) {
  cs::IPackStream stream;
  ASSERT_FALSE(stream.canPeek<int>());
}//*/

TEST(IPackStream, CanPeekAfterInitialization) {
    cs::IPackStream stream;
    uint8_t data[] = {0, 1, 2, 3};
    stream.init(data, sizeof data);
    ASSERT_TRUE(stream.canPeek<uint32_t>());
    ASSERT_FALSE(stream.canPeek<uint64_t>());
}

TEST(IPackStream, CurrentPointerIsEqualToThatPassedDuringInitialization) {
    cs::IPackStream stream;
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof(data));
    ASSERT_EQ(data, stream.getCurrentPtr());
}

/*
TEST(IPackStream, CanNotExtractByteArrayWithoutInitialization) {
  IPackStream stream;
  cs::ByteArray<3> string;
  stream >> string;
  ASSERT_EQ(string[0], 0);
  ASSERT_EQ(string[1], 0);
  ASSERT_EQ(string[2], 0);
}//*/

TEST(IPackStream, CanExtractByteArrayAfterInitialization) {
    cs::IPackStream stream;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    cs::ByteArray<3> string;
    stream >> string;
    ASSERT_EQ(string[0], 1);
    ASSERT_EQ(string[1], 2);
    ASSERT_EQ(string[2], 3);
}

TEST(IPackStream, UnsafeSkipCorrectlyWorks) {
    cs::IPackStream stream;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    cs::ByteArray<3> string;
    stream.skip<uint32_t>();
    stream >> string;
    ASSERT_EQ(string[0], 5);
    ASSERT_EQ(string[1], 6);
    ASSERT_EQ(string[2], 7);
}

TEST(IPackStream, SafeSkipCorrectlySkips) {
    cs::IPackStream stream;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    cs::ByteArray<3> string;
    stream.safeSkip<uint32_t>(1);
    stream >> string;
    ASSERT_EQ(string[0], 5);
    ASSERT_EQ(string[1], 6);
    ASSERT_EQ(string[2], 7);
}

TEST(IPackStream, SafeSkipCorrectlyDeclinesTooBigShift) {
    cs::IPackStream stream;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    stream.safeSkip<uint32_t>(2);
    ASSERT_FALSE(stream.good());
}

TEST(IPackStream, IsAtEndAfterSkipToTheEnd) {
    cs::IPackStream stream;
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7};
    stream.init(data, sizeof data);
    stream.safeSkip<uint8_t>(7);
    ASSERT_TRUE(stream.end());
}

TEST(IPackStream, IsAtEndAfterReadTheWholeBlob) {
    cs::IPackStream stream;
    uint8_t data[] = {0, 1, 2, 3};
    stream.init(data, sizeof data);
    uint32_t discarded_value;
    stream >> discarded_value;
    ASSERT_TRUE(stream.end());
}

TEST(IPackStream, SuccessfulReadOfIntegralType) {
    cs::IPackStream stream;
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};
    stream.init(data, sizeof data);
    uint32_t value;
    stream >> value;
    EXPECT_TRUE(stream.good());
    ASSERT_EQ(value, 0x78563412u);
}

TEST(IPackStream, IsNotGoodAfterRequestForReadingTooMany) {
    cs::IPackStream stream;
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    stream.init(data, sizeof data);
    uint64_t value = 0;
    stream >> value;
    EXPECT_FALSE(stream.good());
    ASSERT_EQ(value, 0);
}

TEST(IPackStream, PeekIntegerValue) {
    cs::IPackStream stream;
    uint8_t data[] = {0x12, 0x34, 0x56, 0x78};
    stream.init(data, sizeof data);
    uint32_t value;
    value = stream.peek<decltype(value)>();
    ASSERT_EQ(value, 0x78563412u);
}

void displayStreamData(cs::IPackStream& stream, const size_t& size) {
    auto ptr = stream.getCurrentPtr();

    for (size_t i = 0; i < size; i++) {
        std::cout << "item " << i << ": " << (int)(*(ptr + i)) << std::endl;
    }
}
