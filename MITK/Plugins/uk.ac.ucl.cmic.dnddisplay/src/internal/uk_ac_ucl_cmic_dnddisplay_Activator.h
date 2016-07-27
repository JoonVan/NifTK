/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef uk_ac_ucl_cmic_dnddisplay_Activator_h
#define uk_ac_ucl_cmic_dnddisplay_Activator_h

#include <ctkPluginActivator.h>

#include <vector>

class QmitkRenderWindow;

namespace mitk
{
class DataNode;
class DataStorage;
}


namespace niftk
{

/**
 * \class uk_ac_ucl_cmic_dnddisplay_Activator
 * \brief CTK Plugin Activator class for the DnD Display Plugin.
 * \ingroup uk_ac_ucl_cmic_dnddisplay_internal
 */
class uk_ac_ucl_cmic_dnddisplay_Activator :
  public QObject, public ctkPluginActivator
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "uk_ac_ucl_cmic_dnddisplay")
#endif

public:

  uk_ac_ucl_cmic_dnddisplay_Activator();

  void start(ctkPluginContext* context);
  void stop(ctkPluginContext* context);

  static uk_ac_ucl_cmic_dnddisplay_Activator* GetInstance();

  ctkPluginContext* GetPluginContext();

private:

  static uk_ac_ucl_cmic_dnddisplay_Activator* s_Instance;

  ctkPluginContext* m_PluginContext;

};

}

#endif
