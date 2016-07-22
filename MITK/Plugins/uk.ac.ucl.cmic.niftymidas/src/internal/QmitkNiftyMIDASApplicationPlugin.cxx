/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkNiftyMIDASApplicationPlugin.h"
#include <niftkSegmentationPerspective.h>
#include <niftkQCPerspective.h>
#include <QmitkNiftyViewApplicationPreferencePage.h>
#include "../QmitkNiftyMIDASApplication.h"

//-----------------------------------------------------------------------------
QmitkNiftyMIDASApplicationPlugin::QmitkNiftyMIDASApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QmitkNiftyMIDASApplicationPlugin::~QmitkNiftyMIDASApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QString QmitkNiftyMIDASApplicationPlugin::GetHelpHomePageURL() const
{
  return QString("qthelp://uk.ac.ucl.cmic.niftymidas/bundle/uk_ac_ucl_cmic_niftymidas_intro.html");
}


//-----------------------------------------------------------------------------
void QmitkNiftyMIDASApplicationPlugin::start(ctkPluginContext* context)
{
  /// Note:
  /// This function has to be redefined so that the superclass
  /// implementation does not run again. The overridden function
  /// has been executed when the commonapps plugin has been loaded.

  this->SetPluginContext(context);

  BERRY_REGISTER_EXTENSION_CLASS(QmitkNiftyMIDASApplication, context);
  BERRY_REGISTER_EXTENSION_CLASS(niftk::SegmentationPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(niftk::QCPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkNiftyViewApplicationPreferencePage, context);

  this->RegisterHelpSystem();
  /// Note:
  /// By default there is a global reinit after file open what reinitialises the global
  /// rendering manager. The ideal would be if the DnD Display could use its own rendering
  /// manager (RM), not the global one. This, however, does not work now because many MITK
  /// views have hard coded reference to the global RM, and they call RequestUpdate on that,
  /// not on the RM of the focused renderer. Until this is fixed in MITK, we have to suppress
  /// the global reinit after file open, and should not use the MITK Display and the DnD Display
  /// together in the same application.
  this->SetFileOpenTriggersReinit(false);
}


//-----------------------------------------------------------------------------
void QmitkNiftyMIDASApplicationPlugin::stop(ctkPluginContext* context)
{
  /// Note:
  /// This function has to be redefined so that the superclass
  /// implementation does not run again. The overridden function
  /// will be executed when the commonapps plugin gets unloaded.
}


//-----------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_niftymidas, QmitkNiftyMIDASApplicationPlugin)
#endif
