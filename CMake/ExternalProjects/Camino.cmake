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
# Camino - external project for diffusion imaging.
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED camino_DIR AND NOT EXISTS ${camino_DIR})
  message(FATAL_ERROR "camino_DIR variable is defined but corresponds to non-existing directory \"${camino_DIR}\".")
endif()

if(BUILD_CAMINO AND NOT WIN32)

  find_package(Java COMPONENTS Development)

  if(NOT "${Java_VERSION}" STREQUAL "")

    set(version "4612bee5fa")
    set(location "${NIFTK_EP_TARBALL_LOCATION}/camino-${version}.tar.gz")
    set(depends "")

    niftkMacroDefineExternalProjectVariables(Camino ${version} ${location} "${depends}")

    if(NOT DEFINED camino_DIR)

      ExternalProject_Add(${proj}
        LIST_SEPARATOR ^^
        PREFIX ${proj_CONFIG}
        SOURCE_DIR ${proj_SOURCE}
        URL ${proj_LOCATION}
        URL_MD5 ${proj_CHECKSUM}
        CONFIGURE_COMMAND ""
        INSTALL_COMMAND ""
        UPDATE_COMMAND ""
        BUILD_IN_SOURCE ON
        LOG_BUILD ON
        CMAKE_ARGS
          ${EP_COMMON_ARGS}
          -DCMAKE_PREFIX_PATH:PATH=${NifTK_PREFIX_PATH}
        CMAKE_CACHE_ARGS
          ${EP_COMMON_CACHE_ARGS}
        CMAKE_CACHE_DEFAULT_ARGS
          ${EP_COMMON_CACHE_DEFAULT_ARGS}
        DEPENDS ${proj_DEPENDENCIES}
      )

      set(camino_DIR ${proj_SOURCE})

      #set(NifTK_PREFIX_PATH ${proj_INSTALL}^^${NifTK_PREFIX_PATH})
      #mitkFunctionInstallExternalCMakeProject(${proj})

      message("SuperBuild loading Camino from ${camino_DIR}")

    else(NOT DEFINED camino_DIR)

      mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

    endif(NOT DEFINED camino_DIR)

  endif()

endif()
