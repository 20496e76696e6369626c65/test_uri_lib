#include <gtest/gtest.h>
#include <Uri/Uri.hpp>

TEST(UriTests, ParseFromStringNoScheme)
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("foo/bar"));
    ASSERT_EQ("", uri.GetScheme());
    ASSERT_EQ((std::vector< std::string >{"foo","bar",}),uri.GetPath());
}


TEST(UriTests, ParseFromStringUrl)
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_EQ("http", uri.GetScheme());
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_EQ((std::vector< std::string> {"","foo", "bar",}), uri.GetPath());
}

TEST(UriTests, ParseFromStringPathCornerCases) {
    struct TestVector {
        std::string path_in;
        std::vector< std::string > path_out;
    };
    const std::vector< TestVector > testVectors{
        {"", {}},
        {"/", {""}},
        {"/foo", {"", "foo"} },
        {"foo/", {"foo", ""} },
    };
    size_t index = 0;
    for (const auto& testVector : testVectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(testVector.path_in)) << index;
        ASSERT_EQ(testVector.path_out, uri.GetPath()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringUrnDefaultPathDelimiter) {
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("urn:test:path:Delimiter"));
    ASSERT_EQ("urn", uri.GetScheme());
    ASSERT_EQ("", uri.GetHost());
    ASSERT_EQ((std::vector< std::string >{"test:path:Delimiter",}),uri.GetPath());
}

TEST(UriTests, ParseFromStringHasAPortNumber)
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:8080/foo/bar"));
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_TRUE(uri.HasPort());
    ASSERT_EQ(8080, uri.GetPort());
}

TEST(UriTests, ParseFromStringDoesNotHaveAPortNumber)
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_EQ("www.example.com", uri.GetHost());
    ASSERT_FALSE(uri.HasPort());
}

TEST(UriTests, ParseFromStringTwiceFirstWithPortNumberThenWithout)
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com:8080/foo/bar"));
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com/foo/bar"));
    ASSERT_FALSE(uri.HasPort());
}


TEST(UriTests, ParseFromStringBadPortNumberPureAlphabetic)
{
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:badtest/foo/bar"));
}


TEST(UriTests, ParseFromStringBadPortNumberStartsNumeric)
{
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:8080badtest/foo/bar"));
}


TEST(UriTests, ParseFromStringBadPortNumberIsBig)
{
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:808080/foo/bar"));
}


TEST(UriTests, ParseFromStringBadPortNegativ)
{
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:-8080/foo/bar"));
}

TEST(UriTests, ParseFromStringBadPortNumberEndNumeric)
{
    Uri::Uri uri;
    ASSERT_FALSE(uri.ParseFromString("http://www.example.com:badtest8080/foo/bar"));
}


TEST(UriTests, ParseFromStringEndsAfterAuthority) 
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://www.example.com"));
}

TEST(UriTests, ParseFromStringRelativeVsNoneRelativeReference)
{
    struct TestVector
    {
        std::string uri_string;
        bool is_relative_reference;
    };

    const std::vector<TestVector> test_vectors
    {
        {"http://www.example.com/", false},
        {"http://www.example.com", false},
        {"/", true},
        {"foo", true},
    };
    size_t index = 0;
    for(const auto& test_vector : test_vectors)
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector.uri_string)) << index;
        ASSERT_EQ(test_vector.is_relative_reference, uri.IsRelativeReference()) << index;
        ++index;
    }

}



TEST(UriTests, ParseFromStringUserInfo) 
{
    struct TestVector
    {
        std::string uri_string;
        std::string user_info;
    };

    const std::vector< TestVector > test_vectors
    {
        {"http://www.example.com/", ""},
        {"http://joe@www.example.com", "joe"},
        {"http://pepe:feelsbadman@www.example.com", "pepe:feelsbadman"},
        {"//www.example.com", ""},
        {"//bob@www.example.com", "bob"},
        {"/", ""},
        {"foo", ""},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors)
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector.uri_string)) << index;
        ASSERT_EQ(test_vector.user_info, uri.GetUserInfo()) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringTwiceFirstUserInfoThenWithout) 
{
    Uri::Uri uri;
    ASSERT_TRUE(uri.ParseFromString("http://joe@www.example.com/foo/bar"));
    ASSERT_TRUE(uri.ParseFromString("/foo/bar"));
    ASSERT_TRUE(uri.GetUserInfo().empty());
}

TEST(UriTests, ParseFromStringSchemeIllegalCharacters) 
{
    const std::vector< std::string > test_vectors
    {
        {"://www.example.com/"},
        {"0://www.example.com/"},
        {"+://www.example.com/"},
        {"@://www.example.com/"},
        {".://www.example.com/"},
        {"h@://www.example.com/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(test_vector)) << index;
        ++index;
    }
}
TEST(UriTests, ParseFromStringSchemeBarelyLegal) 
{
    struct TestVector 
    {
        std::string uri_string;
        std::string scheme;
    };
    const std::vector< TestVector > test_vectors
    {
        {"h://www.example.com/", "h"},
        {"x+://www.example.com/", "x+"},
        {"y-://www.example.com/", "y-"},
        {"z.://www.example.com/", "z."},
        {"aa://www.example.com/", "aa"},
        {"a0://www.example.com/", "a0"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector.uri_string)) << index;
        ASSERT_EQ(test_vector.scheme, uri.GetScheme());
        ++index;
    }
}

