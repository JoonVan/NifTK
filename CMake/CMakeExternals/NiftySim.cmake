#-----------------------------------------------------------------------------
# NIFTYSIM
#-----------------------------------------------------------------------------

# Sanity checks
IF(DEFINED NIFTYSIM_ROOT AND NOT EXISTS ${NIFTYSIM_ROOT})
  MESSAGE(FATAL_ERROR "NIFTYSIM_ROOT variable is defined but corresponds to non-existing directory \"${NIFTYSIM_ROOT}\".")
ENDIF()

IF(BUILD_NIFTYSIM)

  SET(proj NIFTYSIM)
  SET(proj_DEPENDENCIES VTK )
  SET(proj_INSTALL ${EP_BASE}/Install/${proj} )
  SET(NIFTYSIM_DEPENDS ${proj})

  IF(NOT DEFINED NIFTYSIM_ROOT)

    IF(DEFINED VTK_DIR)
      SET(USE_VTK ON)
    ELSE(DEFINED VTK_DIR)
      SET(USE_VTK OFF)
    ENDIF(DEFINED VTK_DIR)
    
    ExternalProject_Add(${proj}
      SVN_REPOSITORY https://niftysim.svn.sourceforge.net/svnroot/niftysim/trunk/nifty_sim/
      CMAKE_GENERATOR ${GEN}
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
        -DBUILD_SHARED_LIBS:BOOL=OFF
	-DUSE_CUDA:BOOL=${NIFTK_USE_CUDA}
        -DCUDA_CUT_INCLUDE_DIR:PATH=${CUDA_CUT_INCLUDE_DIR}
        -DVTK_DIR:PATH=${VTK_DIR}
        -DUSE_VIZ:BOOL=${USE_VTK}
        -DCMAKE_INSTALL_PREFIX:PATH=${proj_INSTALL}
      DEPENDS ${proj_DEPENDENCIES}
      )

    SET(NIFTYSIM_ROOT ${proj_INSTALL})
    SET(NIFTYSIM_INCLUDE_DIR "${NIFTYSIM_ROOT}/include")
    SET(NIFTYSIM_LIBRARY_DIR "${NIFTYSIM_ROOT}/lib")

    MESSAGE("SuperBuild loading NIFTYSIM from ${NIFTYSIM_ROOT}")

  ELSE(NOT DEFINED NIFTYSIM_ROOT)

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")

  ENDIF(NOT DEFINED NIFTYSIM_ROOT)

ENDIF(BUILD_NIFTYSIM)
