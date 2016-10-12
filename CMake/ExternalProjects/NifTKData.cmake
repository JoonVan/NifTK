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
# NifTKData - Downloads the unit-testing data as a separate project.
#-----------------------------------------------------------------------------

# Sanity checks
if (DEFINED NIFTK_DATA_DIR AND NOT EXISTS ${NIFTK_DATA_DIR})
  message(FATAL_ERROR "NIFTK_DATA_DIR variable is defined but corresponds to non-existing directory \"${NIFTK_DATA_DIR}\".")
endif ()

if (BUILD_TESTING)

  # Note:
  #
  # The whole NifTKData repository is rather big. To make the download faster,
  # we do not clone the entire repository, but check out only the single commit
  # that we need. This feature requires Git 2.5 or newer (both on the server and
  # the client) and the 'uploadpack.allowReachableSHA1InWant' option must be set
  # in the configuration of the repository on the server.
  #
  # So that this can work, you must specify the SHA1 commit hash in its whole
  # length, 40 digits, below.

  set(version_sha1 "f102e514c33b82578ec1216c6e5ae8b718c6490e")
  string(SUBSTRING ${version_sha1} 0 10 version)
  set(location "https://cmiclab.cs.ucl.ac.uk/CMIC/NifTKData.git")

  niftkMacroDefineExternalProjectVariables(NifTKData ${version} ${location})

  if (NOT DEFINED NIFTK_DATA_DIR)

    if (GIT_VERSION_STRING VERSION_LESS "2.5.0")
      set(_download_arguments
        GIT_REPOSITORY ${proj_LOCATION}
        GIT_TAG ${proj_VERSION}
      )
    else()
      set(_download_arguments
        # This is a fake download command, but something must be there so that CMake is happy.
        DOWNLOAD_COMMAND echo Downloading ${proj}...
        UPDATE_COMMAND ${GIT_EXECUTABLE} init
               COMMAND ${GIT_EXECUTABLE} fetch --depth 1 ${proj_LOCATION} ${version_sha1}
               COMMAND ${GIT_EXECUTABLE} checkout ${version_sha1}
      )
    endif()

    ExternalProject_Add(${proj}
      PREFIX ${proj_CONFIG}
      SOURCE_DIR ${proj_SOURCE}
      ${_download_arguments}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS ${proj_DEPENDENCIES}
    )

    set(NIFTK_DATA_DIR ${proj_SOURCE})
    message("SuperBuild loading ${proj} from ${NIFTK_DATA_DIR}")

  else ()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif (NOT DEFINED NIFTK_DATA_DIR)

endif (BUILD_TESTING)
