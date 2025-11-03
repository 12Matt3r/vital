# Installation Configuration
# Handles installation rules, directory structure, and platform-specific installation

# =============================================================================
# INSTALLATION DIRECTORY STRUCTURE
# =============================================================================

# Set installation directories
set(VITAL_INSTALL_BIN_DIR bin CACHE PATH "Installation binary directory")
set(VITAL_INSTALL_LIB_DIR lib CACHE PATH "Installation library directory")
set(VITAL_INSTALL_INCLUDE_DIR include CACHE PATH "Installation include directory")
set(VITAL_INSTALL_SHARE_DIR share CACHE PATH "Installation share directory")
set(VITAL_INSTALL_DOC_DIR share/doc CACHE PATH "Installation documentation directory")
set(VITAL_INSTALL_DATA_DIR share CACHE PATH "Installation data directory")

# Platform-specific directory adjustments
if(VITAL_PLATFORM_WINDOWS)
  # Windows-specific directories
  set(CMAKE_INSTALL_BINDIR "${VITAL_INSTALL_BIN_DIR}")
  set(CMAKE_INSTALL_LIBDIR "${VITAL_INSTALL_LIB_DIR}")
  set(CMAKE_INSTALL_INCLUDEDIR "${VITAL_INSTALL_INCLUDE_DIR}")
  set(CMAKE_INSTALL_DATADIR "${VITAL_INSTALL_DATA_DIR}")
  
elseif(VITAL_PLATFORM_MACOS)
  # macOS framework-style directories
  set(CMAKE_INSTALL_BINDIR "${VITAL_INSTALL_BIN_DIR}")
  set(CMAKE_INSTALL_LIBDIR "${VITAL_INSTALL_LIB_DIR}")
  set(CMAKE_INSTALL_INCLUDEDIR "${VITAL_INSTALL_INCLUDE_DIR}")
  set(CMAKE_INSTALL_DATADIR "${VITAL_INSTALL_DATA_DIR}")
  
elseif(VITAL_PLATFORM_LINUX)
  # Linux FHS-compliant directories
  set(CMAKE_INSTALL_BINDIR "${VITAL_INSTALL_BIN_DIR}")
  set(CMAKE_INSTALL_LIBDIR "${VITAL_INSTALL_LIB_DIR}")
  set(CMAKE_INSTALL_INCLUDEDIR "${VITAL_INSTALL_INCLUDE_DIR}")
  set(CMAKE_INSTALL_DATADIR "${VITAL_INSTALL_DATA_DIR}")
endif()

# =============================================================================
# CORE LIBRARY INSTALLATION
# =============================================================================

# Install core library
install(TARGETS VitalCore
  EXPORT VitalTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install library headers
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/vital/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vital
  FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.tpp"
  PATTERN ".git" EXCLUDE
  PATTERN "*.tmp" EXCLUDE
)

# =============================================================================
# PLUGIN INSTALLATION
# =============================================================================

if(BUILD_PLUGIN)
  message(STATUS "=== Plugin Installation ===")
  
  # VST3 Plugin installation
  if(EXISTS ${CMAKE_BINARY_DIR}/VitalVST3_artefacts)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalVST3_artefacts/
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/vst3
      FILES_MATCHING
        PATTERN "*.vst3"
    )
    message(STATUS "  VST3 Plugin:        ${CMAKE_INSTALL_LIBDIR}/vst3")
  endif()
  
  # Audio Unit Plugin installation (macOS only)
  if(EXISTS ${CMAKE_BINARY_DIR}/VitalAU_artefacts)
    if(VITAL_PLATFORM_MACOS)
      install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalAU_artefacts/
        DESTINATION /Library/Audio/Plug-Ins/Components
        FILES_MATCHING
          PATTERN "*.component"
      )
      message(STATUS "  AU Plugin:          /Library/Audio/Plug-Ins/Components")
    else()
      install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalAU_artefacts/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/au
        FILES_MATCHING
          PATTERN "*.component"
      )
      message(STATUS "  AU Plugin:          ${CMAKE_INSTALL_LIBDIR}/au")
    endif()
  endif()
  
  # VST2 Plugin installation (legacy)
  set(VST2_PLUGIN_DIR "${CMAKE_BINARY_DIR}/VST2")
  if(EXISTS ${VST2_PLUGIN_DIR})
    install(DIRECTORY ${VST2_PLUGIN_DIR}/
      DESTINATION ${CMAKE_INSTALL_LIBDIR}/vst
      FILES_MATCHING
        PATTERN "*.dll"
        PATTERN "*.vst"
    )
    message(STATUS "  VST2 Plugin:        ${CMAKE_INSTALL_LIBDIR}/vst")
  endif()
