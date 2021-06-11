cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.14)
project(Qv2ray-Base
    VERSION 3.0.0.0
    LANGUAGES CXX
    DESCRIPTION "Base library for Qv2ray")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_DEBUG_POSTFIX d)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(USE_SYSTEM_QJSONSTRUCT OFF)
set(USE_SYSTEM_PLUGIN_INTERFACE OFF)

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

macro(Qv2rayBase_Features feat)
    # WARNING TODO Change to configure_file
    # option(QV2RAY_FEATURE_${feat} ON)
    if(QV2RAY_FEATURE_${feat})
        add_compile_definitions(QVFEATURE_${feat}=1)
    else()
        add_compile_definitions(QVFEATURE_${feat}=-1)
    endif()
endmacro()

Qv2rayBase_Features(subscriptions)
Qv2rayBase_Features(latency)
Qv2rayBase_Features(statistics)

set(BASELIB_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/Qv2rayBaseLibrary.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Base/SettingsUpgrade.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Plugin/PluginAPIHost.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Plugin/PluginManagerCore.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/Generator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/KernelManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/ProfileManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/Serialization.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Utils/HTTPRequestHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Utils/HTTPRequestHelper.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Utils/ProfileHelpers.cpp)

set(BASELIB_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/StorageProvider.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayBaseLibrary.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayBaseFeatures.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Models/Settings.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Plugin/PluginAPIHost.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Plugin/PluginManagerCore.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/Generator.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/KernelManager.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/ProfileManager.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/Serialization.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Common/Utils.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Common/ProfileHelper.hpp)

source_group(Plugin REGULAR_EXPRESSION "/Plugin/")
source_group(Profile REGULAR_EXPRESSION "/Profile/")
source_group(Common REGULAR_EXPRESSION "/Common/")

add_library(Qv2rayBase ${BASELIB_SOURCES} ${BASELIB_HEADERS} #[[${API_GRPC_SRCS} ${API_GRPC_HDRS} ${API_PROTO_HDRS} ${API_PROTO_SRCS}]])
add_library(Qv2ray::Qv2rayBase ALIAS Qv2rayBase)

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
        Qv2ray::QJsonStruct
        Qv2ray::QvPluginInterface)

target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_TWEAK=${CMAKE_PROJECT_VERSION_TWEAK})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION="${CMAKE_PROJECT_VERSION}")

set_target_properties(Qv2rayBase PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})
set_target_properties(Qv2rayBase PROPERTIES CXX_VISIBILITY_PRESET hidden)

generate_export_header(Qv2rayBase)

configure_package_config_file(cmake/Qv2rayBase.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/Qv2rayBase)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfigVersion.cmake
  COMPATIBILITY SameMajorVersion)

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
