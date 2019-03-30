

#include "CharacterSet.hpp"
#include "PercentEncodedCharacterDecoder.hpp"
#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <inttypes.h>
#include <stdint.h>
#include <functional>
#include <Uri/Uri.hpp>


namespace 
{
    const Uri::CharacterSet ALPHA{
        Uri::CharacterSet('a', 'z'),
        Uri::CharacterSet('A', 'Z')
    };

    const Uri::CharacterSet DIGIT('0', '9');

    const Uri::CharacterSet HEXDIG{
        Uri::CharacterSet('0', '9'),
        Uri::CharacterSet('A', 'F'),
        Uri::CharacterSet('a', 'f')
    };

    const Uri::CharacterSet UNRESERVED{
        ALPHA,
        DIGIT,
        '-', '.', '_', '~'
    };

    const Uri::CharacterSet SUB_DELIMS{
        '!', '$', '&', '\'', '(', ')',
        '*', '+', ',', ';', '='
    };

    const Uri::CharacterSet SCHEME_NOT_FIRST{
        ALPHA,
        DIGIT,
        '+', '-', '.',
    };

    const Uri::CharacterSet PCHAR_NOT_PCT_ENCODED{
        UNRESERVED,
        SUB_DELIMS,
        ':', '@'
    };

    const Uri::CharacterSet QUERY_OR_FRAGMENT_NOT_PCT_ENCODED{
        PCHAR_NOT_PCT_ENCODED,
        '/', '?'
    };

    const Uri::CharacterSet QUERY_NOT_PCT_ENCODED_WITHOUT_PLUS{
        UNRESERVED,
        '!', '$', '&', '\'', '(', ')',
        '*', ',', ';', '=',
        ':', '@',
        '/', '?'
    };

  
    const Uri::CharacterSet USER_INFO_NOT_PCT_ENCODED{
        UNRESERVED,
        SUB_DELIMS,
        ':',
    };

   
    const Uri::CharacterSet REG_NAME_NOT_PCT_ENCODED{
        UNRESERVED,
        SUB_DELIMS
    };
    const Uri::CharacterSet IPV_FUTURE_LAST_PART{
        UNRESERVED,
        SUB_DELIMS,
        ':'
    };

    std::function< bool(char, bool) > LegalSchemeCheckStrategy() 
    {
        auto is_first_character = std::make_shared< bool >(true);
        return [is_first_character](char c, bool end)
        {
            if (end) 
            {
                return !*is_first_character;
            } 
            else 
            {
                bool check;
                if (*is_first_character) 
                {
                    check = ALPHA.Contains(c);
                } 
                else 
                {
                    check = SCHEME_NOT_FIRST.Contains(c);
                }
                *is_first_character = false;
                return check;
            }
        };
    }

    enum class ToIntegerResult 
    {
        SUCCESS,
        NOTNUMBER,
        OVERFLOW_
    };

    ToIntegerResult ToInteger( const std::string& number_string, intmax_t& number) 
    {
        size_t index = 0;
        size_t state = 0;
        bool negative = false;
        intmax_t value = 0;
        while (index < number_string.size()) 
        {
            switch (state) 
            {
                case 0: 
                {
                    if (number_string[index] == '-') 
                    {
                        negative = true;
                        ++index;
                    }
                    state = 1;
                } break;

                case 1:
                {
                    if (number_string[index] == '0') {
                        state = 2;
                    } 
                    else if ((number_string[index] >= '1') && (number_string[index] <= '9'))
                    {
                        state = 3;
                        value = (decltype(value))(number_string[index] - '0');
                        value = (value * (negative ? -1 : 1));
                    } 
                    else 
                    {
                        return ToIntegerResult::NOTNUMBER;
                    }
                    ++index;
                } break;

                case 2: 
                { 
                    return ToIntegerResult::NOTNUMBER;
                } break;

                case 3: 
                { 
                    if ((number_string[index] >= '0') && (number_string[index] <= '9')) 
                    {
                        const auto digit = (decltype(value))(number_string[index] - '0');
                        if (negative) 
                        {
                            if ((std::numeric_limits< decltype(value) >::lowest() + digit) / 10 > value) 
                            {
                                return ToIntegerResult::OVERFLOW_;
                            }
                        } 
                        else 
                        {
                            if ((std::numeric_limits< decltype(value) >::max() - digit) / 10 < value) 
                            {
                                return ToIntegerResult::OVERFLOW_;
                            }
                        }
                        value *= 10;
                        if (negative) 
                        {
                            value -= digit;
                        } 
                        else 
                        {
                            value += digit;
                        }
                        ++index;
                    } 
                    else
                    {
                        return ToIntegerResult::NOTNUMBER;
                    }
                } break;
            }
        }
        if (state >= 2) 
        {
            number = value;
            return ToIntegerResult::SUCCESS;
        } 
        else 
        {
            return ToIntegerResult::NOTNUMBER;
        }
    }

    

