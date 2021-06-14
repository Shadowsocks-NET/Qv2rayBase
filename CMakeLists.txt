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

if(WIN32)
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/Windows.cmake)
endif()

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(USE_SYSTEM_PLUGIN_INTERFACE OFF)

if(USE_SYSTEM_PLUGIN_INTERFACE)
    find_package(QvPluginInterface REQUIRED)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/PluginInterface)
    install(TARGETS QvPluginInterface)
endif()

option(BUILD_SHARED_LIBS "Build Qv2ray baselib as shared library" ON)

find_package(Qt6 COMPONENTS Core Network REQUIRED)

set(BASELIB_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/Qv2rayBaseLibrary.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Base/SettingsUpgrade.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Plugin/LatencyTestHost.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Plugin/PluginAPIHost.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Plugin/PluginManagerCore.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/Generator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/KernelManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/ProfileManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Profile/Serialization.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/Utils/HTTPRequestHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Utils/HTTPRequestHelper.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Utils/ProfileHelpers.cpp)

set(BASELIB_PRIVATE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/src/private/BaseStorageProvider.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/private/BaseStorageProvider.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/private/BaseConfigurationGenerator.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/private/BaseConfigurationGenerator.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/private/LatencyTestThread.hpp
    ${CMAKE_CURRENT_LIST_DIR}/src/private/LatencyTestThread.cpp)

set(BASELIB_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/StorageProvider.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayBaseLibrary.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Qv2rayBaseFeatures.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Models/Settings.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Plugin/LatencyTestHost.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Plugin/PluginAPIHost.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Plugin/PluginManagerCore.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/Generator.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/KernelManager.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/ProfileManager.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Profile/Serialization.hpp

    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Common/Utils.hpp
    ${CMAKE_CURRENT_LIST_DIR}/include/Qv2rayBase/Common/ProfileHelpers.hpp)

source_group(Plugin REGULAR_EXPRESSION "/Plugin/")
source_group(Profile REGULAR_EXPRESSION "/Profile/")
source_group(Common REGULAR_EXPRESSION "/Common/")

add_library(Qv2rayBase ${QV2RAY_RC} ${BASELIB_SOURCES} ${BASELIB_HEADERS} ${BASELIB_PRIVATE_SOURCES})
add_library(Qv2ray::Qv2rayBase ALIAS Qv2rayBase)

target_include_directories(Qv2rayBase
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Qv2rayBase>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Qv2rayBase>"
)

target_include_directories(Qv2rayBase PRIVATE ${QVPLUGIN_INTERFACE_INCLUDE_PATH})

target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_MINOR=${PROJECT_VERSION_MINOR})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_PATCH=${PROJECT_VERSION_PATCH})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION_TWEAK=${PROJECT_VERSION_TWEAK})
target_compile_definitions(Qv2rayBase PUBLIC -DQV2RAY_BASELIB_VERSION="${PROJECT_VERSION}")

set_target_properties(Qv2rayBase PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 0)
set_target_properties(Qv2rayBase PROPERTIES CXX_VISIBILITY_PRESET hidden)

if(CMAKE_CXX_COMPILER_ID EQUAL Clang OR CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  if(UNIX AND NOT APPLE)
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,-z,defs")
  endif()
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Deployment.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/cmake/libuvw.cmake)

target_link_libraries(Qv2rayBase
    PUBLIC
        Qt::Core
        Qt::Network
        Qv2ray::QvPluginInterface
    PRIVATE
        Qv2ray::libuvw)
