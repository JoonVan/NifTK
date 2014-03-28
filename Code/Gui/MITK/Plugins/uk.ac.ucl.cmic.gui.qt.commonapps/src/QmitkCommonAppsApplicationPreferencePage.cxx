/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkCommonAppsApplicationPreferencePage.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

const std::string QmitkCommonAppsApplicationPreferencePage::IMAGE_RESLICE_INTERPOLATION("default reslice interpolation");
const std::string QmitkCommonAppsApplicationPreferencePage::IMAGE_TEXTURE_INTERPOLATION("default texture interpolation");
const std::string QmitkCommonAppsApplicationPreferencePage::LOWEST_VALUE_IS_OPAQUE("lowest value is opaque");
const std::string QmitkCommonAppsApplicationPreferencePage::HIGHEST_VALUE_IS_OPAQUE("highest value is opaque");
const std::string QmitkCommonAppsApplicationPreferencePage::BINARY_OPACITY_NAME("binary opacity");
const double QmitkCommonAppsApplicationPreferencePage::BINARY_OPACITY_VALUE = 1.0;

//-----------------------------------------------------------------------------
QmitkCommonAppsApplicationPreferencePage::QmitkCommonAppsApplicationPreferencePage()
: m_MainControl(0)
, m_ResliceInterpolation(0)
, m_TextureInterpolation(0)
, m_LowestValueIsOpaque(0)
, m_HighestValueIsOpaque(0)
, m_BinaryOpacity(0)
{

}


//-----------------------------------------------------------------------------
QmitkCommonAppsApplicationPreferencePage::QmitkCommonAppsApplicationPreferencePage(const QmitkCommonAppsApplicationPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
QmitkCommonAppsApplicationPreferencePage::~QmitkCommonAppsApplicationPreferencePage()
{

}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPreferencePage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_PreferencesNode = prefService->GetSystemPreferences()->Node("/uk.ac.ucl.cmic.gui.qt.commonapps");

  m_MainControl = new QWidget(parent);

  m_ResliceInterpolation = new QComboBox();
  m_ResliceInterpolation->insertItem(0, "none");
  m_ResliceInterpolation->insertItem(1, "linear");
  m_ResliceInterpolation->insertItem(2, "cubic");

  m_TextureInterpolation = new QComboBox();
  m_TextureInterpolation->insertItem(0, "none");
  m_TextureInterpolation->insertItem(1, "linear");

  m_LowestValueIsOpaque = new QCheckBox();
  m_HighestValueIsOpaque = new QCheckBox();

  m_BinaryOpacity = new QDoubleSpinBox();
  m_BinaryOpacity->setMinimum(0);
  m_BinaryOpacity->setMaximum(1);
  m_BinaryOpacity->setSingleStep(1);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( "Image reslice interpolation:", m_ResliceInterpolation );
  formLayout->addRow( "Image texture interpolation:", m_TextureInterpolation );
  formLayout->addRow( "Lowest lookup table value is opaque:", m_LowestValueIsOpaque);
  formLayout->addRow( "Highest lookup table value is opaque:", m_HighestValueIsOpaque);
  formLayout->addRow( "Default opacity when loading binary images:", m_BinaryOpacity);

  m_MainControl->setLayout(formLayout);
  this->Update();
}


//-----------------------------------------------------------------------------
QWidget* QmitkCommonAppsApplicationPreferencePage::GetQtControl() const
{
  return m_MainControl;
}


//-----------------------------------------------------------------------------
bool QmitkCommonAppsApplicationPreferencePage::PerformOk()
{
  m_PreferencesNode->PutInt(IMAGE_RESLICE_INTERPOLATION, m_ResliceInterpolation->currentIndex());
  m_PreferencesNode->PutInt(IMAGE_TEXTURE_INTERPOLATION, m_TextureInterpolation->currentIndex());
  m_PreferencesNode->PutBool(LOWEST_VALUE_IS_OPAQUE, m_LowestValueIsOpaque->isChecked());
  m_PreferencesNode->PutBool(HIGHEST_VALUE_IS_OPAQUE, m_HighestValueIsOpaque->isChecked());
  m_PreferencesNode->PutDouble(BINARY_OPACITY_NAME, m_BinaryOpacity->value());
  return true;
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPreferencePage::PerformCancel()
{

}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPreferencePage::Update()
{
  m_ResliceInterpolation->setCurrentIndex(m_PreferencesNode->GetInt(IMAGE_RESLICE_INTERPOLATION, 2));
  m_TextureInterpolation->setCurrentIndex(m_PreferencesNode->GetInt(IMAGE_TEXTURE_INTERPOLATION, 1));
  m_LowestValueIsOpaque->setChecked(m_PreferencesNode->GetBool(LOWEST_VALUE_IS_OPAQUE, true));
  m_HighestValueIsOpaque->setChecked(m_PreferencesNode->GetBool(HIGHEST_VALUE_IS_OPAQUE, true));
  m_BinaryOpacity->setValue(m_PreferencesNode->GetDouble(BINARY_OPACITY_NAME, QmitkCommonAppsApplicationPreferencePage::BINARY_OPACITY_VALUE));
}
