/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkNiftyIGIApplicationPlugin.h"
#include <QmitkCommonAppsIGIPerspective.h>
#include "../QmitkNiftyIGIApplication.h"
#include <QmitkNiftyViewApplicationPreferencePage.h>
#include <QmitkCommonAppsApplicationPreferencePage.h>

//-----------------------------------------------------------------------------
QmitkNiftyIGIApplicationPlugin::QmitkNiftyIGIApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QmitkNiftyIGIApplicationPlugin::~QmitkNiftyIGIApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QString QmitkNiftyIGIApplicationPlugin::GetHelpHomePageURL() const
{
  return QString("qthelp://uk.ac.ucl.cmic.gui.qt.niftyigi/bundle/uk_ac_ucl_cmic_gui_qt_niftyigi_intro.html");
}


//-----------------------------------------------------------------------------
void QmitkNiftyIGIApplicationPlugin::start(ctkPluginContext* context)
{
  QmitkCommonAppsApplicationPlugin::start(context);

  BERRY_REGISTER_EXTENSION_CLASS(QmitkNiftyIGIApplication, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCommonAppsIGIPerspective, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkCommonAppsApplicationPreferencePage, context);
  BERRY_REGISTER_EXTENSION_CLASS(QmitkNiftyViewApplicationPreferencePage, context);

  this->RegisterHelpSystem();
}


//-----------------------------------------------------------------------------
void QmitkNiftyIGIApplicationPlugin::stop(ctkPluginContext* context)
{
  this->UnregisterDataStorageListener();
}


//-----------------------------------------------------------------------------
void QmitkNiftyIGIApplicationPlugin::NodeAdded(const mitk::DataNode *constNode)
{
  mitk::DataNode::Pointer node = const_cast<mitk::DataNode*>(constNode);
  this->RegisterLevelWindowProperty("uk.ac.ucl.cmic.gui.qt.niftyigi", node);
}


//-----------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_gui_qt_niftyigi, QmitkNiftyIGIApplicationPlugin)
#endif
