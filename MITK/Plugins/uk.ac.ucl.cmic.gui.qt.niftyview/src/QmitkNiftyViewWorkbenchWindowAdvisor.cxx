/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkNiftyViewWorkbenchWindowAdvisor.h"

//-----------------------------------------------------------------------------
QmitkNiftyViewWorkbenchWindowAdvisor::QmitkNiftyViewWorkbenchWindowAdvisor(
    berry::WorkbenchAdvisor* wbAdvisor,
    berry::IWorkbenchWindowConfigurer::Pointer configurer)
: QmitkBaseWorkbenchWindowAdvisor(wbAdvisor, configurer)
{
}


//-----------------------------------------------------------------------------
void QmitkNiftyViewWorkbenchWindowAdvisor::PostWindowCreate()
{
  QmitkBaseWorkbenchWindowAdvisor::PostWindowCreate();
  this->OpenEditorIfEnvironmentVariableIsON("NIFTK_MITK_DISPLAY", "org.mitk.editors.stdmultiwidget");
}
