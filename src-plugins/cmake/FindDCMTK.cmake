# - find DCMTK libraries and applications
#
#  DCMTK_INCLUDE_DIRS   - Directories to include to use DCMTK
#  DCMTK_LIBRARIES     - Files to link against to use DCMTK
#  DCMTK_FOUND         - If false, don't try to use DCMTK
#  DCMTK_DIR           - (optional) Source directory for DCMTK
#
# DCMTK_DIR can be used to make it simpler to find the various include
# directories and compiled libraries if you've just compiled it in the
# source tree. Just set it to the root of the tree where you extracted
# the source (default to /usr/include/dcmtk/)

#=============================================================================
# Copyright 2004-2009 Kitware, Inc.
# Copyright 2009-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
# Copyright 2010 Thomas Sondergaard <ts@medical-insight.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

#
# Written for VXL by Amitha Perera.
# Upgraded for GDCM by Mathieu Malaterre.
# Modified for EasyViz by Thomas Sondergaard.
#

if(NOT DCMTK_FOUND AND NOT DCMTK_DIR)
  set(DCMTK_DIR
    "/usr"
    CACHE
    PATH
    "Root of DCMTK source or installation tree")
  mark_as_advanced(DCMTK_DIR)
endif()


foreach(lib
    dcmdata
    dcmimage
    dcmimgle
    dcmjpeg
    dcmnet
    dcmpstat
    dcmqrdb
    dcmsign
    dcmsr
    dcmtls
    ijg12
    ijg16
    ijg8
    oflog
    ofstd)
    

foreach(config Debug Release RelWithDebInfo MinSizeRel)
	string(TOUPPER ${config} config_upper)

	find_library(DCMTK_${lib}_${config}_LIBRARY
	${lib}
	PATHS
        ${DCMTK_DIR}/${config}
        ${DCMTK_DIR}/${config}/lib
        ${DCMTK_DIR}/${lib}/libsrc
        ${DCMTK_DIR}/${lib}/libsrc/${config}
        ${DCMTK_DIR}/${lib}/${config}
        ${DCMTK_DIR}/lib)
        
    if (DCMTK_${lib}_${config}_LIBRARY)
        get_filename_component(lib_filename ${DCMTK_${lib}_${config}_LIBRARY} NAME_WE)
        get_filename_component(lib_directory ${DCMTK_${lib}_${config}_LIBRARY} PATH)
            
        if (NOT DCMTK_${lib}_ADDED)
            set(DCMTK_${lib}_ADDED ON)
            add_library(${lib_filename} STATIC IMPORTED)
            list(APPEND DCMTK_LIBRARIES ${lib_filename})
        endif()
        list(APPEND DCMTK_${lib}_LIBRARIES ${DCMTK_${lib}_${config}_LIBRARY})
        
        set_property(TARGET ${lib_filename} APPEND PROPERTY IMPORTED_CONFIGURATIONS ${config_upper})
        
        set_target_properties(${lib_filename} PROPERTIES
          IMPORTED_LOCATION_${config_upper} ${DCMTK_${lib}_${config}_LIBRARY}
          )
    endif()
    

  mark_as_advanced(DCMTK_${lib}_${config}_LIBRARY)

endforeach()

endforeach()


# foreach(libname TIFF ZLIB LIBICONV JPEG)
#   find_package(${libname} REQUIRED)
#   foreach(lib ${${libname}_LIBRARIES})
#     list(APPEND DCMTK_LIBRARIES ${lib})
#   endforeach()
# endforeach()

set(DCMTK_config_TEST_HEADER osconfig.h)
set(DCMTK_dcmdata_TEST_HEADER dctypes.h)
set(DCMTK_dcmimage_TEST_HEADER dicoimg.h)
set(DCMTK_dcmimgle_TEST_HEADER dcmimage.h)
set(DCMTK_dcmjpeg_TEST_HEADER djdecode.h)
set(DCMTK_dcmnet_TEST_HEADER assoc.h)
set(DCMTK_dcmpstat_TEST_HEADER dcmpstat.h)
set(DCMTK_dcmqrdb_TEST_HEADER dcmqrdba.h)
set(DCMTK_dcmsign_TEST_HEADER sicert.h)
set(DCMTK_dcmsr_TEST_HEADER dsrtree.h)
set(DCMTK_dcmtls_TEST_HEADER tlslayer.h)
set(DCMTK_ofstd_TEST_HEADER ofstdinc.h)

