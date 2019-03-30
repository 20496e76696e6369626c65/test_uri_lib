#include "PercentEncodedCharacterDecoder.hpp"
#include "CharacterSet.hpp"

namespace
{
    const Uri::CharacterSet DIGIT('0', '9');
    const Uri::CharacterSet HEX_UPPER('A', 'F');
    const Uri::CharacterSet HEX_LOWER('a', 'f');
}
        
namespace Uri
{        
        struct PercentEncodedCharacterDecoder::Impl
        {
            int decoded_character = 0;
            size_t digits_left = 2;

            bool ShiftInHexDigit(char c) 
            {
                decoded_character <<= 4;
                if (DIGIT.Contains(c))
                {
                    decoded_character += (int)(c - '0');
                } 
                else if (HEX_UPPER.Contains(c)) 
                {
                    decoded_character += (int)(c - 'A') + 10;
                } 
                else if (HEX_LOWER.Contains(c)) 
                {
                    decoded_character += (int)(c - 'a') + 10;
                }
                else 
                {
                    return false;
                }
                return true;
            }
        };
        PercentEncodedCharacterDecoder::~PercentEncodedCharacterDecoder() noexcept = default;
        PercentEncodedCharacterDecoder::PercentEncodedCharacterDecoder(PercentEncodedCharacterDecoder&&) noexcept = default;
        PercentEncodedCharacterDecoder& PercentEncodedCharacterDecoder::operator=(PercentEncodedCharacterDecoder&&) noexcept = default;

        PercentEncodedCharacterDecoder::PercentEncodedCharacterDecoder()
            : impl_(new Impl)
        {
        }

        bool PercentEncodedCharacterDecoder::NextEncodedCharacter(char c)
        {
            if (!impl_->ShiftInHexDigit(c)) 
            {
                return false;
            }
            --impl_->digits_left;
            return true;
        }
        bool PercentEncodedCharacterDecoder::Done() const
        {
            return (impl_->digits_left == 0);
        }
        char PercentEncodedCharacterDecoder::GetDecodedCharacter()const
        {
            return (char)impl_->decoded_character;
        }
}