#ifndef URI_HPP
#define URI_HPP

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

namespace Uri
{
    class Uri
    {
    public:
        ~Uri() noexcept;
        Uri(Uri&&)noexcept;
        Uri(const Uri&);
        Uri& operator=(Uri&&) noexcept;
        Uri& operator=(const Uri&);
        bool operator==(const Uri&) const;
        bool operator!=(const Uri&) const;

    public:
        Uri();
        Uri Resolve (const Uri&) const;
        bool HasPort() const;
        bool HasQuery() const;
        bool HasFragment() const;
        bool IsRelativeReference() const;
        bool ContainsRelativePath() const;
        bool ParseFromString(const std::string&);

        uint16_t GetPort() const;
        void ClearPort();
        void ClearQuery();
        void ClearFragment();
        void NormalizePath();

        void SetScheme(const std::string&);
        void SetPort(uint16_t);
        void SetUserInfo(const std::string&);
        void SetFragment(const std::string&);
        void SetPath(const std::vector<std::string>&);
        void SetHost(const std::string&);
        void SetQuery(const std::string&);


        std::string GetUserInfo() const;     
        std::string GetScheme() const;
        std::string GetHost() const;
        std::string GetFragment() const;
        std::string GetQuery() const;
        std::string GenerateString() const;
        std::vector<std::string> GetPath() const;
        
    private:
        struct Impl;
        std::unique_ptr< struct Impl> impl_;
    };

} // Uri

#endif
