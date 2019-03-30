cmake_minimum_required(VERSION 3.8)

set(This Uri)

set(Headers
    include/Uri/Uri.hpp
    src/PercentEncodedCharacterDecoder.hpp
    src/CharacterSet.hpp
)

set(Sources
    src/Uri.cpp
    src/PercentEncodedCharacterDecoder.cpp
    src/CharacterSet.cpp
)

add_library(${This} STATIC ${Sources} ${Headers})
set_target_properties(${This} PROPERTIES
    FOLDER Libraries
)

target_include_directories(${This} PUBLIC include)

add_subdirectory(test)