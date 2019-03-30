#include "CharacterSet.hpp"

#include <set>

namespace Uri
{
        struct CharacterSet::Impl 
        {
            std::set< char > characters_set;
        };

        CharacterSet::~CharacterSet()noexcept = default;
        CharacterSet::CharacterSet(const CharacterSet& other)
            :impl_(new Impl(*other.impl_))
        {

        }
        CharacterSet& CharacterSet::operator=(const CharacterSet& other)
        {
            if(this != &other)
            {
                *impl_ = *other.impl_;
            }
            return *this;
        }
        CharacterSet::CharacterSet()
            : impl_(new Impl)
        {
        }
        CharacterSet::CharacterSet(CharacterSet&& other) noexcept = default;
       
        CharacterSet::CharacterSet(char c)
            : impl_(new Impl)
            {
                (void)impl_->characters_set.insert(c);
            }

        CharacterSet::CharacterSet(char first, char last)
          : impl_(new Impl)
        {
            for (char c = first; c < last + 1; ++c) 
            {
                (void)impl_->characters_set.insert(c);
            }
        }

        CharacterSet::CharacterSet(std::initializer_list<const CharacterSet> sets)
            : impl_(new Impl)
        {
            for (auto characterSet = sets.begin();
                      characterSet != sets.end();
                      ++characterSet) 
            {
                impl_->characters_set.insert(
                       characterSet->impl_->characters_set.begin(),
                       characterSet->impl_->characters_set.end());
            }
        }

        bool CharacterSet::Contains(char c) const
        {
            return impl_->characters_set.find(c) != impl_->characters_set.end();
        }

}