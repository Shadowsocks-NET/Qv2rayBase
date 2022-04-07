option(USE_SYSTEM_LIBUV "Use system libuv" OFF)
option(USE_SYSTEM_UVW "Use system libuvw" OFF)

if(USE_SYSTEM_UVW)
    set(USE_SYSTEM_LIBUV ON)
else()
    set(BUILD_UVW_LIBS ON)
endif()

if(USE_SYSTEM_LIBUV)
    # Special package name from vcpkg
    find_package(unofficial-libuv CONFIG)
    if(${unofficial-libuv_FOUND})
        add_library(Qv2ray::libuv ALIAS unofficial::libuv::libuv)
    else()
        find_package(LibUV REQUIRED)
        add_library(Qv2ray::libuv ALIAS LibUV::LibUV)
    endif()
endif()

if(USE_SYSTEM_UVW)
    find_package(uvw CONFIG REQUIRED)
    add_library(Qv2ray::libuvw ALIAS uvw::uvw)
else()
    add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/uvw)
    add_library(Qv2ray::libuvw ALIAS uvw)

    # BEGIN - the hack to install libuvw as static libraries
    if(NOT BUILD_SHARED_LIBS)
        # See https://github.com/skypjack/uvw/pull/264
        export(EXPORT uvwConfig)
    endif()
    # END  - the hack to install libuvw as static libraries
endif()