    std::string ToLower(const std::string& in_string) 
    {
        std::string out_string;
        out_string.reserve(in_string.size());
        for (char c: in_string) 
        {
            out_string.push_back(tolower(c));
        }
        return out_string;
    }

    bool FailsMatch(const std::string& candidate, std::function< bool(char, bool) > still_passing) {
        for (const auto c: candidate) 
        {
            if (!still_passing(c, false)) 
            {
                return true;
            }
        }
        return !still_passing(' ', true);
    }

    bool ValidateOctet(const std::string& octet_string) 
    {
        int octet = 0;
        for (auto c: octet_string) 
        {
            if (DIGIT.Contains(c)) 
            {
                octet *= 10;
                octet += (int)(c - '0');
            } 
            else 
            {
                return false;
            }
        }
        return (octet <= 255);
    }

       bool ValidateIpv4Adress(const std::string& address) {
        size_t num_groups = 0;
        size_t state = 0;
        std::string octet_buffer;
        for (auto c: address) 
        {
            switch (state) 
            {
                case 0: 
                {
                    if (DIGIT.Contains(c)) 
                    {
                        octet_buffer.push_back(c);
                        state = 1;
                    } 
                    else 
                    {
                        return false;
                    }
                } break;

                case 1: 
                { 
                    if (c == '.') 
                    {
                        if (num_groups++ >= 4) 
                        {
                            return false;
                        }
                        if (!ValidateOctet(octet_buffer)) 
                        {
                            return false;
                        }
                        octet_buffer.clear();
                        state = 0;
                    } 
                    else if (DIGIT.Contains(c)) 
                    {
                        octet_buffer.push_back(c);
                    } 
                    else 
                    {
                        return false;
                    }
                } break;
            }
        }
        if (!octet_buffer.empty()) 
        {
            ++num_groups;
            if (!ValidateOctet(octet_buffer)) {
                return false;
            }
        }
        return (num_groups == 4);
    }

