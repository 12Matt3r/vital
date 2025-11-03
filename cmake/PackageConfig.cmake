# Package Configuration for Distribution
# Handles creating installable packages for different platforms

# =============================================================================
# PACKAGING OPTIONS
# =============================================================================

# Package formats
option(BUILD_DEB_PACKAGE "Create Debian package" OFF)
option(BUILD_RPM_PACKAGE "Create RPM package" OFF)
option(BUILD_TGZ_PACKAGE "Create tar.gz package" ON)
option(BUILD_ZIP_PACKAGE "Create ZIP package" OFF)
option(BUILD_DMG_PACKAGE "Create macOS DMG package" OFF)
option(BUILD_MSI_PACKAGE "Create Windows MSI installer" OFF)
option(BUILD_NSIS_PACKAGE "Create Windows NSIS installer" OFF)
option(BUILD_APP_IMAGE "Create Linux AppImage" OFF)

# Package metadata
set(VITAL_PACKAGE_NAME "vital-synthesizer" CACHE STRING "Package name")
set(VITAL_PACKAGE_DESCRIPTION "Powerful, Beautiful, Free, Open-Source Synthesizer" CACHE STRING "Package description")
set(VITAL_PACKAGE_VENDOR "The Vital Team" CACHE STRING "Package vendor")
set(VITAL_PACKAGE_HOMEPAGE "https://github.com/Matt-McDaid/vital" CACHE STRING "Package homepage")
set(VITAL_PACKAGE_LICENSE "GPL-3.0" CACHE STRING "Package license")

# Version information
set(VITAL_PACKAGE_VERSION ${PROJECT_VERSION})
set(VITAL_PACKAGE_RELEASE "1" CACHE STRING "Package release number")

# =============================================================================
# CPACK CONFIGURATION
# =============================================================================