set(DCMTK_TOP_INCLUDE_DIR "")

foreach(dir
    config
    dcmdata
    dcmimage
    dcmimgle
    dcmjpeg
    dcmnet
    dcmpstat
    dcmqrdb
    dcmsign
    dcmsr
    dcmtls
    ofstd)

foreach(config Debug Release RelWithDebInfo MinSizeRel)

    find_path(DCMTK_${dir}_INCLUDE_DIR
    ${DCMTK_${dir}_TEST_HEADER}
    PATHS
    ${DCMTK_DIR}/${dir}/include
    ${DCMTK_DIR}/${dir}
    ${DCMTK_DIR}/include/${dir}
    ${DCMTK_DIR}/include/dcmtk/${dir}
    ${DCMTK_DIR}/${dir}/include/dcmtk/${dir}
    ${DCMTK_DIR}/${config}/${dir}/include
    ${DCMTK_DIR}/${config}/${dir}
    ${DCMTK_DIR}/${config}/include/${dir}
    ${DCMTK_DIR}/${config}/include/dcmtk/${dir}
    ${DCMTK_DIR}/${config}/${dir}/include/dcmtk/${dir}
    )
  mark_as_advanced(DCMTK_${dir}_INCLUDE_DIR)

  if(DCMTK_${dir}_INCLUDE_DIR)
    if(NOT DCMTK_TOP_INCLUDE_DIR)
      #
      # since DCMTK client programs often use #include "dcmtk/xxx/xxx.h"
      # add in the top level include directory.
      get_filename_component(DCMTK_TOP_INCLUDE_DIR
        ${DCMTK_${dir}_INCLUDE_DIR} PATH)
      get_filename_component(DCMTK_TOP_INCLUDE_DIR
        ${DCMTK_TOP_INCLUDE_DIR} PATH)
      list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_TOP_INCLUDE_DIR})
    endif()
    list(APPEND
      DCMTK_INCLUDE_DIRS
      ${DCMTK_${dir}_INCLUDE_DIR})
    break()
  endif()
 
endforeach() 
  
endforeach()


if(WIN32)
  list(APPEND DCMTK_LIBRARIES netapi32 wsock32)
endif()

if(DCMTK_ofstd_INCLUDE_DIR)
  get_filename_component(DCMTK_dcmtk_INCLUDE_DIR
    ${DCMTK_ofstd_INCLUDE_DIR}
    PATH
    CACHE)
  list(APPEND DCMTK_INCLUDE_DIRS ${DCMTK_dcmtk_INCLUDE_DIR})
  mark_as_advanced(DCMTK_dcmtk_INCLUDE_DIR)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DCMTK DEFAULT_MSG
  DCMTK_config_INCLUDE_DIR
  DCMTK_ofstd_INCLUDE_DIR
  DCMTK_ofstd_LIBRARIES
  DCMTK_dcmdata_INCLUDE_DIR
  DCMTK_dcmdata_LIBRARIES
  DCMTK_dcmimgle_INCLUDE_DIR
  DCMTK_dcmimgle_LIBRARIES)

# Compatibility: This variable is deprecated
set(DCMTK_INCLUDE_DIR ${DCMTK_INCLUDE_DIRS})

foreach(executable dcmdump dcmdjpeg dcmdrle)
  string(TOUPPER ${executable} EXECUTABLE)
  find_program(DCMTK_${EXECUTABLE}_EXECUTABLE ${executable} ${DCMTK_DIR}/bin)
  mark_as_advanced(DCMTK_${EXECUTABLE}_EXECUTABLE)
endforeach()