endif()

# =============================================================================
# STANDALONE APPLICATION INSTALLATION
# =============================================================================

if(BUILD_STANDALONE)
  message(STATUS "=== Standalone Application Installation ===")
  
  if(VITAL_PLATFORM_WINDOWS)
    # Windows standalone application
    install(TARGETS VitalStandalone
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    # Install Windows-specific files
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/vital.ico)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/vital.ico
        DESTINATION ${CMAKE_INSTALL_DATADIR}/vital
        RENAME vital.ico
      )
    endif()
    
    # Install Windows manifest
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/vital.manifest)
      install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/vital.manifest
        DESTINATION ${CMAKE_INSTALL_DATADIR}/vital
        RENAME vital.manifest
      )
    endif()
    
    message(STATUS "  Windows App:        ${CMAKE_INSTALL_BINDIR}/VitalStandalone.exe")
    
  elseif(VITAL_PLATFORM_MACOS)
    # macOS standalone application bundle
    install(TARGETS VitalStandalone
      BUNDLE DESTINATION .
    )
    
    # macOS application bundle customization
    set_target_properties(VitalStandalone PROPERTIES
      BUNDLE TRUE
      BUNDLE_NAME "Vital Synthesizer"
      BUNDLE_GUI_IDENTIFIER "com.vital.synthesizer"
      BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
      BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
      BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
    )
    
    # Install Info.plist
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist
      DESTINATION Resources
    )
    
    message(STATUS "  macOS App Bundle:   Vital.app")
    
  else()
    # Linux standalone application
    install(TARGETS VitalStandalone
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    # Make executable
    install(CODE "
      file(PERMISSIONS
        \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/VitalStandalone
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                       GROUP_READ GROUP_EXECUTE
                       WORLD_READ WORLD_EXECUTE
      )
    ")
    
    message(STATUS "  Linux App:          ${CMAKE_INSTALL_BINDIR}/VitalStandalone")
  endif()
endif()

# =============================================================================
# RESOURCES AND ASSETS INSTALLATION
# =============================================================================

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources)
  message(STATUS "=== Resources Installation ===")
  
  # Install all resource files
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vital
    FILES_MATCHING
      PATTERN "*.wav"
      PATTERN "*.png"
      PATTERN "*.jpg"
      PATTERN "*.ico"
      PATTERN "*.json"
      PATTERN "*.xml"
      PATTERN "*.plist"
      PATTERN "*.txt"
      PATTERN "*.md"
    PATTERN ".git" EXCLUDE
    PATTERN "*.tmp" EXCLUDE
  )
  
  # Install preset files
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/presets)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/presets/
      DESTINATION ${CMAKE_INSTALL_DATADIR}/vital/presets
    )
  endif()
  
  # Install factory sounds
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/sounds)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/resources/sounds/
      DESTINATION ${CMAKE_INSTALL_DATADIR}/vital/sounds
    )
  endif()
  
  message(STATUS "  Resources:          ${CMAKE_INSTALL_DATADIR}/vital")
endif()

# =============================================================================
# DOCUMENTATION INSTALLATION
# =============================================================================

