# SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>
#
# SPDX-License-Identifier: Apache-2.0

# Keep the heavyweight Windows ONNX Runtime build inputs in one file so the
# installed-prefix cache can be invalidated from the exact configuration.
set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
set(CMAKE_DISABLE_PRECOMPILE_HEADERS ON CACHE BOOL "" FORCE)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
set(onnxruntime_BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)
set(onnxruntime_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(onnxruntime_DISABLE_RTTI OFF CACHE BOOL "" FORCE)
set(onnxruntime_ENABLE_DELAY_LOADING_WIN_DLLS OFF CACHE BOOL "" FORCE)
set(onnxruntime_REDUCED_OPS_BUILD OFF CACHE BOOL "" FORCE)
set(onnxruntime_RUN_ONNX_TESTS OFF CACHE BOOL "" FORCE)
set(onnxruntime_USE_DML ON CACHE BOOL "" FORCE)
set(onnxruntime_USE_VCPKG ON CACHE BOOL "" FORCE)
