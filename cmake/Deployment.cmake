generate_export_header(Qv2rayBase
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBase_export.h)

configure_package_config_file(cmake/Qv2rayBase.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfig.cmake
  INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/Qv2rayBase)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfigVersion.cmake
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBase_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Qv2rayBase/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/Qv2rayBaseConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Qv2rayBase)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS Qv2rayBase
    EXPORT Qv2rayBaseTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

export(EXPORT Qv2rayBaseTargets
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/Qv2rayBaseTargets.cmake"
       NAMESPACE Qv2ray::
)

install(EXPORT Qv2rayBaseTargets
        FILE Qv2rayBaseTargets.cmake
        NAMESPACE Qv2ray::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Qv2rayBase
)
