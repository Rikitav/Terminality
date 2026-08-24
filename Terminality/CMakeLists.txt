add_library(terminality STATIC)
target_include_directories(terminality PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE HEADERS ./*.hpp)
file(GLOB_RECURSE SOURCES ./*.cpp)

# Exclude platform-specific backends from the generic glob; they are added per-platform below.
list(FILTER SOURCES EXCLUDE REGEX "PlatformSpecific/HostApplication\\.(Windows|Linux)\\.cpp$")

target_sources(terminality PUBLIC ${HEADERS})
target_sources(terminality PRIVATE ${SOURCES})

if (WIN32)
  target_sources(terminality PRIVATE
          src/Framework/PlatformSpecific/HostApplication.Windows.cpp
  )
elseif (UNIX)
  target_sources(terminality PRIVATE
          src/Framework/PlatformSpecific/HostApplication.Linux.cpp
  )
endif()
