# SPDX-FileCopyrightText: 2026 Kaito Udagawa <umireon@kaito.tokyo>
# SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.28)

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

set(DIRECTML_DLL "" CACHE FILEPATH "Path to the DirectML redistributable DLL")
set(DIRECTML_INCLUDE_DIR "" CACHE PATH "Path containing DirectML.h")
option(REQUIRE_ONNXRUNTIME_DML_EP "Fail configuration when the ONNX Runtime DirectML EP is unavailable" OFF)

block(PROPAGATE PLUGIN_VERSION PLUGIN_PROJECT_VERSION)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_CONTENT)
  string(STRIP "${VERSION_CONTENT}" PLUGIN_VERSION)
  string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" PLUGIN_PROJECT_VERSION "${PLUGIN_VERSION}")
  if(NOT PLUGIN_VERSION MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+(-[0-9A-Za-z.-]+)?$")
    message(FATAL_ERROR "VERSION must contain a SemVer-compatible version string.")
  endif()
endblock()

project(obs-backgroundremoval VERSION "${PLUGIN_PROJECT_VERSION}" LANGUAGES C CXX)

include(CheckCXXSymbolExists)
include(CheckCXXSourceCompiles)
include(CMakePushCheckState)
include(CTest)
include(GNUInstallDirs)

include(./cmake/collect_licenses.cmake OPTIONAL)

block(
  PROPAGATE PLUGIN_DISPLAY_NAME PLUGIN_AUTHOR PLUGIN_EMAIL PLUGIN_BUNDLE_ID PLUGIN_UPDATE_FEED_URL
)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/buildspec.props" BUILDSPEC_LINES)
  foreach(LINE IN LISTS BUILDSPEC_LINES)
    if(LINE MATCHES "^([a-zA-Z][a-zA-Z0-9_]*)=(.*)$")
      set("${CMAKE_MATCH_1}" "${CMAKE_MATCH_2}")
    endif()
  endforeach()
endblock()

### Global configurations
set(CMAKE_C_EXTENSIONS FALSE)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

add_library(BuildOptions INTERFACE)

if(MSVC)
  set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release)
  target_compile_definitions(BuildOptions INTERFACE NOMINMAX WIN32_LEAN_AND_MEAN UNICODE _UNICODE)
  target_compile_options(
    BuildOptions
    INTERFACE
      /utf-8
      /Zc:preprocessor
      $<$<COMPILE_LANGUAGE:CXX>:/permissive->
      $<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>
      /W4
      /WX
  )
elseif(UNIX)
  target_compile_definitions(BuildOptions INTERFACE SIMDE_ENABLE_OPENMP)
  target_compile_options(
    BuildOptions
    INTERFACE -fno-omit-frame-pointer -fstack-protector-strong -fopenmp-simd -Wall -Wextra -Werror
  )
endif()

target_compile_definitions(
  BuildOptions
  INTERFACE
    PLUGIN_NAME_STR="${CMAKE_PROJECT_NAME}"
    PLUGIN_VERSION_STR="${PLUGIN_VERSION}"
    PLUGIN_UPDATE_FEED_URL="${PLUGIN_UPDATE_FEED_URL}"
    OBS_LOG_HEADER="[${CMAKE_PROJECT_NAME}] "
)

### Dependency helpers
function(map_imported_executable_to_release TARGET_NAME FALLBACK_LOCATION)
  if(NOT TARGET "${TARGET_NAME}")
    return()
  endif()

  get_target_property(TARGET_TYPE "${TARGET_NAME}" TYPE)
  get_target_property(TARGET_LOCATION "${TARGET_NAME}" IMPORTED_LOCATION)
  if(NOT TARGET_LOCATION)
    set(TARGET_LOCATION "${FALLBACK_LOCATION}")
  endif()

  if(TARGET_TYPE STREQUAL "EXECUTABLE" AND TARGET_LOCATION)
    set_target_properties(
      "${TARGET_NAME}"
      PROPERTIES
        IMPORTED_CONFIGURATIONS RELEASE
        IMPORTED_LOCATION_RELEASE "${TARGET_LOCATION}"
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )
  endif()
endfunction()

