# Static Analysis Configuration
# Integration with various static analysis tools for code quality assurance

# =============================================================================
# STATIC ANALYSIS OPTIONS
# =============================================================================

# Tool-specific options
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy static analysis" OFF)
option(ENABLE_CPPCHECK "Enable cppcheck static analysis" OFF)
option(ENABLE_PVS_STUDIO "Enable PVS-Studio static analysis" OFF)
option(ENABLE_INFER "Enable Facebook Infer static analysis" OFF)
option(ENABLE_FLAWFINDER "Enable flawfinder security analysis" OFF)
option(ENABLE_SCAN_BUILD "Enable scan-build (clang analyzer)" OFF)
option(ENABLE_CPP_LINT "Enable cpplint style checking" OFF)

# Clang-Tidy specific options
set(CLANG_TIDY_CHECKS "*,performance-*,readability-*,modernize-*,bugprone-*,clang-analyzer-*" CACHE STRING "Clang-Tidy checks to enable")
set(CLANG_TIDY_DISABLE_CHECKS "" CACHE STRING "Clang-Tidy checks to disable")
set(CLANG_TIDY_WARNINGS_AS_ERRORS "" CACHE STRING "Treat specific warnings as errors")

# =============================================================================
# CLANG-TIDY CONFIGURATION
# =============================================================================

