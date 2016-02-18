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

message(STATUS "Configuring cimg")

set(CIMG_VERSION 169)

string(REPLACE "." "_" BOOST_VERSION_UNDERSCORE ${CIMG_VERSION})
set(CIMG_DIR cimg)
set(CIMG_PATH ${THIRD_PARTY_DIR}/${CIMG_DIR})

# Set up build steps
include(ExternalProject)
ExternalProject_Add(
    cimg
    PREFIX ${CIMG_PATH}
    URL https://github.com/dtschump/CImg/archive/v.${CIMG_VERSION}.zip
    TIMEOUT 600
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    BUILD_IN_SOURCE ON
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_UPDATE ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_TEST ON
    LOG_INSTALL ON
)

ExternalProject_Get_Property(cimg SOURCE_DIR)
set(CIMG_INCLUDE_DIR ${SOURCE_DIR})