function(check_onnxruntime_execution_provider result symbol)
  set(found FALSE)
  set(index 0)

  foreach(header IN LISTS ARGN)
    math(EXPR index "${index} + 1")
    set(check_variable "${result}_${index}")
    check_cxx_symbol_exists("${symbol}" "${header}" "${check_variable}")

    if(${check_variable})
      set(found TRUE)
      break()
    endif()
  endforeach()

  set(${result} "${found}" PARENT_SCOPE)
endfunction()

### Package dependencies
find_package(PkgConfig QUIET)

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)

block()
  find_package(Qt6 COMPONENTS Widgets REQUIRED)

  # WORKAROUND: CMake 4 compatibility
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
    get_property(QT_TARGETS DIRECTORY PROPERTY IMPORTED_TARGETS)
    foreach(TARGET_NAME IN LISTS QT_TARGETS)
      if(TARGET_NAME MATCHES "^Qt::")
        map_imported_executable_to_release("${TARGET_NAME}" "")
      endif()
    endforeach()
  endif()

  # WORKAROUND: Remove deprecated AGL.framework when it is not available
  if(APPLE AND TARGET WrapOpenGL::WrapOpenGL)
    get_target_property(WRAP_OPENGL_LINK_LIBRARIES WrapOpenGL::WrapOpenGL INTERFACE_LINK_LIBRARIES)
    list(FILTER WRAP_OPENGL_LINK_LIBRARIES EXCLUDE REGEX "(^|/)AGL\\.framework$|^-framework AGL$")
    set_target_properties(WrapOpenGL::WrapOpenGL PROPERTIES INTERFACE_LINK_LIBRARIES "${WRAP_OPENGL_LINK_LIBRARIES}")
  endif()

  if(NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found via CMake.")
  endif()
endblock()

block(PROPAGATE CURL_FOUND)
  if(APPLE)
    # WORKAROUND: Prefer system curl on macOS
    find_package(CURL MODULE QUIET)

    if(NOT CURL_FOUND)
      find_package(CURL CONFIG QUIET)
    endif()
  else()
    find_package(CURL QUIET)
  endif()

  if(NOT CURL_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(PC_CURL libcurl IMPORTED_TARGET)
    if(PC_CURL_FOUND)
      add_library(CURL::libcurl ALIAS PkgConfig::PC_CURL)
      set(CURL_FOUND TRUE)
    endif()
  endif()

  if(CURL_FOUND)
    target_compile_definitions(BuildOptions INTERFACE PLUGIN_HAVE_CURL)
  endif()
endblock()

block(PROPAGATE OpenCV_FOUND)
  find_package(OpenCV CONFIG QUIET)

  if(NOT OpenCV_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(PC_OPENCV opencv4 IMPORTED_TARGET)
    if(PC_OPENCV_FOUND)
      add_library(OpenCV::opencv_core ALIAS PkgConfig::PC_OPENCV)
      add_library(OpenCV::opencv_imgproc ALIAS PkgConfig::PC_OPENCV)
      set(OpenCV_FOUND TRUE)
    endif()
  endif()

  if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found via CMake or pkg-config.")
  endif()

  if(NOT TARGET OpenCV::opencv_core)
    add_library(OpenCV::opencv_core ALIAS opencv_core)
  endif()

  if(NOT TARGET OpenCV::opencv_imgproc)
    add_library(OpenCV::opencv_imgproc ALIAS opencv_imgproc)
  endif()

  if(TARGET opencv_geometry AND NOT TARGET OpenCV::opencv_geometry)
    add_library(OpenCV::opencv_geometry ALIAS opencv_geometry)
  endif()
endblock()

block(PROPAGATE onnxruntime_FOUND HAVE_ONNXRUNTIME_CUDA_EP HAVE_ONNXRUNTIME_DML_EP HAVE_ONNXRUNTIME_ROCM_EP)
  set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON)

  find_program(Protobuf_PROTOC_EXECUTABLE NAMES protoc HINTS ${CMAKE_PREFIX_PATH} PATH_SUFFIXES tools/protobuf)

  find_package(onnxruntime CONFIG)

  # WORKAROUND: CMake 4 compatibility
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 4.0)
    map_imported_executable_to_release(protobuf::protoc "${Protobuf_PROTOC_EXECUTABLE}")
  endif()

  if(NOT onnxruntime_FOUND AND PkgConfig_FOUND)
    pkg_check_modules(PC_onnxruntime onnxruntime IMPORTED_TARGET)
    if(PC_onnxruntime_FOUND)
      add_library(onnxruntime::onnxruntime ALIAS PkgConfig::PC_onnxruntime)
      set(onnxruntime_FOUND TRUE)
    endif()
  endif()

  if(NOT onnxruntime_FOUND)
    message(FATAL_ERROR "ONNX Runtime not found via CMake or pkg-config.")
  endif()

  if(NOT TARGET onnxruntime::onnxruntime)
    add_library(onnxruntime::onnxruntime ALIAS onnxruntime)
  endif()

  ### Check execution provider availability
  set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)

  cmake_push_check_state(RESET)
  set(CMAKE_REQUIRED_LIBRARIES onnxruntime::onnxruntime)
  set(CMAKE_REQUIRED_QUIET TRUE)

  if(MSVC)
    set(CMAKE_REQUIRED_INCLUDES "${DIRECTML_INCLUDE_DIR}")
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    check_cxx_source_compiles(
      [[
        #if __has_include(<onnxruntime/dml_provider_factory.h>)
        #include <onnxruntime/dml_provider_factory.h>
        #elif __has_include(<dml_provider_factory.h>)
        #include <dml_provider_factory.h>
        #elif __has_include(<onnxruntime/core/providers/dml/dml_provider_factory.h>)
        #include <onnxruntime/core/providers/dml/dml_provider_factory.h>
        #else
        #error "dml_provider_factory.h was not found"
        #endif

        int main()
        {
          auto append_dml2 = &OrtDmlApi::SessionOptionsAppendExecutionProvider_DML2;
          OrtDmlDeviceOptions options{
            OrtDmlPerformancePreference::HighPerformance,
            OrtDmlDeviceFilter::Gpu,
          };
          (void)append_dml2;
          (void)options;
          return 0;
        }
      ]]
      HAVE_ONNXRUNTIME_DML_EP
    )
    set(CMAKE_TRY_COMPILE_TARGET_TYPE EXECUTABLE)

    if(HAVE_ONNXRUNTIME_DML_EP)
      message(STATUS "Building with ONNX Runtime DirectML EP.")
      target_compile_definitions(BuildOptions INTERFACE HAVE_ONNXRUNTIME_DML_EP)
      target_include_directories(BuildOptions INTERFACE "${DIRECTML_INCLUDE_DIR}")
    else()
      message(STATUS "Building without ONNX Runtime DirectML EP.")
    endif()

    if(REQUIRE_ONNXRUNTIME_DML_EP AND NOT HAVE_ONNXRUNTIME_DML_EP)
      message(
        FATAL_ERROR
        "ONNX Runtime DirectML EP with OrtDmlApi::SessionOptionsAppendExecutionProvider_DML2 is required."
      )
    endif()
    unset(CMAKE_REQUIRED_INCLUDES)
  endif()

  check_onnxruntime_execution_provider(
    HAVE_ONNXRUNTIME_CUDA_EP
    OrtSessionOptionsAppendExecutionProvider_CUDA
    onnxruntime_c_api.h
    onnxruntime/onnxruntime_c_api.h
    onnxruntime/core/session/onnxruntime_c_api.h
    cuda_provider_factory.h
    onnxruntime/cuda_provider_factory.h
    onnxruntime/core/providers/cuda/cuda_provider_factory.h
  )

  if(HAVE_ONNXRUNTIME_CUDA_EP)
    message(STATUS "Building with ONNX Runtime CUDA EP.")
    target_compile_definitions(BuildOptions INTERFACE HAVE_ONNXRUNTIME_CUDA_EP)
  else()
    message(STATUS "Building without ONNX Runtime CUDA EP.")
  endif()

  check_onnxruntime_execution_provider(
    HAVE_ONNXRUNTIME_ROCM_EP
    OrtSessionOptionsAppendExecutionProvider_ROCM
    onnxruntime_c_api.h
    onnxruntime/onnxruntime_c_api.h
    onnxruntime/core/session/onnxruntime_c_api.h
    rocm_provider_factory.h
    onnxruntime/rocm_provider_factory.h
    onnxruntime/core/providers/rocm/rocm_provider_factory.h
  )

  if(HAVE_ONNXRUNTIME_ROCM_EP)
    message(STATUS "Building with ONNX Runtime ROCm EP.")
    target_compile_definitions(BuildOptions INTERFACE HAVE_ONNXRUNTIME_ROCM_EP)
  else()
    message(STATUS "Building without ONNX Runtime ROCm EP.")
  endif()

  cmake_pop_check_state()
