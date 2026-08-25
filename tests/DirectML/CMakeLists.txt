# SPDX-FileCopyrightText: 2026 obs-backgroundremoval contributors
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(DirectMLMediaPipeSmokeTest DirectMLMediaPipeSmokeTest.cpp)
target_link_libraries(DirectMLMediaPipeSmokeTest PRIVATE BuildOptions onnxruntime::onnxruntime)

add_custom_command(
  TARGET DirectMLMediaPipeSmokeTest
  POST_BUILD
  COMMAND
    "${CMAKE_COMMAND}" -E copy_if_different "${DIRECTML_DLL}"
    "$<TARGET_FILE_DIR:DirectMLMediaPipeSmokeTest>/DirectML.dll"
)

add_test(
  NAME DirectMLMediaPipeSmokeTest
  COMMAND DirectMLMediaPipeSmokeTest "${CMAKE_SOURCE_DIR}/data/models/mediapipe.onnx"
)
set_tests_properties(DirectMLMediaPipeSmokeTest PROPERTIES LABELS "directml;gpu" TIMEOUT 120)