    bool ValidateIpv6Address(const std::string& address) 
    {
        
        enum class ValidationState 
        {
            NO_GROUPS_YET,
            COLON_BUT_NO_GROUPS_YET,
            AFTER_COLON_EXPECT_GROUP_OR_IPV4,
            IN_GROUP_NOT_IPV4,
            IN_GROUP_COULD_BE_IPV4,
            COLON_AFTER_GROUP,
        } state = ValidationState::NO_GROUPS_YET;
       
        size_t num_groups = 0;
        size_t num_digits = 0;
        size_t ipv4_address_start = 0;
        size_t position = 0;

        bool double_colon_encountered = false;
        bool ipv4_address_encountered = false;

        for (auto c: address) 
        {
            switch (state) 
            {
                case ValidationState::NO_GROUPS_YET: 
                {
                    if (c == ':') 
                    {
                        state = ValidationState::COLON_BUT_NO_GROUPS_YET;
                    } 
                    else if (DIGIT.Contains(c)) 
                    {
                        ipv4_address_start = position;
                        ++num_digits = 1;
                        state = ValidationState::IN_GROUP_COULD_BE_IPV4;
                    } 
                    else if (HEXDIG.Contains(c)) 
                    {
                        ++num_digits = 1;
                        state = ValidationState::IN_GROUP_NOT_IPV4;
                    } 
                    else 
                    {
                        return false;
                    }
                } break;

                case ValidationState::COLON_BUT_NO_GROUPS_YET: 
                {
                    if (c == ':') 
                    {
                        if (double_colon_encountered) 
                        {
                            return false;
                        } 
                        else 
                        {
                            double_colon_encountered = true;
                            state = ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4;
                        }
                    } 
                    else 
                    {
                        return false;
                    }
                } break;

                case ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4: 
                {
                    if (DIGIT.Contains(c)) 
                    {
                        ipv4_address_start = position;
                        if (++num_digits > 4) 
                        {
                            return false;
                        }
                        state = ValidationState::IN_GROUP_COULD_BE_IPV4;
                    } 
                    else if (HEXDIG.Contains(c)) 
                    {
                        if (++num_digits > 4) 
                        {
                            return false;
                        }
                        state = ValidationState::IN_GROUP_NOT_IPV4;
                    } 
                    else 
                    {
                        return false;
                    }
                } break;

                case ValidationState::IN_GROUP_NOT_IPV4: 
                {
                    if (c == ':') 
                    {
                        num_digits = 0;
                        ++num_groups;
                        state = ValidationState::COLON_AFTER_GROUP;
                    } 
                    else if (HEXDIG.Contains(c)) 
                    {
                        if (++num_digits > 4) 
                        {
                            return false;
                        }
                    } 
                    else 
                    {
                        return false;
                    }
                } break;

                case ValidationState::IN_GROUP_COULD_BE_IPV4: 
                {
                    if (c == ':') 
                    {
                        num_digits = 0;
                        ++num_groups;
                        state = ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4;
                    } 
                    else if (c == '.') 
                    {
                        ipv4_address_encountered = true;
                        break;
                    } 
                    else if (DIGIT.Contains(c)) 
                    {
                        if (++num_digits > 4) 
                        {
                            return false;
                        }
                    } 
                    else if (HEXDIG.Contains(c)) 
                    {
                        if (++num_digits > 4) 
                        {
                            return false;
                        }
                        state = ValidationState::IN_GROUP_NOT_IPV4;
                    } 
                    else 
                    {
                        return false;
                    }
                } break;

                case ValidationState::COLON_AFTER_GROUP: 
                {
                    if (c == ':') 
                    {
                        if (double_colon_encountered) 
                        {
                            return false;
                        } 
                        else 
                        {
                            double_colon_encountered = true;
                            state = ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4;
                        }
                    } 
                    else if (DIGIT.Contains(c)) 
                    {
                        ipv4_address_start = position;
                        ++num_digits;
                        state = ValidationState::IN_GROUP_COULD_BE_IPV4;
                    } 
                    else if (HEXDIG.Contains(c)) 
                    {
                        ++num_digits;
                        state = ValidationState::IN_GROUP_NOT_IPV4;
                    } 
                    else 
                    {
                        return false;
                    }
                } break;
            }
            if (ipv4_address_encountered) 
            {
                break;
            }
            ++position;
        }
        if ((state == ValidationState::IN_GROUP_NOT_IPV4)
            || (state == ValidationState::IN_GROUP_COULD_BE_IPV4)) 
        {
            ++num_groups;
        }
        if ((position == address.length())
            && ((state == ValidationState::COLON_BUT_NO_GROUPS_YET)
             || (state == ValidationState::AFTER_COLON_EXPECT_GROUP_OR_IPV4)
             || (state == ValidationState::COLON_AFTER_GROUP))) 
        { 
            return false;
        }
        if (ipv4_address_encountered) 
        {
            if (!ValidateIpv4Adress(address.substr(ipv4_address_start))) 
            {
                return false;
            }
            num_groups += 2;
        }
        if (double_colon_encountered) 
        {
            return (num_groups <= 7);
        } 
        else 
        {
            return (num_groups == 8);
        }
    }