endblock()

### Plugin main
add_subdirectory(vendor/josuttis-jthread)
add_subdirectory(src/UpdateConfig)

add_library(${CMAKE_PROJECT_NAME} MODULE)

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    BuildOptions
    OBS::libobs
    OBS::obs-frontend-api
    Qt6::Widgets
    onnxruntime::onnxruntime
    UpdateConfig
    OpenCV::opencv_core
    OpenCV::opencv_imgproc
    $<$<TARGET_EXISTS:OpenCV::opencv_geometry>:OpenCV::opencv_geometry>
)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE
    src/plugin-main.cpp
    src/plugin-support.c
    src/UI/AboutDialog.cpp
    src/UI/AboutDialogIntegration.cpp
    src/Resources/resources.qrc
    src/ort-utils/ort-session-utils.cpp
    src/obs-utils/obs-utils.cpp
    src/background-filter.cpp
    src/enhance-filter.cpp
)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES AUTORCC ON)

if(COMMAND collect_licenses)
  set(legal_header "${CMAKE_CURRENT_BINARY_DIR}/legal.hpp")
  collect_licenses("${legal_header}")
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${legal_header}")
  target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
endif()

### Hide symbols
if(APPLE)
  target_link_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/src/exported_symbols_macos.txt"
  )
elseif(MSVC)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/exported_symbols_windows.def")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/src/exported_symbols_linux.map" "-Wl,-Bsymbolic"
  )
