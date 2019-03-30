#ifndef URI_PERCENT_ENCODED_CHARACTER_DECODER_HPP
#define URI_PERCENT_ENCODED_CHARACTER_DECODER_HPP

#include <memory>
#include <stddef.h>

namespace Uri
{
    class PercentEncodedCharacterDecoder
    {
    public:
        ~PercentEncodedCharacterDecoder()noexcept;
        PercentEncodedCharacterDecoder(const PercentEncodedCharacterDecoder&) = delete;
        PercentEncodedCharacterDecoder(PercentEncodedCharacterDecoder&&) noexcept;
        PercentEncodedCharacterDecoder& operator=(const PercentEncodedCharacterDecoder&) = delete;
        PercentEncodedCharacterDecoder& operator=(PercentEncodedCharacterDecoder&&) noexcept;

    public:
        PercentEncodedCharacterDecoder();
        bool NextEncodedCharacter(char);
        bool Done() const;
        char GetDecodedCharacter()const;

    private:
        struct Impl;
        std::unique_ptr< Impl > impl_;
    };
}


#endif