    bool DecodeElement(std::string& element, const Uri::CharacterSet& allowed_characters)
    {
        const auto original_segment = std::move(element);
        element.clear();
        bool decoding_pec = false;
        Uri::PercentEncodedCharacterDecoder pec_decoder;
        for (const auto c: original_segment) 
        {
            if (decoding_pec) 
            {
                if (!pec_decoder.NextEncodedCharacter(c)) 
                {
                    return false;
                }
                if (pec_decoder.Done()) 
                {
                    decoding_pec = false;
                    element.push_back((char)pec_decoder.GetDecodedCharacter());
                }
            } 
            else if (c == '%') 
            {
                decoding_pec = true;
                pec_decoder = Uri::PercentEncodedCharacterDecoder();
            } 
            else 
            {
                if (allowed_characters.Contains(c)) 
                {
                    element.push_back(c);
                } 
                else 
                {
                    return false;
                }
            }
        }
        return true;
    }

    char MakeHexDigit(unsigned int value)
    {
        if ((value >= 0) && (value < 10))
        {
            return (char)(value + '0');
        }

        if ((value >= 10) && (value < 16))
        {
            return (char)(value - 10 + 'A');
        }

        return (char)value;
    }

    std::string EncodeElement(const std::string& element, const Uri::CharacterSet& allowed_characters) 
    {
        std::string encoded_element;
        for (uint8_t c: element) 
        {
            if (allowed_characters.Contains(c)) 
            {
                encoded_element.push_back(c);
            } 
            else 
            {
                encoded_element.push_back('%');
                encoded_element.push_back(MakeHexDigit((unsigned int)c >> 4));
                encoded_element.push_back(MakeHexDigit((unsigned int)c & 0x0F));
            }
        }
        return encoded_element;
    }

    bool DecodeQueryOrFragment(std::string& query_or_fragment)
    {
        return DecodeElement(query_or_fragment, QUERY_OR_FRAGMENT_NOT_PCT_ENCODED);
    }

  

} 

namespace Uri
{
    struct Uri::Impl
    {
       
        std::string scheme;
        std::string host;
        std::string user_info;
        bool has_port = false;
        uint16_t port = 0;
        bool has_query = false;
        std::string  query;
        bool has_fragment = false;
        std::string fragment;
        std::vector<std::string> path;
       