endif()

### Install configurations
set(
  EFFECTS_FILES
  data/effects/blend_images.effect
  data/effects/kawase_blur.effect
  data/effects/mask_alpha_filter.effect
)

set(
  LOCALE_FILES
  data/locale/ar-EG.ini
  data/locale/bn-IN.ini
  data/locale/en-US.ini
  data/locale/es-SP.ini
  data/locale/fr-FR.ini
  data/locale/hi-IN.ini
  data/locale/it-IT.ini
  data/locale/ja-JP.ini
  data/locale/ko-KR.ini
  data/locale/pt-BR.ini
  data/locale/ru-RU.ini
  data/locale/ta-IN.ini
  data/locale/tr-TR.ini
  data/locale/zh-CN.ini
)

set(
  MODELS_FILES
  data/models/mediapipe.onnx
  data/models/mediapipe.onnx.license
  data/models/pphumanseg_fp32.onnx
  data/models/pphumanseg_fp32.onnx.license
  data/models/rvm_mobilenetv3_fp32.onnx
  data/models/rvm_mobilenetv3_fp32.onnx.license
  data/models/selfie_multiclass_256x256.onnx
  data/models/selfie_multiclass_256x256.onnx.license
  data/models/selfie_segmentation.onnx
  data/models/selfie_segmentation.onnx.license
  data/models/semantic_guided_llie_180x324.onnx
  data/models/semantic_guided_llie_180x324.onnx.license
  data/models/SINet_Softmax_simple.onnx
  data/models/SINet_Softmax_simple.onnx.license
  data/models/tbefn_fp32.onnx
  data/models/tbefn_fp32.onnx.license
  data/models/tcmonodepth_tcsmallnet_192x320.onnx
  data/models/tcmonodepth_tcsmallnet_192x320.onnx.license
  data/models/uretinex_net_180x320.onnx
  data/models/uretinex_net_180x320.onnx.license
)

