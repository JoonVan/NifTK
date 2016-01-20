/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "uk_ac_ucl_cmic_igiundistort_Activator.h"
#include <QtPlugin>
#include "UndistortView.h"
#include "UndistortViewPreferencesPage.h"


namespace mitk 
{


ctkPluginContext* uk_ac_ucl_cmic_igiundistort_Activator::m_PluginContext = 0;


//-----------------------------------------------------------------------------
void uk_ac_ucl_cmic_igiundistort_Activator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(UndistortView, context)
  BERRY_REGISTER_EXTENSION_CLASS(UndistortViewPreferencesPage, context);
  m_PluginContext = context;
}


//-----------------------------------------------------------------------------
void uk_ac_ucl_cmic_igiundistort_Activator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
  assert(m_PluginContext == context);
  m_PluginContext = 0;
}


//-----------------------------------------------------------------------------
ctkPluginContext* uk_ac_ucl_cmic_igiundistort_Activator::getContext()
{
  return m_PluginContext;
}


}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_igiundistort, mitk::uk_ac_ucl_cmic_igiundistort_Activator)
#endif