        bool ParseAuthority(const std::string& authority_string)
        {
            enum class HostParsingState
            {
                FIRST_CHARACTER,
                NOT_IP_LITERAL,
                PERCENT_ENCODED_CHARACTER,
                IP_LITERAL,
                IPV6_ADDRESS,
                IPV_FUTURE_NUMBER,
                IPV_FUTURE_BODY,
                GARBAGE_CHECK,
                PORT,
            };

            const auto user_info_delimiter = authority_string.find('@');
            std::string host_port_string;
            user_info.clear();
            if (user_info_delimiter == std::string::npos)
            {
                host_port_string = authority_string;
            } 
            else 
            {
                user_info = authority_string.substr(0, user_info_delimiter);
                if (!DecodeElement(user_info, USER_INFO_NOT_PCT_ENCODED)) 
                {
                    return false;
                }
                host_port_string = authority_string.substr(user_info_delimiter + 1);
            }

           
            std::string port_string;
            HostParsingState host_parsing_state = HostParsingState::FIRST_CHARACTER;
            host.clear();
            PercentEncodedCharacterDecoder pec_decoder;
            bool hostIsRegName = false;
            for (const auto c: host_port_string) 
            {
                switch(host_parsing_state) 
                {
                    case HostParsingState::FIRST_CHARACTER: 
                    {
                        if (c == '[') 
                        {
                            host_parsing_state = HostParsingState::IP_LITERAL;
                            break;
                        } 
                        else 
                        {
                            host_parsing_state = HostParsingState::NOT_IP_LITERAL;
                            hostIsRegName = true;
                        }
                    }

                    case HostParsingState::NOT_IP_LITERAL: 
                    {
                        if (c == '%') 
                        {
                            pec_decoder = PercentEncodedCharacterDecoder();
                            host_parsing_state = HostParsingState::PERCENT_ENCODED_CHARACTER;
                        } 
                        else if (c == ':')
                        {
                            host_parsing_state = HostParsingState::PORT;
                        } 
                        else 
                        {
                            if (REG_NAME_NOT_PCT_ENCODED.Contains(c)) 
                            {
                                host.push_back(c);
                            }
                            else 
                            {
                                return false;
                            }
                        }
                    } break;

                    case HostParsingState::PERCENT_ENCODED_CHARACTER: 
                    {
                        if (!pec_decoder.NextEncodedCharacter(c)) 
                        {
                            return false;
                        }
                        if (pec_decoder.Done()) 
                        {
                            host_parsing_state = HostParsingState::NOT_IP_LITERAL;
                            host.push_back((char)pec_decoder.GetDecodedCharacter());
                        }
                    } break;

                    case HostParsingState::IP_LITERAL: 
                    {
                        if (c == 'v') 
                        {
                            host.push_back(c);
                            host_parsing_state = HostParsingState::IPV_FUTURE_NUMBER;
                            break;
                        } 
                        else 
                        {
                            host_parsing_state = HostParsingState::IPV6_ADDRESS;
                        }
                    }

                    case HostParsingState::IPV6_ADDRESS: 
                    {
                        if (c == ']') 
                        {
                            if (!ValidateIpv6Address(host)) 
                            {
                                return false;
                            }
                            host_parsing_state = HostParsingState::GARBAGE_CHECK;
                        } 
                        else 
                        {
                            host.push_back(c);
                        }
                    } break;

                    case HostParsingState::IPV_FUTURE_NUMBER: 
                    {
                        if (c == '.')
                        {
                           host_parsing_state = HostParsingState::IPV_FUTURE_BODY;
                        } 
                        else if (!HEXDIG.Contains(c)) 
                        {
                            return false;
                        }
                        host.push_back(c);
                    } break;

                    case HostParsingState::IPV_FUTURE_BODY: 
                    {
                        if (c == ']') 
                        {
                            host_parsing_state = HostParsingState::GARBAGE_CHECK;
                        } 
                        else if (!IPV_FUTURE_LAST_PART.Contains(c))
                        {
                            return false;
                        } 
                        else 
                        {
                            host.push_back(c);
                        }
                    } break;

                    case HostParsingState::GARBAGE_CHECK: 
                    {                
                        if (c == ':') 
                        {
                            host_parsing_state = HostParsingState::PORT;
                        } 
                        else 
                        {
                            return false;
                        }
                    } break;

                    case HostParsingState::PORT:
                    {
                        port_string.push_back(c);
                    } break;
                }
            }
            if (   (host_parsing_state != HostParsingState::FIRST_CHARACTER)
                && (host_parsing_state != HostParsingState::NOT_IP_LITERAL)
                && (host_parsing_state != HostParsingState::GARBAGE_CHECK)
                && (host_parsing_state != HostParsingState::PORT))
            {
                return false;
            }
            if (hostIsRegName) 
            {
                host = ToLower(host);
            }
            if (port_string.empty()) 
            {
                has_port = false;
            } else 
            {
                intmax_t port_as_int;
                if (ToInteger(
                        port_string,
                        port_as_int
                    ) != ToIntegerResult::SUCCESS) 
                {
                    return false;
                }
                if ((port_as_int < 0)
                    || (port_as_int > (decltype(port_as_int))std::numeric_limits< decltype(port) >::max())) 
                {
                    return false;
                }
                port = (decltype(port))port_as_int;
                has_port = true;
            }
            return true;
        }