if(APPLE)
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES BUNDLE TRUE BUNDLE_EXTENSION "plugin")
  set_source_files_properties(${EFFECTS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/effects")
  set_source_files_properties(${LOCALE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/locale")
  set_source_files_properties(${MODELS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/models")
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${EFFECTS_FILES} ${LOCALE_FILES} ${MODELS_FILES})

  install(
    TARGETS "${CMAKE_PROJECT_NAME}"
    BUNDLE DESTINATION "Library/Application Support/obs-studio/plugins"
    LIBRARY DESTINATION "Library/Application Support/obs-studio/plugins"
  )
elseif(MSVC)
  set(OBS_PLUGIN_BIN_DIR "${CMAKE_PROJECT_NAME}/bin/64bit")
  set(OBS_PLUGIN_DATA_DIR "${CMAKE_PROJECT_NAME}/data")

  if(HAVE_ONNXRUNTIME_DML_EP AND NOT EXISTS "${DIRECTML_DLL}")
    message(FATAL_ERROR "DIRECTML_DLL must point to the DirectML.dll used to build ONNX Runtime.")
  endif()

  install(
    TARGETS "${CMAKE_PROJECT_NAME}"
    RUNTIME DESTINATION "${OBS_PLUGIN_BIN_DIR}"
    LIBRARY DESTINATION "${OBS_PLUGIN_BIN_DIR}"
  )
  install(FILES ${EFFECTS_FILES} DESTINATION "${OBS_PLUGIN_DATA_DIR}/effects")
  install(FILES ${LOCALE_FILES} DESTINATION "${OBS_PLUGIN_DATA_DIR}/locale")
  install(FILES ${MODELS_FILES} DESTINATION "${OBS_PLUGIN_DATA_DIR}/models" OPTIONAL)
  install(
    FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/DirectML/LICENSE.txt"
      "${CMAKE_CURRENT_SOURCE_DIR}/third_party/DirectML/ThirdPartyNotices.txt"
    DESTINATION "${OBS_PLUGIN_DATA_DIR}/licenses/DirectML"
  )
  if(HAVE_ONNXRUNTIME_DML_EP)
    install(FILES "${DIRECTML_DLL}" DESTINATION "${OBS_PLUGIN_BIN_DIR}")
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(OBS_PLUGIN_DATA_DIR "${CMAKE_INSTALL_DATAROOTDIR}/obs/obs-plugins/${CMAKE_PROJECT_NAME}")
  set(OBS_PLUGIN_DOC_DIR "${CMAKE_INSTALL_DATAROOTDIR}/doc/${CMAKE_PROJECT_NAME}")
  set(OBS_PLUGIN_LIB_DIR "${CMAKE_INSTALL_LIBDIR}/obs-plugins")

  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")

  install(
    TARGETS "${CMAKE_PROJECT_NAME}"
    BUNDLE DESTINATION "${OBS_PLUGIN_LIB_DIR}"
    LIBRARY DESTINATION "${OBS_PLUGIN_LIB_DIR}"
  )
  install(FILES ${EFFECTS_FILES} DESTINATION "${OBS_PLUGIN_DATA_DIR}/effects")
  install(FILES ${LOCALE_FILES} DESTINATION "${OBS_PLUGIN_DATA_DIR}/locale")
  install(FILES ${MODELS_FILES} DESTINATION "${OBS_PLUGIN_DATA_DIR}/models" OPTIONAL)
  install(FILES LICENSE DESTINATION "${OBS_PLUGIN_DOC_DIR}" RENAME copyright)

  # User plugin
  install(TARGETS "${CMAKE_PROJECT_NAME}" LIBRARY DESTINATION "bin/64bit" COMPONENT UserPlugin EXCLUDE_FROM_ALL)
  install(FILES ${EFFECTS_FILES} DESTINATION "data/effects" COMPONENT UserPlugin EXCLUDE_FROM_ALL)
  install(FILES ${LOCALE_FILES} DESTINATION "data/locale" COMPONENT UserPlugin EXCLUDE_FROM_ALL)
  install(FILES ${MODELS_FILES} DESTINATION "data/models" COMPONENT UserPlugin EXCLUDE_FROM_ALL OPTIONAL)
else()
  message("Installing on this platform is not supported officially.")
endif()

### CPack configurations
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PLUGIN_VERSION}")
set(CPACK_PACKAGE_CONTACT "${PLUGIN_AUTHOR} <${PLUGIN_EMAIL}>")

set(CPACK_ARCHIVE_THREADS 0)
set(CPACK_SET_DESTDIR TRUE)
set(CPACK_VERBATIM_VARIABLES TRUE)

set(CPACK_DEBIAN_COMPRESSION_LEVEL 19)
set(CPACK_DEBIAN_COMPRESSION_TYPE zstd)
set(CPACK_DEBIAN_DEBUGINFO_PACKAGE TRUE)
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS TRUE)

include(CPack) # This line must be placed after all the package configurations

### Testing
if(BUILD_TESTING)
  enable_testing()
  set(PLUGIN_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
  add_subdirectory(tests)
endif()
