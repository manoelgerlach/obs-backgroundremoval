# SPDX-FileCopyrightText: 2026 Kaito Udagawa <umireon@kaito.tokyo>
#
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(UI)

if(MSVC AND HAVE_ONNXRUNTIME_DML_EP)
  add_subdirectory(DirectML)
endif()

if(APPLE OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_subdirectory(UpdateConfig)
endif()