        bool ParseScheme (const std::string& uri_string, std::string& rest)
        {
            auto authority_or_path_delimiter_start = uri_string.find('/');
            if (authority_or_path_delimiter_start == std::string::npos) 
            {
                authority_or_path_delimiter_start = uri_string.length();
            }
            const auto scheme_end = uri_string.substr(0, authority_or_path_delimiter_start).find(':');
            if (scheme_end == std::string::npos) {
                scheme.clear();
                rest = uri_string;
            } else {
                scheme = uri_string.substr(0, scheme_end);
                if (
                    FailsMatch(
                        scheme,
                        LegalSchemeCheckStrategy()
                    )
                ) {
                    return false;
                }
                scheme = ToLower(scheme);
                rest = uri_string.substr(scheme_end + 1);
            }
            return true;
        }
        bool ParseFragment(const std::string& query_fragment, std::string rest)
        {
            const auto fragment_delimiter = query_fragment.find('#');
            if (fragment_delimiter == std::string::npos) {
                has_fragment = false;
                fragment.clear();
                rest = query_fragment;
            } else {
                has_fragment = true;
                fragment = query_fragment.substr(fragment_delimiter + 1);
                rest = query_fragment.substr(0, fragment_delimiter);
            }
            return DecodeQueryOrFragment(fragment);
        }
        bool ParsePath(std::string path_string)
        {
            path.clear();
            if(path_string == "/")
            {
                path.push_back("");
                path_string.clear();
            }
            else if(!path_string.empty())
            {
                for(;;)
                {
                    auto path_delimiter = path_string.find('/');
                    if(path_delimiter == std::string::npos)
                    {
                        path.push_back(path_string);
                        path_string.clear();
                        break;
                    }
                    else
                    {
                        path.emplace_back(path_string.begin(), path_string.begin() + path_delimiter);
                        path_string = path_string.substr(path_delimiter + 1);    
                    }
                    
                }
            }
            for(auto& segment : path)
            {
                if(!DecodeElement(segment, PCHAR_NOT_PCT_ENCODED))
                {
                    return false;
                }
            }
            return true;

        }

        bool ParseQuery(const std::string& query_with_delimiter)
        {
            has_query = !query_with_delimiter.empty();
            if(has_query)
            {
                query = query_with_delimiter.substr(1);
            }
            else
            {
                query.clear();
            }
            return DecodeQueryOrFragment(query);
        }

        bool SplitAuthorityFromPathAndParseIt( std::string author_path_string, 
                                               std::string& path_string)
        {
                if (author_path_string.substr(0, 2) == "//") 
                {
                author_path_string = author_path_string.substr(2);
                auto authorityEnd =author_path_string.find('/');
                if (authorityEnd == std::string::npos) 
                {
                    authorityEnd = author_path_string.length();
                }

                path_string = author_path_string.substr(authorityEnd);
                auto authorityString = author_path_string.substr(0, authorityEnd);

                if (!ParseAuthority(authorityString)) 
                {
                    return false;
                }
            }
            else 
            {
                user_info.clear();
                host.clear();
                has_port = false;
                path_string = author_path_string;
            }
            return true;
        }
      
        void SetDefaultPathIfAuthorityPresentAndPathEmpty()
        {
            if(!host.empty() && path.empty())
            {
                path.push_back("");
            }
        }

    

        void NormalizePath() 
        {
            auto old_path = std::move(path);
            path.clear();
            bool directory_level = false;
            for (const auto segment: old_path) 
            {
                if (segment == ".") 
                {
                    directory_level = true;
                } 
                else if (segment == "..") 
                {
                    if (!path.empty()) 
                    {
                        if (CanNavigatePathUpOneLevel()) 
                        {
                            path.pop_back();
                        }
                    }
                    directory_level = true;
                } 
                else 
                {  
                    if (!directory_level || !segment.empty()) 
                    {
                        path.push_back(segment);
                    }
                    directory_level = segment.empty();
                }
            }
            if (directory_level&& (!path.empty()&& !path.back().empty())) 
            {
                path.push_back("");
            }
        }

        void CopyScheme(const Uri& other)
        {
            scheme = other.impl_->scheme;
        }

        void CopyAuthority(const Uri& other)
        {
            host = other.impl_->host;
            user_info = other.impl_->user_info;
            has_port = other.impl_->has_port;
            port = other.impl_->port;
        }

        void CopyPath(const Uri& other)
        {
            path = other.impl_->path;
        }

        void CopyQuery(const Uri& other)
        {
            has_query = other.impl_->has_query;
            query = other.impl_->query;
        }

        void CopyFragment(const Uri& other)
        {
            has_fragment = other.impl_->has_fragment;
            fragment = other.impl_->fragment;
        }
        void CopyAndNormalizePath(const Uri& other)
        {
            CopyPath(other);
            NormalizePath();
        }
        bool HasAuthority() const 
        {
            return(!host.empty() || !user_info.empty() || has_port);
        }