if(ENABLE_CLANG_TIDY)
  message(STATUS "=== Clang-Tidy Static Analysis ===")
  
  # Find clang-tidy
  find_program(CLANG_TIDY_EXE
    NAMES clang-tidy
    HINTS /usr/bin /usr/local/bin
  )
  
  if(CLANG_TIDY_EXE)
    message(STATUS "  Found:             ${CLANG_TIDY_EXE}")
    
    # Get clang-tidy version
    execute_process(
      COMMAND ${CLANG_TIDY_EXE} --version
      OUTPUT_VARIABLE CLANG_TIDY_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "  Version:           ${CLANG_TIDY_VERSION}")
    
    # Create clang-tidy configuration file
    set(CLANG_TIDY_CONFIG_FILE "${CMAKE_BINARY_DIR}/clang_tidy_config.json")
    file(WRITE ${CLANG_TIDY_CONFIG_FILE}
      {
        "Checks": "${CLANG_TIDY_CHECKS}",
        "CheckOptions": [
          {
            "key": "modernize-loop-convert.MaxCopySize",
            "value": 16
          },
          {
            "key": "modernize-make-shared.IgnoreImplicitFuncs",
            "value": "std::make_shared"
          },
          {
            "key": "performance-for-range-copy.Emit copy size",
            "value": "16"
          }
        ]
      }
    )
    
    # Add checks configuration
    if(CLANG_TIDY_DISABLE_CHECKS)
      string(REPLACE "," ",-" DISABLED_CHECKS "-${CLANG_TIDY_DISABLE_CHECKS}")
      file(APPEND ${CLANG_TIDY_CONFIG_FILE}
        ,
        {
          "CheckOptions": [
            {
              "key": "DisableChecks",
              "value": "${DISABLED_CHECKS}"
            }
          ]
        }
      )
    endif()
    
    # Set warnings as errors
    if(CLANG_TIDY_WARNINGS_AS_ERRORS)
      file(APPEND ${CLANG_TIDY_CONFIG_FILE}
        ,
        {
          "WarningsAsErrors": "${CLANG_TIDY_WARNINGS_AS_ERRORS}"
        }
      )
    endif()
    
    # Apply clang-tidy to target
    set_target_properties(VitalCore PROPERTIES
      CXX_CLANG_TIDY
      "${CLANG_TIDY_EXE};-config=${CLANG_TIDY_CONFIG_FILE};-header-filter=${CMAKE_CURRENT_SOURCE_DIR}/include/*;${CLANG_TIDY_EXTRA_ARGS}"
    )
    
    # Custom target for clang-tidy
    add_custom_target(clang-tidy
      COMMAND ${CLANG_TIDY_EXE} -config=${CLANG_TIDY_CONFIG_FILE}
              -header-filter=${CMAKE_CURRENT_SOURCE_DIR}/include/*
              ${CLANG_TIDY_EXTRA_ARGS}
              ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running Clang-Tidy static analysis"
    )
    
    message(STATUS "  Config:            ${CLANG_TIDY_CONFIG_FILE}")
    message(STATUS "  Checks:            ${CLANG_TIDY_CHECKS}")
    
  else()
    message(WARNING "Clang-Tidy not found. Install clang-tidy to enable static analysis.")
  endif()
endif()

# =============================================================================
# CPPCHECK CONFIGURATION
# =============================================================================

if(ENABLE_CPPCHECK)
  message(STATUS "=== cppcheck Static Analysis ===")
  
  # Find cppcheck
  find_program(CPPCHECK_EXE
    NAMES cppcheck
    HINTS /usr/bin /usr/local/bin
  )
  
  if(CPPCHECK_EXE)
    message(STATUS "  Found:             ${CPPCHECK_EXE}")
    
    # Get cppcheck version
    execute_process(
      COMMAND ${CPPCHECK_EXE} --version
      OUTPUT_VARIABLE CPPCHECK_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "  Version:           ${CPPCHECK_VERSION}")
    
    # cppcheck options
    set(CPPCHECK_OPTIONS
      --enable=all
      --std=c++20
      --platform=unix64
      --inline-suppr
      --quiet
      --template=gcc
    )
    
    # Add include paths
    list(APPEND CPPCHECK_OPTIONS --include-dir=${CMAKE_CURRENT_SOURCE_DIR}/include)
    
    # Add includes
    list(APPEND CPPCHECK_OPTIONS 
      -I ${CMAKE_CURRENT_SOURCE_DIR}/include
      -I ${VITAL_PHASE1_DIR}/juce_modernization
      -I ${VITAL_PHASE2_DIR}/simd_optimization
      -I ${VITAL_PHASE3_DIR}
      -I ${VITAL_PHASE4_DIR}/modern_interface/include
    )
    
    # Custom target for cppcheck
    add_custom_target(cppcheck-analysis
      COMMAND ${CPPCHECK_EXE} ${CPPCHECK_OPTIONS}
              ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running cppcheck static analysis"
    )
    
  else()
    message(WARNING "cppcheck not found. Install cppcheck to enable static analysis.")
  endif()
endif()

# =============================================================================
# PVS-STUDIO CONFIGURATION
# =============================================================================

if(ENABLE_PVS_STUDIO)
  message(STATUS "=== PVS-Studio Static Analysis ===")
  
  # Find PVS-Studio
  find_program(PVS_STUDIO_EXE
    NAMES pvs-studio pvs-studio-analyzer
    HINTS /usr/bin /usr/local/bin /opt/pvs-studio
  )
  
  if(PVS_STUDIO_EXE)
    message(STATUS "  Found:             ${PVS_STUDIO_EXE}")
    
    # Get PVS-Studio version
    execute_process(
      COMMAND ${PVS_STUDIO_EXE} --version
      OUTPUT_VARIABLE PVS_STUDIO_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "  Version:           ${PVS_STUDIO_VERSION}")
    
    # PVS-Studio options
    set(PVS_STUDIO_OPTIONS
      --analysis-mode=4
      --output-file=${CMAKE_BINARY_DIR}/pvs-studio.log
      --file-list=${CMAKE_BINARY_DIR}/source_files.txt
    )
    
    # Generate source file list
    file(WRITE ${CMAKE_BINARY_DIR}/source_files.txt "")
    file(GLOB_RECURSE SOURCE_FILES 
      ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
    )
    
    foreach(SOURCE_FILE ${SOURCE_FILES})
      file(APPEND ${CMAKE_BINARY_DIR}/source_files.txt "${SOURCE_FILE}\n")
    endforeach()
    
    # Custom target for PVS-Studio
    add_custom_target(pvs-studio-analysis
      COMMAND ${PVS_STUDIO_EXE} ${PVS_STUDIO_OPTIONS}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running PVS-Studio static analysis"
    )
    
  else()
    message(WARNING "PVS-Studio not found. Install PVS-Studio to enable static analysis.")
  endif()
endif()

# =============================================================================
# FACEBOOK INFER CONFIGURATION
# =============================================================================

if(ENABLE_INFER)
  message(STATUS "=== Facebook Infer Static Analysis ===")
  
  # Find Infer
  find_program(INFER_EXE
    NAMES infer
    HINTS /usr/bin /usr/local/bin /opt/infer/bin
  )
  
  if(INFER_EXE)
    message(STATUS "  Found:             ${INFER_EXE}")
    
    # Get Infer version
    execute_process(
      COMMAND ${INFER_EXE} --version
      OUTPUT_VARIABLE INFER_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "  Version:           ${INFER_VERSION}")
    
    # Create Infer configuration
    set(INFER_CONFIG_FILE "${CMAKE_BINARY_DIR}/infer.json")
    file(WRITE ${INFER_CONFIG_FILE}
      {
        "project-root": "${CMAKE_CURRENT_SOURCE_DIR}",
        "source-path": [
          "${CMAKE_CURRENT_SOURCE_DIR}/src",
          "${CMAKE_CURRENT_SOURCE_DIR}/include"
        ],
        "compilation-database": "compile_commands.json",
        "results-dir": "${CMAKE_BINARY_DIR}/infer-out"
      }
    )
    
    # Custom target for Infer
    add_custom_target(infer-analysis
      COMMAND ${INFER_EXE} run --config ${INFER_CONFIG_FILE}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running Facebook Infer static analysis"
    )
    
  else()
    message(WARNING "Facebook Infer not found. Install Infer to enable static analysis.")
  endif()
endif()

# =============================================================================
# FLAWFINDER CONFIGURATION
# =============================================================================

if(ENABLE_FLAWFINDER)
  message(STATUS "=== flawfinder Security Analysis ===")
  
  # Find flawfinder
  find_program(FLAWFINDER_EXE
    NAMES flawfinder
    HINTS /usr/bin /usr/local/bin
  )
  
  if(FLAWFINDER_EXE)
    message(STATUS "  Found:             ${FLAWFINDER_EXE}")
    
    # flawfinder options
    set(FLAWFINDER_OPTIONS
      --context
      --minlevel=1
      --dataonly
    )
    
    # Custom target for flawfinder
    add_custom_target(flawfinder-analysis
      COMMAND ${FLAWFINDER_EXE} ${FLAWFINDER_OPTIONS}
              ${CMAKE_CURRENT_SOURCE_DIR}/src/
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running flawfinder security analysis"
    )
    
  else()
    message(WARNING "flawfinder not found. Install flawfinder to enable security analysis.")
  endif()
endif()

# =============================================================================
# CLANG ANALYZER (scan-build) CONFIGURATION
# =============================================================================

if(ENABLE_SCAN_BUILD)
  message(STATUS "=== Clang Static Analyzer (scan-build) ===")
  
  # Find scan-build
  find_program(SCAN_BUILD_EXE
    NAMES scan-build
    HINTS /usr/bin /usr/local/bin
  )
  
  if(SCAN_BUILD_EXE)
    message(STATUS "  Found:             ${SCAN_BUILD_EXE}")
    
    # Custom target for scan-build
    add_custom_target(scan-build-analysis
      COMMAND ${SCAN_BUILD_EXE} -o ${CMAKE_BINARY_DIR}/scan-build
              ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}
              -c ${CMAKE_CURRENT_SOURCE_DIR}/src/synthesizer_core.cpp
              -o ${CMAKE_BINARY_DIR}/test.o
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running Clang Static Analyzer"
    )
    
  else()
    message(WARNING "scan-build not found. Install clang to enable static analysis.")
  endif()
endif()

# =============================================================================
# CPPLINT CONFIGURATION
# =============================================================================

if(ENABLE_CPP_LINT)
  message(STATUS "=== cpplint Style Checking ===")
  
  # Find cpplint
  find_program(CPPLINT_EXE
    NAMES cpplint.py cpplint
    HINTS /usr/bin /usr/local/bin
  )
  
  if(CPPLINT_EXE)
    message(STATUS "  Found:             ${CPPLINT_EXE}")
    
    # cpplint options
    set(CPPLINT_OPTIONS
      --filter=-build/include_subdir
      --filter=-build/c++11
      --filter=-readability/casting
      --filter=-runtime/sizeof
      --counting=detailed
      --linelength=120
    )
    
    # Custom target for cpplint
    add_custom_target(cpplint-check
      COMMAND ${CPPLINT_EXE} ${CPPLINT_OPTIONS}
              ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
              ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMENT "Running cpplint style checking"
    )
    
  else()
    message(WARNING "cpplint not found. Install cpplint to enable style checking.")
  endif()
endif()

# =============================================================================
# COMPREHENSIVE STATIC ANALYSIS TARGET
# =============================================================================

if(ENABLE_STATIC_ANALYSIS)
  # Create a comprehensive static analysis target
  set(STATIC_ANALYSIS_TARGETS)
  
  if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS clang-tidy)
  endif()
  
  if(ENABLE_CPPCHECK AND CPPCHECK_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS cppcheck-analysis)
  endif()
  
  if(ENABLE_PVS_STUDIO AND PVS_STUDIO_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS pvs-studio-analysis)
  endif()
  
  if(ENABLE_INFER AND INFER_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS infer-analysis)
  endif()
  
  if(ENABLE_FLAWFINDER AND FLAWFINDER_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS flawfinder-analysis)
  endif()
  
  if(ENABLE_SCAN_BUILD AND SCAN_BUILD_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS scan-build-analysis)
  endif()
  
  if(ENABLE_CPP_LINT AND CPPLINT_EXE)
    list(APPEND STATIC_ANALYSIS_TARGETS cpplint-check)
  endif()
  
  # Master target for all static analysis
  if(STATIC_ANALYSIS_TARGETS)
    add_custom_target(static-analysis ALL
      DEPENDS ${STATIC_ANALYSIS_TARGETS}
      COMMENT "Running comprehensive static analysis"
    )
    
    # Add dependency to build
    add_dependencies(VitalCore static-analysis)
  endif()
endif()

# =============================================================================
# CODE QUALITY METRICS
# =============================================================================

# Cyclomatic complexity calculation
if(ENABLE_CLANG_TIDY)
  # Add cyclomatic complexity check
  target_compile_definitions(VitalCore PUBLIC VITAL_CYCLOMATIC_COMPLEXITY_CHECK=1)
endif()

# Static analysis report
set(STATIC_ANALYSIS_REPORT_FILE "${CMAKE_BINARY_DIR}/static_analysis_report.md")
file(WRITE ${STATIC_ANALYSIS_REPORT_FILE}
"# Vital Synthesizer - Static Analysis Report

Generated: $(date)

## Analysis Tools Configuration

### Clang-Tidy
- **Enabled**: ${ENABLE_CLANG_TIDY}
- **Checks**: ${CLANG_TIDY_CHECKS}
- **Executable**: ${CLANG_TIDY_EXE}

### cppcheck
- **Enabled**: ${ENABLE_CPPCHECK}
- **Executable**: ${CPPCHECK_EXE}

### PVS-Studio
- **Enabled**: ${ENABLE_PVS_STUDIO}
- **Executable**: ${PVS_STUDIO_EXE}

### Facebook Infer
- **Enabled**: ${ENABLE_INFER}
- **Executable**: ${INFER_EXE}

### flawfinder
- **Enabled**: ${ENABLE_FLAWFINDER}
- **Executable**: ${FLAWFINDER_EXE}

### Clang Analyzer (scan-build)
- **Enabled**: ${ENABLE_SCAN_BUILD}
- **Executable**: ${SCAN_BUILD_EXE}

### cpplint
- **Enabled**: ${ENABLE_CPP_LINT}
- **Executable**: ${CPPLINT_EXE}

## Source Files Analyzed

")

# Add source files to report
file(GLOB_RECURSE SOURCE_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
)

foreach(SOURCE_FILE ${SOURCE_FILES})
  file(APPEND ${STATIC_ANALYSIS_REPORT_FILE} "- ${SOURCE_FILE}\n")
endforeach()

file(APPEND ${STATIC_ANALYSIS_REPORT_FILE}
"\n## Analysis Results

Results are stored in individual analysis output files.

## Build Information

- **CMake Version**: ${CMAKE_VERSION}
- **Compiler**: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}
- **Build Type**: ${CMAKE_BUILD_TYPE}
- **Platform**: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}

"
)

# =============================================================================
# STATIC ANALYSIS SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "=== Static Analysis Configuration ===")
message(STATUS "Enabled Tools:")
if(ENABLE_CLANG_TIDY AND CLANG_TIDY_EXE)
  message(STATUS "  Clang-Tidy:        YES")
elseif(ENABLE_CLANG_TIDY)
  message(STATUS "  Clang-Tidy:        NOT FOUND")
endif()

if(ENABLE_CPPCHECK AND CPPCHECK_EXE)
  message(STATUS "  cppcheck:          YES")
elseif(ENABLE_CPPCHECK)
  message(STATUS "  cppcheck:          NOT FOUND")
endif()

if(ENABLE_PVS_STUDIO AND PVS_STUDIO_EXE)
  message(STATUS "  PVS-Studio:        YES")
elseif(ENABLE_PVS_STUDIO)
  message(STATUS "  PVS-Studio:        NOT FOUND")
endif()

if(ENABLE_INFER AND INFER_EXE)
  message(STATUS "  Facebook Infer:    YES")
elseif(ENABLE_INFER)
  message(STATUS "  Facebook Infer:    NOT FOUND")
endif()

if(ENABLE_FLAWFINDER AND FLAWFINDER_EXE)
  message(STATUS "  flawfinder:        YES")
elseif(ENABLE_FLAWFINDER)
  message(STATUS "  flawfinder:        NOT FOUND")
endif()

if(ENABLE_SCAN_BUILD AND SCAN_BUILD_EXE)
  message(STATUS "  scan-build:        YES")
elseif(ENABLE_SCAN_BUILD)
  message(STATUS "  scan-build:        NOT FOUND")
endif()

if(ENABLE_CPP_LINT AND CPPLINT_EXE)
  message(STATUS "  cpplint:           YES")
elseif(ENABLE_CPP_LINT)
  message(STATUS "  cpplint:           NOT FOUND")
endif()

message(STATUS "===================================")
message(STATUS "")