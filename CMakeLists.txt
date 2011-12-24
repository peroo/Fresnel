project(Fresnel)
cmake_minimum_required(VERSION 2.6)

# check libs and packages (headers + lib)

# std C libraries
#find_package(stdlibs REQUIRED)

# libpthread with pthread.h
find_package(Threads REQUIRED)
set(LIBS ${LIBS} ${Threads_LIBRARIES})

set(BOOST_LIBS thread-mt system)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
set(LIBS ${LIBS} ${Boost_LIBRARIES})

find_package(PNG REQUIRED)
set(LIBS ${LIBS} ${PNG_LIBRARIES})

find_package(JPEG REQUIRED)
set(LIBS ${LIBS} ${JPEG_LIBRARIES})

find_library(SQLITE sqlite3)
set(LIBS ${LIBS} ${SQLITE})

find_library(VORBIS vorbis)
find_library(VORBISENC vorbisenc)
set(LIBS ${LIBS} ${VORBIS} ${VORBISENC})

find_library(OGG ogg)
set(LIBS ${LIBS} ${OGG})

find_library(TAGLIB tag)
set(LIBS ${LIBS} ${TAGLIB})

find_library(MPG123 mpg123)
set(LIBS ${LIBS} ${MPG123})

find_library(FLAC FLAC)
find_library(FLAC++ FLAC++)
set(LIBS ${LIBS} ${FLAC} ${FLAC++})

find_library(MHD microhttpd)
set(LIBS ${LIBS} ${MHD})

find_library(XSPF xspf)
set(LIBS ${LIBS} ${XSPF})

FIND_library(MINIUPNPC miniupnpc)
set(LIBS ${LIBS} ${LIBS} ${MINIUPNPC})

# additional compiler flags
add_definitions(-Wall -Wextra -pedantic -Werror -std=c++0x -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE)


# 'other' documentation files
#set(DOC_FILES AUTHORS NEWS README)
#set(DOC_PATH "share/doc/testprogram-0.1")
#install(FILES ${DOC_FILES}
#        DESTINATION ${DOC_PATH})

add_subdirectory(daemon)
