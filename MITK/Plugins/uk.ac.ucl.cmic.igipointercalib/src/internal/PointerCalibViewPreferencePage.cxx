/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "PointerCalibViewPreferencePage.h"
#include "PointerCalibView.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <ctkPathLineEdit.h>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

//-----------------------------------------------------------------------------
PointerCalibViewPreferencePage::PointerCalibViewPreferencePage()
: m_MainControl(0)
, m_Initializing(false)
, m_PointerCalibViewPreferencesNode(0)
{
}


//-----------------------------------------------------------------------------
PointerCalibViewPreferencePage::PointerCalibViewPreferencePage(const PointerCalibViewPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
PointerCalibViewPreferencePage::~PointerCalibViewPreferencePage()
{
}


//-----------------------------------------------------------------------------
void PointerCalibViewPreferencePage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void PointerCalibViewPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Initializing = true;

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  m_PointerCalibViewPreferencesNode = prefService->GetSystemPreferences()->Node(PointerCalibView::VIEW_ID);

  m_MainControl = new QWidget(parent);
  QFormLayout *formLayout = new QFormLayout;

  m_MainControl->setLayout(formLayout);
  this->Update();

  m_Initializing = false;
}


//-----------------------------------------------------------------------------
QWidget* PointerCalibViewPreferencePage::GetQtControl() const
{
  return m_MainControl;
}


//-----------------------------------------------------------------------------
bool PointerCalibViewPreferencePage::PerformOk()
{
  return true;
}


//-----------------------------------------------------------------------------
void PointerCalibViewPreferencePage::PerformCancel()
{

}


//-----------------------------------------------------------------------------
void PointerCalibViewPreferencePage::Update()
{
}
