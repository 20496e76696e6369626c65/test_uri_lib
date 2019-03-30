#include <gtest/gtest.h>
#include <src/PercentEncodedCharacterDecoder.hpp>
#include <stddef.h>
#include <vector>

TEST(PercentEncodedCharacterDecoderTests, GoodSequences) 
{
    Uri::PercentEncodedCharacterDecoder pec;
    struct TestVector 
    {
        char sequence[2];
        char expected_output;
    };
    const std::vector< TestVector > test_vectors
    {
        {{'4', '1'}, 'A'},
        {{'5', 'A'}, 'Z'},
        {{'6', 'e'}, 'n'},
        {{'e', '1'}, (char)0xe1},
        {{'C', 'A'}, (char)0xca},
    };
    size_t index = 0;
    for (auto test_vector: test_vectors) 
    {
        pec = Uri::PercentEncodedCharacterDecoder();
        ASSERT_FALSE(pec.Done());
        ASSERT_TRUE(pec.NextEncodedCharacter(test_vector.sequence[0]));
        ASSERT_FALSE(pec.Done());
        ASSERT_TRUE(pec.NextEncodedCharacter(test_vector.sequence[1]));
        ASSERT_TRUE(pec.Done());
        ASSERT_EQ(test_vector.expected_output, pec.GetDecodedCharacter()) << index;
        ++index;
    }
}

TEST(PercentEncodedCharacterDecoderTests, BadSequences) 
{
    Uri::PercentEncodedCharacterDecoder pec;
    std::vector< char > test_vectors
    {
        'G', 'g', '.', 'z', '-', ' ', 'V',
    };
    for (auto test_vector: test_vectors) 
    {
        pec = Uri::PercentEncodedCharacterDecoder();
        ASSERT_FALSE(pec.Done());
        ASSERT_FALSE(pec.NextEncodedCharacter(test_vector));
    }
}