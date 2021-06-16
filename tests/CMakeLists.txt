set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
enable_testing(true)

find_package(Qt6 COMPONENTS Test REQUIRED)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false "${CMAKE_CURRENT_LIST_DIR}/**/tst_*.cpp")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    message(STATUS "Adding test: ${TEST_NAME}")
    add_executable(${TEST_NAME} ${TEST_SOURCE} "${CMAKE_CURRENT_LIST_DIR}/TestCommon.hpp")
    target_link_libraries(${TEST_NAME} PRIVATE Qt::Test Qv2ray::Qv2rayBase Qv2ray::QvPluginInterface)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

target_compile_definitions(tst_StaticPlugin PRIVATE -DQT_STATICPLUGIN=1)
