# -*- coding: utf-8 -*-
# Copyright (C) 2012-2013 MUJIN Inc. <rosen.diankov@mujin.co.jp>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

include_directories(${CURL_INCLUDE_DIRS} ${LOG4CXX_INCLUDEDIR})
link_directories(${MUJINWEBSTACKCLIENT_LINK_DIRS})

set(SOURCE_FILES
  boost_assertion_failed.cpp
  mujinjson.cpp
  mujindefinitions.cpp
  utf8.h
  common.h
  common.cpp
  logging.h
  webstackclient.cpp )

add_library(libmujinwebstackclient SHARED ${SOURCE_FILES})
if( EXTRA_MSVC_DEPEND )
  message(STATUS "adding msvc_boost dependency")
  add_dependencies(libmujinwebstackclient ${EXTRA_MSVC_DEPEND})
endif()

set_target_properties(libmujinwebstackclient PROPERTIES
  COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} ${Boost_CFLAGS} -DMUJINWEBSTACKCLIENT_DLL_EXPORTS -DMUJINWEBSTACKCLIENT_DLL"
  LINK_FLAGS ""
  OUTPUT_NAME mujinwebstackclient${MUJINWEBSTACKCLIENT_LIBRARY_SUFFIX}
  SOVERSION 0 # always have it 0 since we're including the soversion as part of the library name
  VERSION ${MUJINWEBSTACKCLIENT_VERSION}
  CLEAN_DIRECT_OUTPUT 1
  )

target_link_libraries(libmujinwebstackclient ${CURL_LIBRARIES} ${Boost_THREAD_LIBRARY} ${Boost_DATE_TIME_LIBRARY}  ${EXTRA_LIBRARIES} ${libzmq_LIBRARIES} ${LOG4CXX_LIBRARIES})
if( MSVC )
  install(TARGETS libmujinwebstackclient RUNTIME DESTINATION bin LIBRARY DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX})
else()
  install(TARGETS libmujinwebstackclient DESTINATION lib${LIB_SUFFIX})
endif()

if( OPT_BUILD_STATIC )
  # visual studio needs static lib built
  if( MSVC )
    # static version needs to have different name
    set(LIBMUJINWEBSTACKCLIENT_NAME libmujinwebstackclient${MUJINWEBSTACKCLIENT_LIBRARY_SUFFIX})
  else()
    set(LIBMUJINWEBSTACKCLIENT_NAME mujinwebstackclient${MUJINWEBSTACKCLIENT_LIBRARY_SUFFIX})
  endif()

  add_library(libmujinwebstackclient_static STATIC ${SOURCE_FILES})
  set_target_properties(libmujinwebstackclient_static PROPERTIES
    OUTPUT_NAME ${LIBMUJINWEBSTACKCLIENT_NAME}
    SOVERSION 0 # always have it 0 since we're including the soversion as part of the library name
    VERSION ${MUJINWEBSTACKCLIENT_VERSION}
    CLEAN_DIRECT_OUTPUT 1
    COMPILE_FLAGS "${EXTRA_COMPILE_FLAGS} ${Boost_CFLAGS} -DMUJINWEBSTACKCLIENT_DLL_EXPORTS -DMUJINWEBSTACKCLIENT_DLL"
    LINK_FLAGS "")
  
  target_link_libraries(libmujinwebstackclient_static ${CURL_LIBRARIES} ${EXTRA_LIBRARIES} ${libzmq_LIBRARIES} ${libzmq_LIBRARIES} ${LOG4CXX_LIBRARIES})
  install(TARGETS libmujinwebstackclient_static DESTINATION lib${LIB_SUFFIX})
endif()
