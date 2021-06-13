project(Qv2rayBase-BuiltinStaticPlugins)

set(CMAKE_AUTOMOC ON)

add_library(BuiltinLatencyTesterPlugin STATIC
    ${CMAKE_CURRENT_LIST_DIR}/BuiltinLatencyTesterPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/BuiltinLatencyTesterPlugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/DNSBase.hpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/TCPLatencyTester.hpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/TCPLatencyTester.cpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/ICMPLatencyTester.hpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/ICMPLatencyTester.cpp)

target_compile_definitions(BuiltinLatencyTesterPlugin PRIVATE QT_STATICPLUGIN)
target_link_libraries(BuiltinLatencyTesterPlugin
    PRIVATE
        Qt::Core
        Qv2ray::libuvw
        Qv2ray::QvPluginInterface)

add_library(Qv2rayBase::StaticLatencyTestPlugin ALIAS BuiltinLatencyTesterPlugin)