if(BUILD_DOCS)
  message(STATUS "=== Documentation Installation ===")
  
  # Install generated documentation
  if(EXISTS ${CMAKE_BINARY_DIR}/docs)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/docs/
      DESTINATION ${CMAKE_INSTALL_DOC_DIR}/vital
      FILES_MATCHING
        PATTERN "*.html"
        PATTERN "*.css"
        PATTERN "*.js"
        PATTERN "*.pdf"
    )
  endif()
  
  # Install README and other documentation files
  file(GLOB DOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    ${CMAKE_CURRENT_SOURCE_DIR}/AUTHORS
  )
  
  foreach(DOC_FILE ${DOC_FILES})
    if(EXISTS ${DOC_FILE})
      install(FILES ${DOC_FILE}
        DESTINATION ${CMAKE_INSTALL_DOC_DIR}/vital
      )
    endif()
  endforeach()
  
  message(STATUS "  Documentation:      ${CMAKE_INSTALL_DOC_DIR}/vital")
endif()

# =============================================================================
# EXAMPLE PROJECTS INSTALLATION
# =============================================================================

if(BUILD_EXAMPLES)
  message(STATUS "=== Examples Installation ===")
  
  # Install example projects
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vital/examples
    FILES_MATCHING
      PATTERN "*.cpp"
      PATTERN "*.h"
      PATTERN "*.hpp"
      PATTERN "*.txt"
      PATTERN "CMakeLists.txt"
      PATTERN "*.md"
    PATTERN ".git" EXCLUDE
  )
  
  # Make example files executable where needed
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vital/examples
    DIRECTORY_PERMISSIONS
      OWNER_READ OWNER_WRITE OWNER_EXECUTE
      GROUP_READ GROUP_EXECUTE
      WORLD_READ WORLD_EXECUTE
    FILE_PERMISSIONS
      OWNER_READ OWNER_WRITE
      GROUP_READ
      WORLD_READ
  )
  
  message(STATUS "  Examples:           ${CMAKE_INSTALL_DATADIR}/vital/examples")
endif()

# =============================================================================
# DEVELOPMENT FILES INSTALLATION
# =============================================================================

# Install CMake package configuration
install(EXPORT VitalTargets
  FILE VitalTargets.cmake
  NAMESPACE Vital::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Vital
)

# Install CMake package version file
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/VitalConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VitalConfig.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Vital
)

# Install pkg-config file (Linux/Unix)
if(VITAL_PLATFORM_UNIX)
  file(WRITE ${CMAKE_BINARY_DIR}/vital.pc
    "prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}
includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}

Name: Vital Synthesizer
Description: ${VITAL_PACKAGE_DESCRIPTION}
Version: ${PROJECT_VERSION}
Libs: -L\${libdir} -lVitalCore
Cflags: -I\${includedir}
Requires.private: juce-7.0
"
  )
  
  install(FILES ${CMAKE_BINARY_DIR}/vital.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  )
endif()

# Install pkg-config file for macOS framework
if(VITAL_PLATFORM_MACOS)
  file(WRITE ${CMAKE_BINARY_DIR}/vital-macos.pc
    "prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/${CMAKE_INSTALL_LIBDIR}
includedir=\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}
frameworkdir=\${exec_prefix}/Library/Frameworks

Name: Vital Synthesizer
Description: ${VITAL_PACKAGE_DESCRIPTION}
Version: ${PROJECT_VERSION}
Libs: -F\${frameworkdir} -framework VitalCore
Cflags: -I\${includedir}
Requires.private: juce-7.0
"
  )
  
  install(FILES ${CMAKE_BINARY_DIR}/vital-macos.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  )
endif()

# =============================================================================
# PLATFORM-SPECIFIC INSTALLATION RULES
# =============================================================================

