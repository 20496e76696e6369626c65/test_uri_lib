#ifndef URI_CHARACTER_SET_HPP
#define URI_CHARACTER_SET_HPP

#include <initializer_list>
#include <memory>

namespace Uri
{
    class CharacterSet 
    {
    public:
        ~CharacterSet() noexcept;
        CharacterSet(const CharacterSet&);
        CharacterSet(CharacterSet&&) noexcept;
        CharacterSet& operator=(const CharacterSet&);
        CharacterSet& operator=(CharacterSet&&) noexcept;

    public:
        CharacterSet();
        CharacterSet(char);
        CharacterSet(char, char);
        CharacterSet(std::initializer_list<const CharacterSet>);
        bool Contains(char) const;
    private:
        struct Impl;
        std::unique_ptr< Impl > impl_;

    };

}

#endif