cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.14)

project(Qv2ray-Base
    VERSION 3.0.0.0
    LANGUAGES CXX
    DESCRIPTION "Base library for Qv2ray")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(USE_SYSTEM_QJSONSTRUCT ON)
option(USE_SYSTEM_PLUGIN_INTERFACE ON)

if(USE_SYSTEM_QJSONSTRUCT)
    find_package(QJsonStruct REQUIRED)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdparty/QJsonStruct)
    install(TARGETS QJsonStruct)
endif()

if(USE_SYSTEM_PLUGIN_INTERFACE)
    find_package(QvPluginInterface REQUIRED)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/3rdparty/QvPluginInterface)
    install(TARGETS QvPluginInterface)
endif()

option(USE_MINGW "Use MinGW on Windows" OFF)

if(WIN32)
    include(cmake/versioninfo/generate_product_version.cmake)
    generate_product_version(
        QV2RAY_RC
        NAME               "Qv2ray Base Library"
        BUNDLE             "Qv2ray Project Family"
        ICON               "${CMAKE_CURRENT_LIST_DIR}/assets/qv2ray.ico"
        VERSION_MAJOR      ${CMAKE_PROJECT_VERSION_MAJOR}
        VERSION_MINOR      ${CMAKE_PROJECT_VERSION_MINOR}
        VERSION_PATCH      ${CMAKE_PROJECT_VERSION_PATCH}
        VERSION_REVISION   ${CMAKE_PROJECT_VERSION_TWEAK}
        COMPANY_NAME       "Qv2ray Workgroup"
        COMPANY_COPYRIGHT  "Qv2ray Workgroup 2021"
        FILE_DESCRIPTION   "Qv2ray Base Library"
        )
    add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)
    if(USE_MINGW)
        if(NOT DEFINED MinGW_ROOT)
            set(MinGW_ROOT "C:/msys64/mingw64")
        endif()
    else()
        add_compile_options("/utf-8")
        add_compile_options("/std:c++17")
        add_definitions(-D_WIN32_WINNT=0x600 -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
    endif()
endif()

option(BUILD_SHARED_LIBS "Build Qv2ray baselib as shared library" ON)

find_package(Qt6 COMPONENTS Core Network REQUIRED)

set(BASELIB_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/core/Qv2rayBaseApplication.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/utils/HTTPRequestHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/HTTPRequestHelper.hpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/QvHelpers.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/plugins/PluginAPIHost.cpp
    ${CMAKE_CURRENT_LIST_DIR}/plugins/PluginManagerCore.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/ConnectionIO.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/generation/inbounds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/generation/outbounds.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/generation/filters.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/generation/routing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/generation/misc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/Serialization.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/serialization/ss.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/serialization/ssd.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/serialization/vmess.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/serialization/vmess_new.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/connection/serialization/vless.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/core/CoreUtils.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/core/handler/ConfigHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/handler/KernelInstanceHandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/handler/RouteHandler.cpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/core/kernel/APIBackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/kernel/APIBackend.hpp
    ${CMAKE_CURRENT_LIST_DIR}/core/kernel/V2RayKernelInteractions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/kernel/V2RayKernelInteractions.hpp
    #
    ${CMAKE_CURRENT_LIST_DIR}/core/settings/SettingsBackend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/settings/SettingsBackend.hpp
    ${CMAKE_CURRENT_LIST_DIR}/core/settings/SettingsUpgrade.cpp)

set(BASELIB_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/ConfigHandler.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/ConnectionIO.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/CoreUtils.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Generation.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/KernelInstanceHandler.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayBase.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayBaseApplication.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayFeatures.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayLog.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/QvHelpers.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/PluginAPIHost.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/PluginManagerCore.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/RouteHandler.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Serialization.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/models/CoreObjectModels.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/models/QvComplexConfigModels.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/models/QvCoreSettings.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/models/QvRuntimeConfig.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/models/QvSafeType.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/models/QvSettingsObject.hpp)

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf REQUIRED)

set(API_PROTO "${CMAKE_CURRENT_LIST_DIR}/assets/v2ray_api.proto")
set(API_PROTO_PATH "${CMAKE_CURRENT_LIST_DIR}/assets")
set(API_PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/v2ray_api.pb.cc")
set(API_PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/v2ray_api.pb.h")
set(API_GRPC_SRCS "${CMAKE_CURRENT_BINARY_DIR}/v2ray_api.grpc.pb.cc")
set(API_GRPC_HDRS "${CMAKE_CURRENT_BINARY_DIR}/v2ray_api.grpc.pb.h")

add_custom_command(
    OUTPUT
        "${API_GRPC_SRCS}"
        "${API_GRPC_HDRS}"
        "${API_PROTO_HDRS}"
        "${API_PROTO_SRCS}"
    COMMAND
        ${Protobuf_PROTOC_EXECUTABLE}
    ARGS
        --grpc_out  "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out   "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${API_PROTO_PATH}"
        --plugin=protoc-gen-grpc="${GRPC_CPP_PLUGIN}"
        "${API_PROTO}"
    DEPENDS
        "${API_PROTO}"
    )

add_library(Qv2rayBase ${BASELIB_SOURCES} ${BASELIB_HEADERS} ${API_GRPC_SRCS} ${API_GRPC_HDRS} ${API_PROTO_HDRS} ${API_PROTO_SRCS})
add_library(Qv2ray::Qv2rayBase ALIAS Qv2rayBase)

include(GenerateExportHeader)
generate_export_header(Qv2rayBase)

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/Qv2rayBase.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/Qv2rayBase)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfigVersion.cmake
  COMPATIBILITY SameMinorVersion)

target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_TWEAK=${CMAKE_PROJECT_VERSION_TWEAK})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION="${CMAKE_PROJECT_VERSION}")

set_target_properties(Qv2rayBase PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(Qv2rayBase PROPERTIES CXX_VISIBILITY_PRESET hidden)

target_include_directories(Qv2rayBase
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Qv2rayBase>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Qv2rayBase>"
)

target_include_directories(Qv2rayBase PRIVATE ${QVPLUGIN_INTERFACE_INCLUDE_PATH})

target_link_libraries(Qv2rayBase
    PUBLIC
        Qt::Core
        Qt::Network
        gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts
        protobuf::libprotobuf
        Qv2ray::QJsonStruct
        Qv2ray::QvPluginInterface)

include(GNUInstallDirs)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Qv2rayBase)


install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS Qv2rayBase
    EXPORT Qv2rayBaseTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT Qv2rayBaseTargets
        FILE Qv2rayBaseTargets.cmake
        NAMESPACE Qv2ray::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Qv2rayBase
)
export(EXPORT Qv2rayBaseTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/Qv2rayBaseTargets.cmake"
       NAMESPACE Qv2ray::
)