        bool IsPathAbsolute() const 
        {
            return (!path.empty() && (path[0] == ""));
        }

        bool CanNavigatePathUpOneLevel() const 
        {
            return (!IsPathAbsolute()|| (path.size()>1));
        }
    };



    Uri::~Uri()noexcept = default;

    Uri::Uri(const Uri& tmp)
        :impl_(new Impl)
        {
            *this = tmp;
        }   

    Uri::Uri(Uri&&) noexcept = default;

    Uri& Uri::operator=(const Uri& other) {
        if (this != &other) {
            *impl_ = *other.impl_;
        }
        return *this;
    }
    Uri& Uri::operator=(Uri&&) noexcept = default;

    Uri::Uri()
        : impl_(new Impl)
    {
    }

     bool Uri::operator==(const Uri& other) const 
     {
     return (
            (impl_->scheme == other.impl_->scheme)
            && (impl_->user_info == other.impl_->user_info)
            && (impl_->host == other.impl_->host)
            && (
                (!impl_->has_port && !other.impl_->has_port)
                || (
                    (impl_->has_port && other.impl_->has_port)
                    && (impl_->port == other.impl_->port)
                )
            )
            && (impl_->path == other.impl_->path)
            && (
                (!impl_->has_query && !other.impl_->has_query)
                || (
                    (impl_->has_query && other.impl_->has_query)
                    && (impl_->query == other.impl_->query)
                )
            )
            && (
                (!impl_->has_fragment && !other.impl_->has_fragment)
                || (
                    (impl_->has_fragment && other.impl_->has_fragment)
                    && (impl_->fragment == other.impl_->fragment)
                )
            )
           );
    }

    bool Uri::operator!=(const Uri& other) const 
    {
        return !(*this == other);
    }

    bool Uri::ParseFromString(const std::string& uri_string)
    {
        std::string rest;
        if (!impl_->ParseScheme(uri_string, rest)) 
        {
            return false;
        }
        const auto path_end = rest.find_first_of("?#");
        const auto authority_and_path_string = rest.substr(0, path_end);
        const auto query_and_or_fragment = rest.substr(authority_and_path_string.length());
        std::string path_string;
        if (!impl_->SplitAuthorityFromPathAndParseIt(authority_and_path_string, path_string))
        {
            return false;
        }
        if (!impl_->ParsePath(path_string)) 
        {
            return false;
        }
        impl_->SetDefaultPathIfAuthorityPresentAndPathEmpty();
        if (!impl_->ParseFragment(query_and_or_fragment, rest)) 
        {
            return false;
        }
        return impl_->ParseQuery(rest);

    }


    std::string Uri::GetScheme() const
    {
        return impl_->scheme;
    }
    std::string Uri::GetHost() const
    {
        return impl_->host;
    }
    std::vector<std::string> Uri::GetPath() const
    {
        return impl_->path;
    }
    
    bool Uri::HasPort() const
    {
        return impl_->has_port;
    }

    uint16_t Uri::GetPort() const
    {
        return impl_->port;
    }

    bool Uri::HasQuery() const
    {
        return impl_->has_query;
    }

    std::string Uri::GetQuery() const
    {
        return impl_->query;
    }

    bool Uri::IsRelativeReference() const
    {
        return impl_->scheme.empty();
    }

    bool Uri::HasFragment() const
    {
        return impl_->has_fragment;
    }

    std::string Uri::GetFragment() const
    {
        return impl_->fragment;
    }
    
    bool Uri::ContainsRelativePath() const
    {
        return !impl_->IsPathAbsolute();
    }

    std::string Uri::GetUserInfo() const
    {
        return impl_->user_info;
    }


