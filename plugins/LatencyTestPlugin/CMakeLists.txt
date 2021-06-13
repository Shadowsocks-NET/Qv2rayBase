set(CMAKE_AUTOMOC ON)

add_library(BuiltinLatencyTestPlugin MODULE
    ${CMAKE_CURRENT_LIST_DIR}/BuiltinLatencyTesterPlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/BuiltinLatencyTesterPlugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/DNSBase.hpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/TCPLatencyTester.hpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/TCPLatencyTester.cpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/ICMPLatencyTester.hpp
    ${CMAKE_CURRENT_LIST_DIR}/latency/ICMPLatencyTester.cpp)

target_compile_definitions(BuiltinLatencyTestPlugin PRIVATE QT_STATICPLUGIN)
target_link_libraries(BuiltinLatencyTestPlugin
    PRIVATE
        Qt::Core
        Qv2ray::libuvw
        Qv2ray::QvPluginInterface)

qv2ray_configure_plugin(BuiltinLatencyTestPlugin)
