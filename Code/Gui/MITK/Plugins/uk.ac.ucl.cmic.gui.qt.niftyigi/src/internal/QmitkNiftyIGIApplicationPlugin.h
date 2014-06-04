/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkNiftyIGIApplicationPlugin_h
#define QmitkNiftyIGIApplicationPlugin_h

#include <QmitkCommonAppsApplicationPlugin.h>

/**
 * \class QmitkNiftyIGIApplicationPlugin
 * \brief Implements QT and CTK specific functionality to launch the application as a plugin.
 * \ingroup uk_ac_ucl_cmic_gui_qt_niftyigi_internal
 */
class QmitkNiftyIGIApplicationPlugin : public QmitkCommonAppsApplicationPlugin
{
  Q_OBJECT
  
public:

  QmitkNiftyIGIApplicationPlugin();
  ~QmitkNiftyIGIApplicationPlugin();

  virtual void start(ctkPluginContext*);
  virtual void stop(ctkPluginContext*);

protected:

  /// \brief Called each time a data node is added, so we make sure it is initialised with a Window/Level.
  virtual void NodeAdded(const mitk::DataNode *node);

  /// \brief Called by framework to get a URL for help system.
  virtual QString GetHelpHomePageURL() const;

private:

};

#endif /* QMITKNIFTYIGIAPPLICATIONPLUGIN_H_ */
