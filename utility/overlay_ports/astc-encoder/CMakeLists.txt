##################################################    Project     ##################################################
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project               (astc-encoder VERSION 2.0 LANGUAGES CXX)

set_property          (GLOBAL PROPERTY USE_FOLDERS ON)

set                   (CMAKE_CXX_STANDARD                14)
set                   (CMAKE_CXX_VISIBILITY_PRESET       hidden)
set                   (CMAKE_VISIBILITY_INLINES_HIDDEN   1)

##################################################    Utility     ##################################################
if(MSVC)
  # MSVC:
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX /fp:precise /Zi /D_Console /D_CRT_SECURE_NO_WARNINGS")
  # When not testing the next line should be: set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /RTC1 /Od /D_DEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd /RTC1 /Od /D_DEBUG")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GF /MT /GS- /O2 /LTCG")
else()
  # GCC:
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3 -flto")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0 -g -DNDEBUG")
endif()

##################################################    OPTIONS     ##################################################
option(BUILD_SHARED_LIBS "Build the shared library (otherwise static)" OFF)

set(APP astcenc CACHE STRING "Configure the application to build (astcenc, astcdec)")
#   * astcenc - a full compressor/decompressor
#   * astcdec - a decompressor without compression support
set(VEC avx2 CACHE STRING "Configure the SIMD ISA and intrinsics support (neon, sse2, sse4.2, avx2)")
#   * neon   - compile for Arm aarch64 + NEON
#   * sse2   - allow use of x86-64 + sse2
#   * sse4.2 - allow use of x86-64 + sse4.2 and popcnt
#   * avx2   - allow use of x86-64 + avx2, sse4.2, and popcnt
set(BUILD debug CACHE STRING "Configure the build type (release, debug)")
#   * release - build a fully optimized build without symbols
#   * debug   - build an unoptimized build with symbols

##################################################    Sources     ##################################################
set(SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_averages_and_directions.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_block_sizes2.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_color_quantize.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_color_unquantize.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_compress_symbolic.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_compute_variance.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_decompress_symbolic.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_encoding_choice_error.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_entry.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_find_best_partitioning.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_ideal_endpoints_and_weights.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_integer_sequence.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_kmeans_partitioning.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_mathlib.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_mathlib_softfloat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_partition_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_percentile_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_pick_best_endpoint_format.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_platform_isa_detection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_quantization.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_symbolic_physical.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_weight_align.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_weight_quant_xfer_tables.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_error_metrics.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_image.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_image_load_store.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_platform_dependents.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_toplevel.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_toplevel_help.cpp
)

set(EXTERNAL_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/stb_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/stb_image_write.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/tinyexr.h
)

set(HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenc_mathlib.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/astcenccli_internal.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/stb_image.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/stb_image_write.h
  ${CMAKE_CURRENT_SOURCE_DIR}/Source/tinyexr.h
)

