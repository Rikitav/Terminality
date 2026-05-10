add_library(terminality STATIC)
target_include_directories(terminality PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE MODULES ./*.ixx)
file(GLOB_RECURSE SOURCES ./*.cpp)

target_sources(terminality
        PRIVATE
        ${SOURCES}
)

target_sources(terminality
        PUBLIC
        FILE_SET CXX_MODULES FILES
        ${MODULES}
)

if (WIN32)
  target_sources(terminality PRIVATE
          src/PlatformSpecific/HostApplication.Windows.cpp
  )
elseif (UNIX)
  target_sources(terminality PRIVATE
          src/PlatformSpecific/HostApplication.Linux.cpp
  )
endif()