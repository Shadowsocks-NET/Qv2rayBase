set(CMAKE_AUTOMOC ON)

add_library(BuiltinV2RayCorePlugin MODULE
    ${CMAKE_CURRENT_LIST_DIR}/BuiltinV2RayCorePlugin.hpp
    ${CMAKE_CURRENT_LIST_DIR}/BuiltinV2RayCorePlugin.cpp)

target_compile_definitions(BuiltinV2RayCorePlugin PRIVATE QT_STATICPLUGIN)
target_link_libraries(BuiltinV2RayCorePlugin
    PRIVATE
        Qt::Core
        Qv2ray::QvPluginInterface)

qv2ray_configure_plugin(BuiltinV2RayCorePlugin)
