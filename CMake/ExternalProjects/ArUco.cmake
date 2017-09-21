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
# aruco - external project for tracking AR markers.
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED aruco_DIR AND NOT EXISTS ${aruco_DIR})
  message(FATAL_ERROR "aruco_DIR variable is defined but corresponds to non-existing directory \"${aruco_DIR}\".")
endif()

if(BUILD_NiftyIGI)

  set(version "1.3.0")
  set(location "${NIFTK_EP_TARBALL_LOCATION}/aruco-${version}.tar.gz")
  set(depends OpenCV)

  niftkMacroDefineExternalProjectVariables(ArUco ${version} ${location} "${depends}")

  if(NOT DEFINED aruco_DIR)

    ExternalProject_Add(${proj}
      LIST_SEPARATOR ^^
      PREFIX ${proj_CONFIG}
      SOURCE_DIR ${proj_SOURCE}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${proj_LOCATION}
      URL_MD5 ${proj_CHECKSUM}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${EP_COMMON_ARGS}
        -DCMAKE_PREFIX_PATH:PATH=${NifTK_PREFIX_PATH}
        -DOpenCV_DIR:PATH=${OpenCV_DIR}
        -DCMAKE_DEBUG_POSTFIX:STRING=
      CMAKE_CACHE_ARGS
        ${EP_COMMON_CACHE_ARGS}
      CMAKE_CACHE_DEFAULT_ARGS
        ${EP_COMMON_CACHE_DEFAULT_ARGS}
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(aruco_DIR ${proj_INSTALL})

    mitkFunctionInstallExternalCMakeProject(${proj})

    message("SuperBuild loading ArUco from ${aruco_DIR}")

  else(NOT DEFINED aruco_DIR)

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif(NOT DEFINED aruco_DIR)

endif()


