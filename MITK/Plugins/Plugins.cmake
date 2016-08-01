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

###################################################################
# Plug-ins must be ordered according to their dependencies.
#
# Imagine that the build process, configures these plugins
# in order, from top to bottom. So, if plugin A depends on
# plugin B, then plugin B must be configured BEFORE plugin
# A. i.e. nearer the top of this file.
#
# This is of particular importance if you change a base
# class of a plugin. For example lets say your plugin
# currently depends on MITK's QmitkAbstractView, and
# hence you declare a dependency in manifest_headers.cmake
# on org.mitk.gui.qt.common.
#
# If you then subsequently change to derive from QmitkBaseView
# from the NifTK project, and declare a dependency in
# manifest_headers.cmake on uk.ac.ucl.cmic.common
# then your plugin must occur AFTER uk.ac.ucl.cmic.common.
#
# This is difficult to spot, as quite typically, during development
# then will be multiple build, config, build, config cycles
# anyway, such that the plugin will appear to be correctly
# configured. Worse than this, if you see a problem and then
# re-run cmake, the problem will disappear, but will re-appear
# on the next full clean build.
###################################################################

set(PROJECT_PLUGINS

# These are 'Application' plugins, and so are 'View' independent.
  Plugins/uk.ac.ucl.cmic.commonapps:ON

# These are 'View' plugins, and just depend on MITK.
  Plugins/uk.ac.ucl.cmic.snapshot:ON
  Plugins/uk.ac.ucl.cmic.imagestatistics:ON
  Plugins/uk.ac.ucl.cmic.xnat:ON
  Plugins/uk.ac.ucl.cmic.niftyreg:OFF                     # Must be after the xnat plugin
  Plugins/uk.ac.ucl.cmic.niftyseg:OFF                     # Not ready yet.
  Plugins/uk.ac.ucl.cmic.breastsegmentation:OFF           # Under development

# This 'common' plugin is our preferred base class for things that can't just derive from MITK.
  Plugins/uk.ac.ucl.cmic.common:ON
  Plugins/uk.ac.ucl.cmic.imagelookuptables:ON
  Plugins/uk.ac.ucl.cmic.affinetransform:ON
  Plugins/uk.ac.ucl.cmic.surfaceextractor:ON
)


# ---------------------------------------------------------------------------------------------------
# NiftyView Specific Plugins
# ---------------------------------------------------------------------------------------------------

set(NiftyView_PLUGINS
  Plugins/uk.ac.ucl.cmic.niftyview:ON
)

if(BUILD_VL)
  set(NiftyView_PLUGINS
    ${NiftyView_PLUGINS}
    Plugins/uk.ac.ucl.cmic.vlstandarddisplayeditor:ON
  )
endif()

if(NIFTK_Apps/NiftyView)
  set(PROJECT_PLUGINS
    ${PROJECT_PLUGINS}
    ${NiftyView_PLUGINS}
  )
endif()

# ---------------------------------------------------------------------------------------------------
# NiftyMIDAS Specific Plugins
# ---------------------------------------------------------------------------------------------------

set(NiftyMIDAS_PLUGINS
  Plugins/uk.ac.ucl.cmic.commonmidas:ON
  Plugins/uk.ac.ucl.cmic.dnddisplay:ON
  Plugins/uk.ac.ucl.cmic.niftymidas:ON
  Plugins/uk.ac.ucl.cmic.sideviewer:ON
  Plugins/uk.ac.ucl.cmic.thumbnail:ON
  Plugins/uk.ac.ucl.cmic.midasmorphologicalsegmentor:ON
  Plugins/uk.ac.ucl.cmic.midasgeneralsegmentor:ON
  Plugins/uk.ac.ucl.cmic.pointsetconverter:ON
)

if(NIFTK_Apps/NiftyMIDAS)
  set(PROJECT_PLUGINS
    ${PROJECT_PLUGINS}
    ${NiftyMIDAS_PLUGINS}
  )
endif()

# ---------------------------------------------------------------------------------------------------
# NiftyIGI Specific Plugins
# ---------------------------------------------------------------------------------------------------

set(NiftyIGI_PLUGINS
  Plugins/uk.ac.ucl.cmic.niftyigi:ON
  Plugins/uk.ac.ucl.cmic.igivideooverlayeditor:ON
  Plugins/uk.ac.ucl.cmic.igiultrasoundoverlayeditor:ON
  Plugins/uk.ac.ucl.cmic.igidatasources:ON
  Plugins/uk.ac.ucl.cmic.igisurfacerecon:ON
  Plugins/uk.ac.ucl.cmic.igitrackedimage:ON
  Plugins/uk.ac.ucl.cmic.igitrackedpointer:ON
  Plugins/uk.ac.ucl.cmic.igipointreg:ON
  Plugins/uk.ac.ucl.cmic.igisurfacereg:ON
  Plugins/uk.ac.ucl.cmic.igiundistort:ON
  Plugins/uk.ac.ucl.cmic.igirmserror:ON
  Plugins/uk.ac.ucl.cmic.igipointsetcropper:ON
  Plugins/uk.ac.ucl.cmic.igipivotcalibration:ON
  Plugins/uk.ac.ucl.cmic.igipointercalib:ON
  Plugins/uk.ac.ucl.cmic.igicameracal:ON
)

if(WIN32)
  set(NiftyIGI_PLUGINS
    ${NiftyIGI_PLUGINS}
    Plugins/uk.ac.ucl.cmic.igifootpedalhotkey:ON
  )
endif()

if(BUILD_VL)
  set(NiftyIGI_PLUGINS
    ${NiftyIGI_PLUGINS}
    Plugins/uk.ac.ucl.cmic.igivlvideooverlayeditor:ON
  )
endif()

if(NIFTK_Apps/NiftyIGI)
  set(PROJECT_PLUGINS
    ${PROJECT_PLUGINS}
    ${NiftyIGI_PLUGINS}
  )
endif()

# ---------------------------------------------------------------------------------------------------
# OBSOLETE, Unsupported plugins.
# ---------------------------------------------------------------------------------------------------
set(OBSOLETE_PLUGINS
  Plugins/uk.ac.ucl.cmic.niftyseg:OFF
  Plugins/uk.ac.ucl.cmic.niftyreg:OFF # If turned on, must be listed after the xnat plugin.
  Plugins/uk.ac.ucl.cmic.breastsegmentation:OFF
  Plugins/uk.ac.ucl.cmic.mitksegmentation:OFF
  Plugins/it.unito.cim.intensityprofile:OFF
)
