set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/Audio
    ${CMAKE_CURRENT_SOURCE_DIR}/Image
)

set(HDRS
    Database.h
    DBQuery.h
    HttpRequest.h
    Indexer.h
    ResFile.h
    Resource.h
    Fresnel.h
    SQLite.h
    Audio/AudioDecoder.h
    Audio/AudioEncoder.h
    Audio/Audio.h
    Audio/FLACDecoder.h
    Audio/Metadata.h
    Audio/MP3Decoder.h
    Audio/VorbisEncoder.h
    Image/Image.h
)

set(SRC
    Database.cpp
    DBQuery.cpp
    HttpRequest.cpp
    Indexer.cpp
    ResFile.cpp
    Resource.cpp
    Fresnel.cpp
    SQLite.cpp
    Audio/AudioDecoder.cpp
    Audio/AudioEncoder.cpp
    Audio/Audio.cpp
    Audio/FLACDecoder.cpp
    Audio/Metadata.cpp
    Audio/MP3Decoder.cpp
    Audio/VorbisEncoder.cpp
    Image/Image.cpp
)

add_executable(fresnel ${SRC})
target_link_libraries(fresnel ${LIBS})
    
