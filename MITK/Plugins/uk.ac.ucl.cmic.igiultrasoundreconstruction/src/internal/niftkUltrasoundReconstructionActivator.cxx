/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkUltrasoundReconstructionActivator.h"
#include "niftkUltrasoundReconstructionView.h"
#include "niftkUltrasoundReconstructionPreferencePage.h"

#include <QtPlugin>

namespace niftk
{

ctkPluginContext* UltrasoundReconstructionActivator::m_PluginContext = nullptr;

//-----------------------------------------------------------------------------
void UltrasoundReconstructionActivator::start(ctkPluginContext* context)
{
  BERRY_REGISTER_EXTENSION_CLASS(UltrasoundReconstructionView, context);
  BERRY_REGISTER_EXTENSION_CLASS(UltrasoundReconstructionPreferencePage, context);
  m_PluginContext = context;
}

//-----------------------------------------------------------------------------
void UltrasoundReconstructionActivator::stop(ctkPluginContext* context)
{
  Q_UNUSED(context)
}


//-----------------------------------------------------------------------------
ctkPluginContext* UltrasoundReconstructionActivator::getContext()
{
  return m_PluginContext;
}


} // end namespace

//-----------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_ultrasoundreconstruction, niftk::UltrasoundReconstructionActivator)
#endif
