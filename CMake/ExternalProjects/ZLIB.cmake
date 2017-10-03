#/*============================================================================
#
#  NifTK: A software platform for medical image computing.
#
#  Copyright (c) University College London (UCL). All rights reserved.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  See LICENSE.txt in the top level directory for details.
#
#============================================================================*/

#------------------------------------------------------------------
# ZLIB
#------------------------------------------------------------------
if(MITK_USE_ZLIB)

  set(version "66a75305")
  set(location "${NIFTK_EP_TARBALL_LOCATION}/zlib-${version}.tar.gz")
  set(depends "")

  niftkMacroDefineExternalProjectVariables(ZLIB ${version} ${location} "${depends}")

  if(NOT DEFINED ZLIB_DIR)

    set(additional_cmake_args )
    if(CTEST_USE_LAUNCHERS)
      list(APPEND additional_cmake_args
        "-DCMAKE_PROJECT_${proj}_INCLUDE:FILEPATH=${CMAKE_ROOT}/Modules/CTestUseLaunchers.cmake"
      )
    endif()

    # Using the ZLIB from CTK:
    # https://github.com/commontk/zlib
    ExternalProject_Add(${proj}
      LIST_SEPARATOR ^^
      PREFIX ${proj_CONFIG}
      SOURCE_DIR ${proj_SOURCE}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${proj_LOCATION}
      URL_MD5 ${proj_CHECKSUM}
      CMAKE_ARGS
        ${EP_COMMON_ARGS}
        ${additional_cmake_args}
      CMAKE_CACHE_ARGS
        ${EP_COMMON_CACHE_ARGS}
        -DBUILD_SHARED_LIBS:BOOL=OFF
        -DZLIB_MANGLE_PREFIX:STRING=mitk_zlib_
        -DZLIB_INSTALL_INCLUDE_DIR:STRING=include/mitk_zlib
      CMAKE_CACHE_DEFAULT_ARGS
        ${EP_COMMON_CACHE_DEFAULT_ARGS}
      DEPENDS ${proj_DEPENDENCIES}
      )

    set(ZLIB_DIR ${proj_INSTALL})
    set(ZLIB_INCLUDE_DIR ${ZLIB_DIR}/include/mitk_zlib)
    if(WIN32)
      set(ZLIB_LIBRARY ${ZLIB_DIR}/lib/zlib.lib)
    else()
      set(ZLIB_LIBRARY ${ZLIB_DIR}/lib/libzlib.a)
    endif()

    mitkFunctionInstallExternalCMakeProject(${proj})
    message("SuperBuild loading ZLIB from ${ZLIB_DIR}")
    mark_as_advanced(ZLIB_LIBRARY)
  else()
    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  endif()
endif()

