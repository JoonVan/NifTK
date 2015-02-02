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

#-----------------------------------------------------------------------------
# OpenCV
#-----------------------------------------------------------------------------


# Sanity checks
if(DEFINED OpenCV_DIR AND NOT EXISTS ${OpenCV_DIR})
  message(FATAL_ERROR "OpenCV_DIR variable is defined but corresponds to non-existing directory")
endif()

if(BUILD_IGI)

  niftkMacroDefineExternalProjectVariables(OpenCV ${NIFTK_VERSION_OpenCV})

  if(NOT DEFINED OpenCV_DIR)

    set(additional_cmake_args
      -DBUILD_opencv_java:BOOL=OFF
    )

    set(OpenCV_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/CMake/CMakeExternals/EmptyFileForPatching.dummy -P ${CMAKE_SOURCE_DIR}/CMake/CMakeExternals/PatchOpenCV-2.4.8.2.cmake)

    niftkMacroGetChecksum(NIFTK_CHECKSUM_OpenCV ${NIFTK_LOCATION_OpenCV})

    ExternalProject_Add(${proj}
      SOURCE_DIR ${proj_SOURCE}
      PREFIX ${proj_CONFIG}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${NIFTK_LOCATION_OpenCV}
      URL_MD5 ${NIFTK_CHECKSUM_OpenCV}
      UPDATE_COMMAND  ""
      INSTALL_COMMAND ""
      PATCH_COMMAND ${OpenCV_PATCH_COMMAND}
      CMAKE_GENERATOR ${GEN}
      CMAKE_CACHE_ARGS
        ${EP_COMMON_ARGS}
        -DBUILD_opencv_core:BOOL=ON
        -DBUILD_opencv_calib3d:BOOL=ON
        -DBUILD_opencv_features2d:BOOL=ON
        -DBUILD_opencv_imgproc:BOOL=ON
        -DBUILD_opencv_video:BOOL=ON
        -DBUILD_opencv_python:BOOL=OFF
        -DBUILD_opencv_ts:BOOL=OFF
        -DBUILD_opencv_java:BOOL=OFF
        -DBUILD_opencv_nonfree:BOOL=${OPENCV_WITH_NONFREE}
        -DBUILD_DOCS:BOOL=OFF
        -DBUILD_TESTS:BOOL=OFF
        -DBUILD_EXAMPLES:BOOL=OFF
        -DBUILD_DOXYGEN_DOCS:BOOL=OFF
        -DBUILD_PERF_TESTS:BOOL=OFF
        -DWITH_CUDA:BOOL=${OPENCV_WITH_CUDA}
        -DWITH_QT:BOOL=OFF
        -DWITH_EIGEN:BOOL=OFF
        -DWITH_FFMPEG:BOOL=${OPENCV_WITH_FFMPEG}
        -DADDITIONAL_C_FLAGS:STRING=${OPENCV_ADDITIONAL_C_FLAGS}
        -DADDITIONAL_CXX_FLAGS:STRING=${OPENCV_ADDITIONAL_CXX_FLAGS}
        ${additional_cmake_args}
      DEPENDS ${proj_DEPENDENCIES}
    )
    set(OpenCV_DIR ${proj_BUILD})
    message("SuperBuild loading OpenCV from ${OpenCV_DIR}")

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()

endif()