# Windows-specific installation
if(VITAL_PLATFORM_WINDOWS)
  # Install Visual C++ redistributable information
  install(CODE "
    message(STATUS \"Installing Visual C++ runtime requirements\")
    message(STATUS \"Please ensure Visual C++ 2019/2022 redistributable is installed\")
  ")
  
  # Register VST3 plugin in registry (optional)
  install(CODE "
    if(CMAKE_INSTALL_PREFIX MATCHES \"Program Files\")
      message(STATUS \"Registering VST3 plugin with system...\")
      # Registry registration would go here
    endif()
  ")
endif()

# macOS-specific installation
if(VITAL_PLATFORM_MACOS)
  # Install macOS-specific files
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist
      DESTINATION Resources
    )
  endif()
  
  # Code signing for distribution
  if(DEFINED ENV{CODESIGN_IDENTITY})
    install(CODE "
      execute_process(
        COMMAND codesign --force --deep --sign \"$ENV{CODESIGN_IDENTITY}\"
                     \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/Vital.app\"
        RESULT_VARIABLE CODESIGN_RESULT
      )
      if(CODESIGN_RESULT EQUAL 0)
        message(STATUS \"Application code signing: SUCCESS\")
      else()
        message(WARNING \"Application code signing: FAILED\")
      endif()
    ")
  endif()
endif()

# Linux-specific installation
if(VITAL_PLATFORM_LINUX)
  # Desktop integration
  install(CODE "
    if(EXISTS \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications/vital.desktop\")
      execute_process(
        COMMAND update-desktop-database
                \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications\"
        RESULT_VARIABLE DESKTOP_DB_RESULT
      )
      if(DESKTOP_DB_RESULT EQUAL 0)
        message(STATUS \"Desktop database updated successfully\")
      else()
        message(WARNING \"Desktop database update failed\")
      endif()
    endif()
  ")
  
  # MIME type registration
  install(CODE "
    if(EXISTS \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/mime/packages/vital-mime.xml\")
      execute_process(
        COMMAND update-mime-database
                \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/mime\"
        RESULT_VARIABLE MIME_DB_RESULT
      )
      if(MIME_DB_RESULT EQUAL 0)
        message(STATUS \"MIME database updated successfully\")
      else()
        message(WARNING \"MIME database update failed\")
      endif()
    endif()
  ")
  
  # Update shared library cache
  install(CODE "
    if(EXISTS /sbin/ldconfig)
      execute_process(
        COMMAND /sbin/ldconfig -n ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}
        RESULT_VARIABLE LDCONFIG_RESULT
      )
      if(LDCONFIG_RESULT EQUAL 0)
        message(STATUS \"Shared library cache updated\")
      else()
        message(WARNING \"Shared library cache update failed\")
      endif()
    endif()
  ")
endif()

# =============================================================================
# PLUGIN AUTO-DISCOVERY
# =============================================================================

# Create plugin discovery script
set(PLUGIN_DISCOVERY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/scripts/discover_plugins.py")
if(EXISTS ${PLUGIN_DISCOVERY_SCRIPT})
  install(FILES ${PLUGIN_DISCOVERY_SCRIPT}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/vital/scripts
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE
  )
endif()

# =============================================================================
# POST-INSTALLATION TASKS
# =============================================================================

# Create installation verification script
install(CODE "
  set(INSTALL_VERIFICATION_FILE \"\${CMAKE_INSTALL_PREFIX}/.vital_installation_verified\")
  file(WRITE \${INSTALL_VERIFICATION_FILE}
    \"Vital Synthesizer installation verified on: \${CMAKE_CURRENT_LIST_DIR}\"
  )
  message(STATUS \"Installation verification completed\")
")

# =============================================================================
# INSTALLATION SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "=== Installation Configuration ===")
message(STATUS "Install Prefix:         ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Binary Directory:       ${CMAKE_INSTALL_BINDIR}")
message(STATUS "Library Directory:      ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "Include Directory:      ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "Data Directory:         ${CMAKE_INSTALL_DATADIR}")
message(STATUS "Documentation:          ${CMAKE_INSTALL_DOC_DIR}")
message(STATUS "")
message(STATUS "Platform:               ${VITAL_PLATFORM_NAME}")
message(STATUS "Standalone App:         ${BUILD_STANDALONE}")
message(STATUS "Plugin:                 ${BUILD_PLUGIN}")
message(STATUS "Documentation:          ${BUILD_DOCS}")
message(STATUS "Examples:               ${BUILD_EXAMPLES}")
message(STATUS "===============================")
message(STATUS "")