TEST(UriTests, ParseFromStringSchemeMixedCase) 
{
    const std::vector< std::string > test_vectors
    {
        {"http://www.example.com/"},
        {"hTtp://www.example.com/"},
        {"HTTP://www.example.com/"},
        {"Http://www.example.com/"},
        {"HttP://www.example.com/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector)) << index;
        ASSERT_EQ("http", uri.GetScheme()) << ">>> Failed for test vector element " << index << " <<<";
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfoIllegalCharacters) 
{
    const std::vector< std::string > test_vectors
    {
        {"//%X@www.example.com/"},
        {"//{@www.example.com/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(test_vector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringUserInfoBarelyLegal) 
{
    struct TestVector 
    {
        std::string uri_string;
        std::string user_info;
    };
    const std::vector< TestVector > test_vectors{
        {"//%41@www.example.com/", "A"},
        {"//@www.example.com/", ""},
        {"//!@www.example.com/", "!"},
        {"//'@www.example.com/", "'"},
        {"//(@www.example.com/", "("},
        {"//;@www.example.com/", ";"},
        {"http://:@www.example.com/", ":"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector.uri_string)) << index;
        ASSERT_EQ(test_vector.user_info, uri.GetUserInfo());
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostIllegalCharacters) 
{
    const std::vector< std::string > test_vectors
    {
        {"//%X@www.example.com/"},
        {"//@www:example.com/"},
        {"//[vX.:]/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(test_vector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringHostMixedCase) {
    const std::vector< std::string > test_vectors
    {
        {"http://www.example.com/"},
        {"http://www.EXAMPLE.com/"},
        {"http://www.exAMple.com/"},
        {"http://www.example.cOM/"},
        {"http://wWw.exampLe.Com/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector)) << index;
        ASSERT_EQ("www.example.com", uri.GetHost()) << ">>> Failed for test vector element " << index << " <<<";
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathIllegalCharacters) {
    const std::vector< std::string > test_vectors
    {
        {"http://www.example.com/foo[bar"},
        {"http://www.example.com/]bar"},
        {"http://www.example.com/foo]"},
        {"http://www.example.com/["},
        {"http://www.example.com/abc/foo]"},
        {"http://www.example.com/abc/["},
        {"http://www.example.com/foo]/abc"},
        {"http://www.example.com/[/abc"},
        {"http://www.example.com/foo]/"},
        {"http://www.example.com/[/"},
        {"/foo[bar"},
        {"/]bar"},
        {"/foo]"},
        {"/["},
        {"/abc/foo]"},
        {"/abc/["},
        {"/foo]/abc"},
        {"/[/abc"},
        {"/foo]/"},
        {"/[/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(test_vector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringPathBarelyLegal) 
{
    struct TestVector 
    {
        std::string uri_string;
        std::vector< std::string > path;
    };
    const std::vector< TestVector > test_vectors
    {
        {"/:/foo", {"", ":", "foo"}},
        {"bob@/foo", {"bob@", "foo"}},
        {"hello!", {"hello!"}},
        {"urn:hello,%20w%6Frld", {"hello, world"}},
        {"//example.com/foo/(bar)/", {"", "foo", "(bar)", ""}},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector.uri_string)) << index;
        ASSERT_EQ(test_vector.path, uri.GetPath());
        ++index;
    }
}

TEST(UriTests, ParseFromStringQueryIllegalCharacters) 
{
    const std::vector< std::string > test_vectors
    {
        {"http://www.example.com/?foo[bar"},
        {"http://www.example.com/?]bar"},
        {"http://www.example.com/?foo]"},
        {"http://www.example.com/?["},
        {"http://www.example.com/?abc/foo]"},
        {"http://www.example.com/?abc/["},
        {"http://www.example.com/?foo]/abc"},
        {"http://www.example.com/?[/abc"},
        {"http://www.example.com/?foo]/"},
        {"http://www.example.com/?[/"},
        {"?foo[bar"},
        {"?]bar"},
        {"?foo]"},
        {"?["},
        {"?abc/foo]"},
        {"?abc/["},
        {"?foo]/abc"},
        {"?[/abc"},
        {"?foo]/"},
        {"?[/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(test_vector)) << index;
        ++index;
    }
}

TEST(UriTests, ParseFromStringFragmentIllegalCharacters) 
{
    const std::vector< std::string > test_vectors
    {
        {"http://www.example.com/#foo[bar"},
        {"http://www.example.com/#]bar"},
        {"http://www.example.com/#foo]"},
        {"http://www.example.com/#["},
        {"http://www.example.com/#abc/foo]"},
        {"http://www.example.com/#abc/["},
        {"http://www.example.com/#foo]/abc"},
        {"http://www.example.com/#[/abc"},
        {"http://www.example.com/#foo]/"},
        {"http://www.example.com/#[/"},
        {"#foo[bar"},
        {"#]bar"},
        {"#foo]"},
        {"#["},
        {"#abc/foo]"},
        {"#abc/["},
        {"#foo]/abc"},
        {"#[/abc"},
        {"#foo]/"},
        {"#[/"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors) 
    {
        Uri::Uri uri;
        ASSERT_FALSE(uri.ParseFromString(test_vector)) << index;
        ++index;
    }
}


TEST(UriTests, ParseFromStringPathsWithPercentEncodedCharacters) 
{
    struct TestVector 
    {
        std::string uri_string;
        std::string path_first_segment;
    };
    const std::vector< TestVector > test_vectors
    {
        {"%41", "A"},
        {"%4A", "J"},
        {"%4a", "J"},
        {"%bc", "\xbc"},
        {"%Bc", "\xbc"},
        {"%bC", "\xbc"},
        {"%BC", "\xbc"},
        {"%41%42%43", "ABC"},
        {"%41%4A%43%4b", "AJCK"},
    };
    size_t index = 0;
    for (const auto& test_vector : test_vectors)
    {
        Uri::Uri uri;
        ASSERT_TRUE(uri.ParseFromString(test_vector.uri_string)) << index;
        ASSERT_EQ(test_vector.path_first_segment, uri.GetPath()[0]);
        ++index;
    }
}

