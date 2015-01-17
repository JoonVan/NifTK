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

# This flag should always be on. The if() statement is left so that it is easier
# to see what has changed in this file in MITK.
set(MITK_USE_DCMTK 1)

#-----------------------------------------------------------------------------
# DCMTK
#-----------------------------------------------------------------------------
if(MITK_USE_DCMTK)

  # Sanity checks
  if(DEFINED DCMTK_DIR AND NOT EXISTS ${DCMTK_DIR})
    message(FATAL_ERROR "DCMTK_DIR variable is defined but corresponds to non-existing directory")
  endif()

  set(proj DCMTK)
  set(proj_VERSION ${NIFTK_VERSION_${proj}})
  set(proj_SOURCE ${EP_BASE}/${proj}-${proj_VERSION}-src)
  set(proj_CONFIG ${EP_BASE}/${proj}-${proj_VERSION}-cmake)
  set(proj_BUILD ${EP_BASE}/${proj}-${proj_VERSION}-build)
  set(proj_INSTALL ${EP_BASE}/${proj}-${proj_VERSION}-install)
  set(proj_DEPENDENCIES )
  set(DCMTK_DEPENDS ${proj})

  if(CMAKE_GENERATOR MATCHES Xcode)
    set(DCMTK_PATCH_COMMAND ${CMAKE_COMMAND} -DTEMPLATE_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/CMake/CMakeExternals/EmptyFileForPatching.dummy -P ${CMAKE_SOURCE_DIR}/CMake/CMakeExternals/PatchDCMTK-20121102.cmake)
  endif()

  if(NOT DEFINED DCMTK_DIR)
    if(DCMTK_DICOM_ROOT_ID)
      set(DCMTK_CXX_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
      set(DCMTK_C_FLAGS "${DCMTK_CXX_FLAGS} -DSITE_UID_ROOT=\\\"${DCMTK_DICOM_ROOT_ID}\\\"")
    endif()

    niftkMacroGetChecksum(NIFTK_CHECKSUM_DCMTK ${NIFTK_LOCATION_DCMTK})

    ExternalProject_Add(${proj}
      SOURCE_DIR ${proj_SOURCE}
      PREFIX ${proj_CONFIG}
      BINARY_DIR ${proj_BUILD}
      INSTALL_DIR ${proj_INSTALL}
      URL ${NIFTK_LOCATION_DCMTK}
      URL_MD5 ${NIFTK_CHECKSUM_DCMTK}
      PATCH_COMMAND ${DCMTK_PATCH_COMMAND}
      CMAKE_GENERATOR ${gen}
      CMAKE_ARGS
        ${EP_COMMON_ARGS}
        #-DDCMTK_OVERWRITE_WIN32_COMPILER_FLAGS:BOOL=OFF
        -DBUILD_SHARED_LIBS:BOOL=ON
        "-DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS} ${DCMTK_CXX_FLAGS}"
        "-DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS} ${DCMTK_C_FLAGS}"
        -DCMAKE_INSTALL_PREFIX:PATH=${proj_INSTALL}
        -DDCMTK_INSTALL_BINDIR:STRING=bin/${CMAKE_CFG_INTDIR}
        -DDCMTK_INSTALL_LIBDIR:STRING=lib/${CMAKE_CFG_INTDIR}
        -DDCMTK_WITH_DOXYGEN:BOOL=OFF
        -DDCMTK_WITH_ZLIB:BOOL=OFF # see MITK bug #9894
        -DDCMTK_WITH_OPENSSL:BOOL=OFF # see MITK bug #9894
        -DDCMTK_WITH_PNG:BOOL=OFF # see MITK bug #9894
        -DDCMTK_WITH_TIFF:BOOL=OFF  # see MITK bug #9894
        -DDCMTK_WITH_XML:BOOL=OFF  # see MITK bug #9894
        -DDCMTK_WITH_ICONV:BOOL=OFF  # see MITK bug #9894
        -DCMAKE_INSTALL_NAME_DIR:STRING=<INSTALL_DIR>/lib
      DEPENDS ${proj_DEPENDENCIES}
    )
    set(DCMTK_DIR ${proj_INSTALL})
    message("SuperBuild loading DCMTK from ${DCMTK_DIR}")

  else()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  endif()
endif()