    void Uri::NormalizePath()
    {

    }
    Uri Uri::Resolve (const Uri& relative_ref) const
    {
        Uri target;
        if (!relative_ref.impl_->scheme.empty()) 
        {
            target.impl_->CopyScheme(relative_ref);
            target.impl_->CopyAuthority(relative_ref);
            target.impl_->CopyAndNormalizePath(relative_ref);
            target.impl_->CopyQuery(relative_ref);
        }
        else 
        {
            if (!relative_ref.impl_->host.empty()) 
            {
                target.impl_->CopyAuthority(relative_ref);
                target.impl_->CopyAndNormalizePath(relative_ref);
                target.impl_->CopyQuery(relative_ref);
            } 
            else 
            {
                if (relative_ref.impl_->path.empty()) 
                {
                    target.impl_->path = impl_->path;
                    if (!relative_ref.impl_->query.empty())
                    {
                        target.impl_->CopyQuery(relative_ref);
                    } 
                    else 
                    {
                        target.impl_->CopyQuery(*this);
                    }
                } 
                else 
                {
                    if (relative_ref.impl_->IsPathAbsolute()) 
                    {
                        target.impl_->CopyAndNormalizePath(relative_ref);
                    }
                    else 
                    {
                        target.impl_->CopyPath(*this);
                        if (target.impl_->path.size() > 1) 
                        {
                            target.impl_->path.pop_back();
                        }
                        std::copy(relative_ref.impl_->path.begin(),
                                  relative_ref.impl_->path.end(),
                                  std::back_inserter(target.impl_->path));
                        target.NormalizePath();
                    }
                    target.impl_->CopyQuery(relative_ref);
                }
                target.impl_->CopyAuthority(*this);
            }
            target.impl_->CopyScheme(*this);
        }
        target.impl_->CopyFragment(relative_ref);
        return target;
    }

    void Uri::SetScheme(const std::string& scheme)
    {
        impl_->scheme = scheme;
    }

    void Uri::SetUserInfo(const std::string& user_info)
    {
        impl_->user_info = user_info;
    }

    void Uri::SetHost(const std::string& host)
    {
        impl_->host = host;
    }

    void Uri::SetPort(uint16_t port)
    {
        impl_->port = port;
        impl_->has_port = true;
    }

    void Uri::SetQuery(const std::string& query)
    {
        impl_->query = query;
        impl_->has_query = true;
    }
    void Uri::SetPath(const std::vector<std::string>& path)
    {
        impl_->path = path;
    }
    void Uri::SetFragment(const std::string& fragment)
    {
        impl_->fragment = fragment;
        impl_->has_fragment = true;
    }
    
    void Uri::ClearQuery()
    {
        impl_->has_query = false;
    }
    void Uri::ClearPort()
    {
        impl_->has_port = false;
    }
    void Uri::ClearFragment()
    {
        impl_->has_fragment = false;
    }

    std::string Uri::GenerateString() const
    {
        std::ostringstream buffer;
        if (!impl_->scheme.empty()) 
        {
            buffer << impl_->scheme << ':';
        }
        if (impl_->HasAuthority())
        {
            buffer << "//";
            if (!impl_->user_info.empty()) 
            {
                buffer << EncodeElement(impl_->user_info, USER_INFO_NOT_PCT_ENCODED) << '@';
            }
            if (!impl_->host.empty()) 
            {
                if (ValidateIpv6Address(impl_->host)) 
                {
                    buffer << '[' << ToLower(impl_->host) << ']';
                } 
                else 
                {
                    buffer << EncodeElement(impl_->host, REG_NAME_NOT_PCT_ENCODED);
                }
            }
            if (impl_->has_port) 
            {
                buffer << ':' << impl_->port;
            }
        }
        if (impl_->IsPathAbsolute() && (impl_->path.size() == 1)) 
        {
            buffer << '/';
        }
        size_t i = 0;
        for (const auto& segment: impl_->path) 
        {
            buffer << EncodeElement(segment, PCHAR_NOT_PCT_ENCODED);
            if (i + 1 < impl_->path.size()) 
            {
                buffer << '/';
            }
            ++i;
        }
        if (impl_->has_query) 
        {
            buffer << '?' << EncodeElement(impl_->query, QUERY_NOT_PCT_ENCODED_WITHOUT_PLUS);
        }
        if (impl_->has_fragment) 
        {
            buffer << '#' << EncodeElement(impl_->fragment, QUERY_OR_FRAGMENT_NOT_PCT_ENCODED);
        }
        return buffer.str();
    }
}