if(BUILD_TGZ_PACKAGE OR BUILD_ZIP_PACKAGE OR BUILD_DEB_PACKAGE OR BUILD_RPM_PACKAGE OR BUILD_DMG_PACKAGE OR BUILD_MSI_PACKAGE OR BUILD_NSIS_PACKAGE OR BUILD_APP_IMAGE)
  message(STATUS "=== Package Configuration ===")
  
  # Include CPack
  include(CPack)
  
  # General CPack settings
  set(CPACK_PACKAGE_NAME ${VITAL_PACKAGE_NAME})
  set(CPACK_PACKAGE_VERSION ${VITAL_PACKAGE_VERSION})
  set(CPACK_PACKAGE_DESCRIPTION ${VITAL_PACKAGE_DESCRIPTION})
  set(CPACK_PACKAGE_VENDOR ${VITAL_PACKAGE_VENDOR})
  set(CPACK_PACKAGE_HOMEPAGE ${VITAL_PACKAGE_HOMEPAGE})
  set(CPACK_PACKAGE_LICENSE ${VITAL_PACKAGE_LICENSE})
  set(CPACK_PACKAGE_CONTACT "https://github.com/Matt-McDaid/vital")
  
  # Universal settings
  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY ${CPACK_PACKAGE_NAME})
  
  # Component definitions
  set(CPACK_COMPONENTS_ALL core standalone plugin examples docs)
  set(CPACK_COMPONENT_CORE_DISPLAY_NAME "Core Library")
  set(CPACK_COMPONENT_CORE_DESCRIPTION "Vital synthesizer core library")
  set(CPACK_COMPONENT_STANDALONE_DISPLAY_NAME "Standalone Application")
  set(CPACK_COMPONENT_STANDALONE_DESCRIPTION "Standalone Vital synthesizer application")
  set(CPACK_COMPONENT_PLUGIN_DISPLAY_NAME "Audio Plugin")
  set(CPACK_COMPONENT_PLUGIN_DESCRIPTION "VST3/AU plugin version")
  set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Example Projects")
  set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example code and demos")
  set(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
  set(CPACK_COMPONENT_DOCS_DESCRIPTION "User and developer documentation")
endif()

# =============================================================================
# TAR.GZ PACKAGE (Universal)
# =============================================================================

if(BUILD_TGZ_PACKAGE)
  message(STATUS "  Creating tar.gz package: YES")
  
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_TGZ_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz")
  
  # Package contents
  set(CPACK_TGZ_INCLUDE_FILES
    ${CMAKE_BINARY_DIR}/include/vital/*.h
    ${CMAKE_BINARY_DIR}/lib/*.a
  )
  
  set(CPACK_TGZ_REMOVE_FILES
    *.txt
    *.md
  )

# =============================================================================
# ZIP PACKAGE (Windows/Linux)
# =============================================================================

elseif(BUILD_ZIP_PACKAGE)
  message(STATUS "  Creating ZIP package: YES")
  
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_ZIP_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.zip")
endif()

# =============================================================================
# DEBIAN PACKAGE (Linux)
# =============================================================================

if(BUILD_DEB_PACKAGE AND VITAL_PLATFORM_LINUX)
  message(STATUS "  Creating Debian package: YES")
  
  set(CPACK_GENERATOR "DEB")
  
  # Debian-specific settings
  set(CPACK_DEB_PACKAGE_NAME ${VITAL_PACKAGE_NAME})
  set(CPACK_DEB_PACKAGE_VERSION ${VITAL_PACKAGE_VERSION})
  set(CPACK_DEB_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
  set(CPACK_DEB_PACKAGE_DEPENDS "libjuce7, libstdc++6, libc6")
  set(CPACK_DEB_PACKAGE_MAINTAINER "The Vital Team <contact@vital.dev>")
  set(CPACK_DEB_PACKAGE_DESCRIPTION ${VITAL_PACKAGE_DESCRIPTION})
  set(CPACK_DEB_PACKAGE_SECTION "sound")
  set(CPACK_DEB_PACKAGE_PRIORITY "optional")
  
  # Control file customization
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/debian/postinst;${CMAKE_BINARY_DIR}/debian/prerm")
  
  # Create postinst script
  file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/debian)
  file(WRITE ${CMAKE_BINARY_DIR}/debian/postinst
    "#!/bin/bash
# Vital Synthesizer post-installation script
set -e

# Update shared library cache
if command -v ldconfig >/dev/null 2>&1; then
    ldconfig
fi

# Register VST3 plugin if building plugin
if [ -d /usr/lib/vst3 ]; then
    mkdir -p /usr/lib/vst3
fi

exit 0
")
  
  # Create prerm script
  file(WRITE ${CMAKE_BINARY_DIR}/debian/prerm
    "#!/bin/bash
# Vital Synthesizer pre-removal script
set -e

# Update shared library cache
if command -v ldconfig >/dev/null 2>&1; then
    ldconfig
fi

exit 0
")
  
  make_directory(${CMAKE_BINARY_DIR}/debian)
endif()

# =============================================================================
# RPM PACKAGE (Linux)
# =============================================================================

if(BUILD_RPM_PACKAGE AND VITAL_PLATFORM_LINUX)
  message(STATUS "  Creating RPM package: YES")
  
  set(CPACK_GENERATOR "RPM")
  
  # RPM-specific settings
  set(CPACK_RPM_PACKAGE_NAME ${VITAL_PACKAGE_NAME})
  set(CPACK_RPM_PACKAGE_VERSION ${VITAL_PACKAGE_VERSION})
  set(CPACK_RPM_PACKAGE_RELEASE ${VITAL_PACKAGE_RELEASE})
  set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
  set(CPACK_RPM_PACKAGE_REQUIRES "libjuce7, libstdc++6, glibc")
  set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
  set(CPACK_RPM_PACKAGE_LICENSE ${VITAL_PACKAGE_LICENSE})
  set(CPACK_RPM_PACKAGE_SUMMARY ${VITAL_PACKAGE_DESCRIPTION})
  set(CPACK_RPM_PACKAGE_URL ${VITAL_PACKAGE_HOMEPAGE})
  
  # RPM requires
  set(CPACK_RPM_PACKAGE_REQUIRES "libjuce7 >= 7.0.0, libstdc++6 >= 4.8.0")
endif()

# =============================================================================
# macOS DMG PACKAGE
# =============================================================================

if(BUILD_DMG_PACKAGE AND VITAL_PLATFORM_MACOS)
  message(STATUS "  Creating macOS DMG package: YES")
  
  set(CPACK_GENERATOR "DragNDrop")
  
  # macOS-specific settings
  set(CPACK_DMG_VOLUME_NAME "Vital Synthesizer")
  set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dmg_setup.scpt")
  set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/dmg_background.png")
  set(CPACK_DMG_WINDOW_WIDTH 540)
  set(CPACK_DMG_WINDOW_HEIGHT 420)
  set(CPACK_DMG_WINDOW_Y 100)
  set(CPACK_DMG_WINDOW_X 130)
  
  # Create DMG setup script
  file(WRITE ${CMAKE_BINARY_DIR}/dmg_setup.scpt
    "tell application \"Finder\"
      tell disk
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {400, 100, 940, 420}
        set position of item \"Vital.app\" of container window to {140, 90}
        set position of item \"VitalVST3.vst3\" of container window to {340, 90}
        set position of item \"VitalAU.component\" of container window to {540, 90}
        set position of item \"Applications\" of container window to {140, 250}
        close
        open
        set current view of container window to list view
        set the bounds of container window to {400, 100, 940, 420}
        close
        open
        set current view of container window to icon view
        delay 1
      end tell
    end tell
  ")
endif()

# =============================================================================
# WINDOWS MSI PACKAGE
# =============================================================================

if(BUILD_MSI_PACKAGE AND VITAL_PLATFORM_WINDOWS)
  message(STATUS "  Creating Windows MSI package: YES")
  
  set(CPACK_GENERATOR "MSI")
  
  # Windows-specific settings
  set(CPACK_MSI_PACKAGE_NAME ${VITAL_PACKAGE_NAME})
  set(CPACK_MSI_PACKAGE_VERSION ${VITAL_PACKAGE_VERSION})
  set(CPACK_MSI_PACKAGE_DESCRIPTION ${VITAL_PACKAGE_DESCRIPTION})
  set(CPACK_MSI_PACKAGE_VENDOR ${VITAL_PACKAGE_VENDOR})
  set(CPACK_MSI_PACKAGE_CONTACT "https://github.com/Matt-McDaid/vital")
  set(CPACK_MSI_PACKAGE_LICENSEE "The Vital Team")
  set(CPACK_MSI_PACKAGE_UPGRADE_GUID "12345678-1234-1234-1234-123456789012")
  
  # MSI features
  set(CPACK_MSI_FEATURE_CORE "Core")
  set(CPACK_MSI_FEATURE_STANDALONE "Standalone")
  set(CPACK_MSI_FEATURE_PLUGIN "Plugin")
  set(CPACK_MSI_FEATURE_DOCS "Documentation")
endif()

# =============================================================================
# WINDOWS NSIS PACKAGE
# =============================================================================

if(BUILD_NSIS_PACKAGE AND VITAL_PLATFORM_WINDOWS)
  message(STATUS "  Creating Windows NSIS package: YES")
  
  set(CPACK_GENERATOR "NSIS")
  
  # NSIS-specific settings
  set(CPACK_NSIS_PACKAGE_NAME ${VITAL_PACKAGE_NAME})
  set(CPACK_NSIS_PACKAGE_VERSION ${VITAL_PACKAGE_VERSION})
  set(CPACK_NSIS_DISPLAY_NAME "Vital Synthesizer")
  set(CPACK_NSIS_PACKAGE_DESCRIPTION ${VITAL_PACKAGE_DESCRIPTION})
  set(CPACK_NSIS_HELP_LINK "https://github.com/Matt-McDaid/vital")
  set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/Matt-McDaid/vital")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/vital_icon.ico")
  set(CPACK_NSIS_MUI_UNICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/vital_icon.ico")
  set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
  
  # NSIS installer options
  set(CPACK_NSIS_INCLUDE_WINDOWSMESSAGE 1)
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
endif()

# =============================================================================
# LINUX APPIMAGE PACKAGE
# =============================================================================

if(BUILD_APP_IMAGE AND VITAL_PLATFORM_LINUX)
  message(STATUS "  Creating Linux AppImage package: YES")
  
  # AppImage requires appimagetool
  find_program(APPIMAGETOOL_EXE appimagetool HINTS /usr/bin /usr/local/bin)
  
  if(APPIMAGETOOL_EXE)
    set(CPACK_GENERATOR "TGZ")
    
    # Create AppDir structure
    set(APPDIR_DIR ${CMAKE_BINARY_DIR}/AppDir)
    set(APPDIR_BIN_DIR ${APPDIR_DIR}/usr/bin)
    set(APPDIR_LIB_DIR ${APPDIR_DIR}/usr/lib)
    set(APPDIR_SHARE_DIR ${APPDIR_DIR}/usr/share)
    
    # Create directories
    file(MAKE_DIRECTORY ${APPDIR_BIN_DIR})
    file(MAKE_DIRECTORY ${APPDIR_LIB_DIR})
    file(MAKE_DIRECTORY ${APPDIR_SHARE_DIR})
    
    # Copy executable and libraries
    if(BUILD_STANDALONE)
      configure_file(${CMAKE_BINARY_DIR}/VitalStandalone ${APPDIR_BIN_DIR}/vital-standalone COPYONLY)
    endif()
    
    # Copy AppRun script
    file(WRITE ${APPDIR_DIR}/AppRun
      "#!/bin/bash
HERE=\"$(dirname \"$(readlink -f \"${0}\")\")\"
export LD_LIBRARY_PATH=\"${HERE}/usr/lib:${LD_LIBRARY_PATH}\"
exec \"${HERE}/usr/bin/vital-standalone\" \"$@\"
")
    
    chmod(${CMAKE_CURRENT_BINARY_DIR}/AppDir/AppRun 755)
    
    # Custom target for AppImage
    add_custom_target(create-appimage
      COMMAND ${APPIMAGETOOL_EXE} ${APPDIR_DIR} ${CMAKE_BINARY_DIR}/${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.AppImage
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Creating Linux AppImage package"
    )
    
  else()
    message(WARNING "appimagetool not found. AppImage creation skipped.")
  endif()
endif()

# =============================================================================
# PLUGIN PACKAGING
# =============================================================================

if(BUILD_PLUGIN)
  # VST3 plugin packaging
  if(EXISTS ${CMAKE_BINARY_DIR}/VitalVST3_artefacts)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalVST3_artefacts/
      DESTINATION lib/vst3
      FILES_MATCHING PATTERN "*.vst3"
    )
    
    # Add VST3 component to package
    set(CPACK_COMPONENT_PLUGIN_DESCRIPTION "VST3 plugin - ${CPACK_PACKAGE_DESCRIPTION}")
  endif()
  
  # AU plugin packaging (macOS only)
  if(EXISTS ${CMAKE_BINARY_DIR}/VitalAU_artefacts AND VITAL_PLATFORM_MACOS)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalAU_artefacts/
      DESTINATION Library/Audio/Plug-Ins/Components
      FILES_MATCHING PATTERN "*.component"
    )
  endif()
endif()

# =============================================================================
# STANDALONE APPLICATION PACKAGING
# =============================================================================

if(BUILD_STANDALONE)
  # Install standalone application
  if(VITAL_PLATFORM_WINDOWS)
    install(TARGETS VitalStandalone RUNTIME DESTINATION bin)
  elseif(VITAL_PLATFORM_MACOS)
    install(TARGETS VitalStandalone BUNDLE DESTINATION .)
  else()
    install(TARGETS VitalStandalone RUNTIME DESTINATION bin)
  endif()
  
  # Add standalone component to package
  set(CPACK_COMPONENT_STANDALONE_DESCRIPTION "Standalone ${CPACK_PACKAGE_DESCRIPTION}")
endif()

# =============================================================================
# DOCUMENTATION PACKAGING
# =============================================================================

# Install documentation
if(BUILD_DOCS)
  install(DIRECTORY ${CMAKE_BINARY_DIR}/docs/
    DESTINATION share/doc/${VITAL_PACKAGE_NAME}
    FILES_MATCHING PATTERN "*.html" PATTERN "*.pdf" PATTERN "*.md"
  )
  
  # Add docs component to package
  set(CPACK_COMPONENT_DOCS_DESCRIPTION "User and developer documentation for ${CPACK_PACKAGE_NAME}")
endif()

# =============================================================================
# EXAMPLE PROJECTS PACKAGING
# =============================================================================

if(BUILD_EXAMPLES)
  # Install example projects
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/
    DESTINATION share/${VITAL_PACKAGE_NAME}/examples
    FILES_MATCHING PATTERN "*.cpp" PATTERN "*.h" PATTERN "*.txt" PATTERN "CMakeLists.txt"
  )
  
  # Add examples component to package
  set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Example code and demos for ${VITAL_PACKAGE_NAME}")
endif()

# =============================================================================
# LIBRARY PACKAGING
# =============================================================================

# Core library
install(TARGETS VitalCore
  EXPORT VitalTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/vital/
  DESTINATION include/vital
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# =============================================================================
# PACKAGE SIGNING (Optional)
# =============================================================================

# macOS code signing
if(VITAL_PLATFORM_MACOS AND DEFINED ENV{CODESIGN_IDENTITY})
  set(CPACK_DMG_CODESIGN_IDENTITY $ENV{CODESIGN_IDENTITY})
  set(CPACK_DMG_CODESIGN_IDENTITY_STUB ${CMAKE_CURRENT_SOURCE_DIR}/cmake/entitlements.plist)
endif()

# Windows code signing
if(VITAL_PLATFORM_WINDOWS AND DEFINED ENV{CODESIGN_CERT})
  set(CPACK_NSIS_SIGNING_TOOL "signtool")
  set(CPACK_NSIS_SIGNING_ALG "sha256")
  set(CPACK_NSIS_SIGNING_CERT $ENV{CODESIGN_CERT})
  set(CPACK_NSIS_SIGNING_CERT_PASSWORD $ENV{CODESIGN_PASSWORD})
  set(CPACK_NSIS_SIGNING_TIMESTAMP_SERVER "http://timestamp.sectigo.com")
endif()

# =============================================================================
# CROSS-PLATFORM INSTALLATION RULES
# =============================================================================

# Desktop integration files
if(VITAL_PLATFORM_LINUX)
  # Desktop entry
  file(WRITE ${CMAKE_BINARY_DIR}/vital.desktop
    "[Desktop Entry]
Name=Vital Synthesizer
Comment=${VITAL_PACKAGE_DESCRIPTION}
Exec=vital-standalone
Icon=vital
Terminal=false
Type=Application
Categories=Audio;Music;
MimeType=audio/x-vital-patch;
"
  )
  
  install(FILES ${CMAKE_BINARY_DIR}/vital.desktop
    DESTINATION share/applications
  )
  
  # MIME type
  file(WRITE ${CMAKE_BINARY_DIR}/vital-mime.xml
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<mime-info xmlns=\"http://www.freedesktop.org/standards/shared-mime-info\">
  <mime-type type=\"audio/x-vital-patch\">
    <comment>Vital Patch File</comment>
    <comment xml:lang=\"en\">Vital Patch File</comment>
    <glob pattern=\"*.vital\"/>
    <sub-class-of type=\"audio/x-wav\"/>
  </mime-type>
</mime-info>
")
  
  install(FILES ${CMAKE_BINARY_DIR}/vital-mime.xml
    DESTINATION share/mime/packages
  )
  
  # Update MIME database
  add_custom_command(TARGET VitalCore POST_BUILD
    COMMAND update-mime-database ${CMAKE_INSTALL_PREFIX}/share/mime
    COMMENT "Updating MIME database"
  )
endif()

# =============================================================================
# PACKAGE BUILD TARGETS
# =============================================================================

# Package building targets
if(BUILD_TGZ_PACKAGE OR BUILD_ZIP_PACKAGE)
  add_custom_target(package-universal
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package
    COMMENT "Building universal package"
  )
endif()

if(BUILD_DEB_PACKAGE)
  add_custom_target(package-deb
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package_deb
    COMMENT "Building Debian package"
  )
endif()

if(BUILD_RPM_PACKAGE)
  add_custom_target(package-rpm
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package_rpm
    COMMENT "Building RPM package"
  )
endif()

if(BUILD_DMG_PACKAGE)
  add_custom_target(package-dmg
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package_dmg
    COMMENT "Building macOS DMG package"
  )
endif()

if(BUILD_MSI_PACKAGE)
  add_custom_target(package-msi
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package_msi
    COMMENT "Building Windows MSI package"
  )
endif()

if(BUILD_NSIS_PACKAGE)
  add_custom_target(package-nsis
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package_nsis
    COMMENT "Building Windows NSIS package"
  )
endif()

# Master package target
add_custom_target(package-all
  DEPENDS 
    $<$<BOOL:${BUILD_TGZ_PACKAGE}>:package-universal>
    $<$<BOOL:${BUILD_DEB_PACKAGE}>:package-deb>
    $<$<BOOL:${BUILD_RPM_PACKAGE}>:package-rpm>
    $<$<BOOL:${BUILD_DMG_PACKAGE}>:package-dmg>
    $<$<BOOL:${BUILD_MSI_PACKAGE}>:package-msi>
    $<$<BOOL:${BUILD_NSIS_PACKAGE}>:package-nsis>
    $<$<BOOL:${BUILD_APP_IMAGE}>:create-appimage>
  COMMENT "Building all packages"
)

# =============================================================================
# PACKAGE SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "=== Package Configuration Summary ===")
message(STATUS "Package Name:           ${VITAL_PACKAGE_NAME}")
message(STATUS "Package Version:        ${VITAL_PACKAGE_VERSION}")
message(STATUS "Package Vendor:         ${VITAL_PACKAGE_VENDOR}")
message(STATUS "Package License:        ${VITAL_PACKAGE_LICENSE}")
message(STATUS "")
message(STATUS "Package Formats:")
message(STATUS "  tar.gz:               ${BUILD_TGZ_PACKAGE}")
message(STATUS "  ZIP:                  ${BUILD_ZIP_PACKAGE}")
message(STATUS "  Debian:               ${BUILD_DEB_PACKAGE}")
message(STATUS "  RPM:                  ${BUILD_RPM_PACKAGE}")
message(STATUS "  macOS DMG:            ${BUILD_DMG_PACKAGE}")
message(STATUS "  Windows MSI:          ${BUILD_MSI_PACKAGE}")
message(STATUS "  Windows NSIS:         ${BUILD_NSIS_PACKAGE}")
message(STATUS "  Linux AppImage:       ${BUILD_APP_IMAGE}")
message(STATUS "==================================")
message(STATUS "")