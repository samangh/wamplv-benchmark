cmake_minimum_required(VERSION 3.13.4)

project(wamplv-benchmark
  VERSION "0.0.1"
  DESCRIPTION "Cross-pltform Data Acquisition Sequencer"
  LANGUAGES C CXX)

set(NAMESPACE wamplv-benchmark)

##
## CMake module paths
##

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

##
## Compile support
##

# Export compile database for IDEs, needed for QtCreator
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

##
## Import functions
##

include(setup_ide_folders)
include(get_standard_library_name)

# Check for name of standard library, use by common
include(CheckCXXSourceCompiles)
get_standard_library_name(STANDARD_LIBRARY)

##
## Packages
##

find_package(wampcc REQUIRED)
set_target_properties(wampcc::wampcc_shared
  PROPERTIES
  IMPORTED_GLOBAL
  TRUE)
add_library(wampcc ALIAS wampcc::wampcc_shared)

##
## Targets/Sources
##

add_subdirectory(external/cli11)
add_subdirectory(publish)

# Generate target list
get_property(TARGETS GLOBAL PROPERTY ${PROJECT_NAME}_TARGETS)

##
## Compile options
##

# Set the default build type if none specified by user
include(BuildType)

foreach(t ${TARGETS})
  # Enable extra errors for all projects
  target_compile_options(${t}
    PRIVATE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:-Wall>
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>)

  target_compile_features(${t} PRIVATE cxx_std_17)
endforeach()