#################################################    Functions     #################################################
# Add compiler-specific instructions for the selected instruction set (AVX2, SSE4.2, SSE2, Neon)
# (No native CMake-support for this)
function (add_vec INSTRUCTION_FORMAT)
  if (${INSTRUCTION_FORMAT} STREQUAL avx2)
    if(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2" PARENT_SCOPE)
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -mavx2 -mpopcnt" PARENT_SCOPE)
    endif()

  elseif (${INSTRUCTION_FORMAT} STREQUAL sse4.2)
    if(MSVC)
      # Do nothing
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -msse4.2 -mpopcnt" PARENT_SCOPE)
    endif()
  
  elseif (${INSTRUCTION_FORMAT} STREQUAL sse2)
    if(MSVC)
      # Do nothing
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpmath=sse -msse2" PARENT_SCOPE)
    endif()
  endif()
endfunction(add_vec)

function(set_max_warning_level)
  if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W3" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    endif()
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror -Werror=shadow -Wdouble-promotion")
  endif()
endfunction(set_max_warning_level)

##############################################    Input Validation     #############################################
# Check if APP = astcenc/astcdec and patch CXX flags
if (${APP} STREQUAL astcenc)
  # Do nothing
elseif (${APP} STREQUAL astcdec)
  list(APPEND PROJECT_COMPILE_DEFINITIONS -DASTCENC_DECOMPRESS_ONLY)
else()
  message(FATAL_ERROR "Unsupported app, use APP=astcenc/astcdec")
endif()

# Check if VEC = neon/sse2/sse4.2/avx2 and patch CXX flags
if (${VEC} STREQUAL neon)
  # NEON is on by default; no enabled needed
  list(APPEND PROJECT_COMPILE_DEFINITIONS -DASTCENC_SSE=0 -DASTCENC_AVX=0 -DASTCENC_POPCNT=0 -DASTCENC_VECALIGN=16)
elseif (${VEC} STREQUAL sse2)
  list(APPEND PROJECT_COMPILE_DEFINITIONS -DASTCENC_SSE=20 -DASTCENC_AVX=0 -DASTCENC_POPCNT=0 -DASTCENC_VECALIGN=16)
  add_vec("sse2")
elseif (${VEC} STREQUAL sse4.2)
  list(APPEND PROJECT_COMPILE_DEFINITIONS -DASTCENC_SSE=42 -DASTCENC_AVX=0 -DASTCENC_POPCNT=1 -DASTCENC_VECALIGN=16)
  add_vec("sse4.2")
elseif (${VEC} STREQUAL avx2)
  list(APPEND PROJECT_COMPILE_DEFINITIONS -DASTCENC_SSE=42 -DASTCENC_AVX=2 -DASTCENC_POPCNT=1 -DASTCENC_VECALIGN=32)
  add_vec("avx2")
else()
  message(FATAL_ERROR "Unsupported VEC target, use VEC=neon/sse2/sse4.2/avx2")
endif()

# Disable necessary optimizations and warnings for third-party source files
set(CXX_FLAGS_EXTERNAL ${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-double-promotion -fno-strict-aliasing)

##################################################    Targets     ##################################################
if(MSVC)
  add_library               ("${PROJECT_NAME}" STATIC ${SOURCES} ${EXTERNAL_SOURCES} ${HEADERS})
  target_include_directories("${PROJECT_NAME}" PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include> PRIVATE Source)
  target_compile_definitions("${PROJECT_NAME}" PUBLIC ${PROJECT_COMPILE_DEFINITIONS})
  set_target_properties     ("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
else()
  # Dependencies
  add_library(stb_image OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h)
  target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
  target_compile_definitions(stb_image PUBLIC -DSTB_IMAGE_IMPLEMENTATION -DSTBI_NO_PSD -DSTBI_NO_GIF -DSTBI_NO_PIC -DSTBI_NO_PNM)
  target_compile_options(stb_image PUBLIC ${CXX_FLAGS_EXTERNAL})
  set_target_properties(stb_image PROPERTIES LINKER_LANGUAGE CXX)


  add_library(stb_image_write OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write.h)
  target_include_directories(stb_image_write PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
  target_compile_definitions(stb_image_write PUBLIC -DSTB_IMAGE_WRITE_IMPLEMENTATION)
  target_compile_options(stb_image_write PUBLIC ${CXX_FLAGS_EXTERNAL})
  set_target_properties(stb_image_write PROPERTIES LINKER_LANGUAGE CXX)

  add_library(tinyexr OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/tinyexr.h)
  target_include_directories(tinyexr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
  target_compile_definitions(tinyexr PUBLIC -DTINYEXR_IMPLEMENTATION)
  target_compile_options(tinyexr PUBLIC ${CXX_FLAGS_EXTERNAL})
  set_target_properties(tinyexr PROPERTIES LINKER_LANGUAGE CXX)

  # astc-encoder
  add_library("${PROJECT_NAME}" ${SOURCES} ${HEADERS} $<TARGET_OBJECTS:stb_image> $<TARGET_OBJECTS:stb_image_write> $<TARGET_OBJECTS:tinyexr>)
  target_include_directories("${PROJECT_NAME}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)
  target_link_libraries("${PROJECT_NAME}" PUBLIC ${PROJECT_LIBRARIES})
  target_compile_definitions("${PROJECT_NAME}" PUBLIC ${PROJECT_COMPILE_DEFINITIONS})
  set_target_properties     ("${PROJECT_NAME}" PROPERTIES LINKER_LANGUAGE CXX)
endif()

##################################################  Installation  ##################################################
if(MSVC)
  install(TARGETS "${PROJECT_NAME}" EXPORT "${PROJECT_NAME}-config"
          ARCHIVE DESTINATION lib
          LIBRARY DESTINATION lib
          RUNTIME DESTINATION bin
  )
  install(DIRECTORY Source/ DESTINATION include
          FILES_MATCHING PATTERN "*.h"
  )
  install(EXPORT "${PROJECT_NAME}-config" DESTINATION cmake)
  export (TARGETS "${PROJECT_NAME}" FILE "${PROJECT_NAME}-config.cmake")
endif()
