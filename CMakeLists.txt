# The MIT License (MIT)
#
# Copyright (c) 2015 Jeremy Letang
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# min config
cmake_minimum_required (VERSION 2.8.1)

project (preview)

set (PREVIEW_VERSION_MAJOR 0)
set (PREVIEW_VERSION_MINOR 2)
set (PREVIEW_VERSION_PATCH 0)

set (CIMG_CCFLAGS "-Dcimg_display=0 -Dcimg_verbosity=0")

# Flags to enable native support for JPEG, TIFF, PNG
set (CIMG_JPEG_CCFLAGS -Dcimg_use_jpeg)
set (CIMG_TIFF_CCFLAGS -Dcimg_use_tiff)
set (CIMG_PNG_CCFLAGS -Dcimg_use_png)
set (CIMG_ZLIB_CCFLAGS -Dcimg_use_zlib)

# try to find images formats supports
find_package (PkgConfig)
find_package (X11)
find_package (Threads REQUIRED)
find_package (PNG)
find_package (ZLIB)
find_package (JPEG)
find_package (TIFF)

set_property (GLOBAL PROPERTY USE_FOLDERS ON)
set (CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM ON)
set (THIRD_PARTY_DIR "${CMAKE_BINARY_DIR}/third_party")

# download cimg
include (cimg)
include_directories (${CIMG_INCLUDE_DIR})

# if tiff library found add it to the build
if(TIFF_FOUND)
  get_filename_component(TIFF_LIB_DIRS ${TIFF_LIBRARIES} PATH)
  set (CIMG_CCFLAGS "${CIMG_CCFLAGS} ${CIMG_TIFF_CCFLAGS}")
  link_directories(${TIFF_LIB_DIRS})
  include_directories(${TIFF_INCLUDE_DIR})
  set (CIMG_DEPENDENCIES ${CIMG_DEPENDENCIES} ${TIFF_LIBRARIES})
endif()

# if zlib+png library found add it to the build
if (ZLIB_FOUND)
    set (CIMG_CCFLAGS "${CIMG_CCFLAGS} ${CIMG_ZLIB_CCFLAGS}")
    link_directories (${ZLIB_LIB_DIRS})
    include_directories (${ZLIB_INCLUDE_DIR})
    set (CIMG_DEPENDENCIES ${CIMG_DEPENDENCIES} ${ZLIB_LIBRARIES})

    # PNG requires ZLIB
    if(PNG_FOUND)
        set (CIMG_CCFLAGS "${CIMG_CCFLAGS} ${CIMG_PNG_CCFLAGS}")
        link_directories (${PNG_LIB_DIRS})
        include_directories (${PNG_INCLUDE_DIR} )
        set (CIMG_DEPENDENCIES ${CIMG_DEPENDENCIES} ${PNG_LIBRARIES} )
    endif ()
endif ()

# if jpeg library found add it to the build
if(JPEG_FOUND)
  get_filename_component (JPEG_LIB_DIRS ${JPEG_LIBRARIES} PATH)
  set (CIMG_CCFLAGS "${CIMG_CCFLAGS} ${CIMG_JPEG_CCFLAGS}")
  link_directories (${JPEG_LIB_DIRS})
  include_directories (${JPEG_INCLUDE_DIR})
  set (CIMG_DEPENDENCIES ${CIMG_DEPENDENCIES} ${JPEG_LIBRARIES})
endif()

# basic flags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -W -Wall -Wextra -O3") #enable C++14
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# add cimg_ccflags
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CIMG_CCFLAGS}")

configure_file (
    "${PROJECT_SOURCE_DIR}/version.h.in"
    "${PROJECT_BINARY_DIR}/version.h"
)

include_directories(${PROJECT_BINARY_DIR})

add_executable (preview main.cpp)

# linkg requireds libraries depending on the cimg_flags
target_link_libraries (preview ${CIMG_DEPENDENCIES} pthread)
# wait for cimg to be downloaded
add_dependencies (preview cimg)
