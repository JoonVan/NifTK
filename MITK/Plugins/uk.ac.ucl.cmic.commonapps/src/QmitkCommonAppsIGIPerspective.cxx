/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkCommonAppsIGIPerspective.h"
#include <berryIViewLayout.h>

//-----------------------------------------------------------------------------
QmitkCommonAppsIGIPerspective::QmitkCommonAppsIGIPerspective()
{
}


//-----------------------------------------------------------------------------
QmitkCommonAppsIGIPerspective::QmitkCommonAppsIGIPerspective(const QmitkCommonAppsIGIPerspective& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsIGIPerspective::CreateInitialLayout(berry::IPageLayout::Pointer layout)
{
  QString editorArea = layout->GetEditorArea();

  layout->AddView("org.mitk.views.datamanager",
    berry::IPageLayout::LEFT, 0.2f, editorArea);

  berry::IViewLayout::Pointer lo = layout->GetViewLayout("org.mitk.views.datamanager");
  lo->SetCloseable(false);

  layout->AddView("org.mitk.views.properties",
    berry::IPageLayout::BOTTOM, 0.3f, "org.mitk.views.datamanager");

  layout->AddView("uk.ac.ucl.cmic.igidatasources",
    berry::IPageLayout::RIGHT, 0.66f, editorArea);
}
