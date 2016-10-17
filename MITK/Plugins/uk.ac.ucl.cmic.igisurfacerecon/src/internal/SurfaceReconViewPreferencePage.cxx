/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "SurfaceReconViewPreferencePage.h"
#include "SurfaceReconView.h"

#include <cassert>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

//-----------------------------------------------------------------------------
const char* SurfaceReconViewPreferencePage::s_DefaultCalibrationFilePathPrefsName   = "default calib file path";
const char* SurfaceReconViewPreferencePage::s_UseUndistortionDefaultPathPrefsName   = "use undistort default path";
const char* SurfaceReconViewPreferencePage::s_DefaultTriangulationErrorPrefsName    = "default triangulation error";
const char* SurfaceReconViewPreferencePage::s_DefaultMinDepthRangePrefsName         = "default min depth range";
const char* SurfaceReconViewPreferencePage::s_DefaultMaxDepthRangePrefsName         = "default max depth range";
const char* SurfaceReconViewPreferencePage::s_DefaultBakeCameraTransformPrefsName   = "default bake camera transform";


//-----------------------------------------------------------------------------
SurfaceReconViewPreferencePage::SurfaceReconViewPreferencePage()
: m_UseUndistortPluginDefaultPath(false)
{
}


//-----------------------------------------------------------------------------
SurfaceReconViewPreferencePage::SurfaceReconViewPreferencePage(const SurfaceReconViewPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
SurfaceReconViewPreferencePage::~SurfaceReconViewPreferencePage()
{
  bool  ok = false;
  ok = disconnect(m_DefaultCalibrationLocationBrowseButton, SIGNAL(clicked()), this, SLOT(OnDefaultPathBrowseButtonClicked()));
  assert(ok);
  ok = disconnect(m_UseUndistortionDefaultPathRadioButton, SIGNAL(clicked()), this, SLOT(OnUseUndistortRadioButtonClicked()));
  assert(ok);
  ok = disconnect(m_UseOwnLocationRadioButton, SIGNAL(clicked()), this, SLOT(OnUseUndistortRadioButtonClicked()));
  assert(ok);
}


//-----------------------------------------------------------------------------
void SurfaceReconViewPreferencePage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void SurfaceReconViewPreferencePage::OnDefaultPathBrowseButtonClicked()
{
  QString   file = QFileDialog::getExistingDirectory(GetQtControl(), "Calibration File Path", m_DefaultCalibrationFilePath);
  if (!file.isEmpty())
  {
    m_DefaultCalibrationFilePath = file;
    m_DefaultCalibrationLocationLineEdit->setText(m_DefaultCalibrationFilePath);
  }
}


//-----------------------------------------------------------------------------
void SurfaceReconViewPreferencePage::OnUseUndistortRadioButtonClicked()
{
  if (m_UseUndistortionDefaultPathRadioButton->isChecked())
  {
    assert(!m_UseOwnLocationRadioButton->isChecked());
    m_UseUndistortPluginDefaultPath = true;
  }
  else
  {
    assert(m_UseOwnLocationRadioButton->isChecked());
    m_UseUndistortPluginDefaultPath = false;
  }

  m_DefaultCalibrationLocationLineEdit->setEnabled(!m_UseUndistortPluginDefaultPath);
  m_DefaultCalibrationLocationBrowseButton->setEnabled(!m_UseUndistortPluginDefaultPath);
}


//-----------------------------------------------------------------------------
void SurfaceReconViewPreferencePage::CreateQtControl(QWidget* parent)
{
  setupUi(parent);

  bool  ok = false;
  ok = connect(m_DefaultCalibrationLocationBrowseButton, SIGNAL(clicked()), this, SLOT(OnDefaultPathBrowseButtonClicked()));
  assert(ok);
  ok = connect(m_UseUndistortionDefaultPathRadioButton, SIGNAL(clicked()), this, SLOT(OnUseUndistortRadioButtonClicked()), Qt::QueuedConnection);
  assert(ok);
  ok = connect(m_UseOwnLocationRadioButton, SIGNAL(clicked()), this, SLOT(OnUseUndistortRadioButtonClicked()), Qt::QueuedConnection);
  assert(ok);

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  m_SurfaceReconViewPreferencesNode = prefService->GetSystemPreferences()->Node(SurfaceReconView::VIEW_ID);

  Update();
}


//-----------------------------------------------------------------------------
QWidget* SurfaceReconViewPreferencePage::GetQtControl() const
{
  return SurfaceReconViewPreferencePageWidget;
}


//-----------------------------------------------------------------------------
bool SurfaceReconViewPreferencePage::PerformOk()
{
  m_SurfaceReconViewPreferencesNode->Put(s_DefaultCalibrationFilePathPrefsName, m_DefaultCalibrationLocationLineEdit->text());
  m_SurfaceReconViewPreferencesNode->PutBool(s_UseUndistortionDefaultPathPrefsName, m_UseUndistortionDefaultPathRadioButton->isChecked());
  m_SurfaceReconViewPreferencesNode->PutFloat(s_DefaultTriangulationErrorPrefsName, (float) m_DefaultTriangulationErrorThresholdSpinBox->value());
  m_SurfaceReconViewPreferencesNode->PutFloat(s_DefaultMinDepthRangePrefsName, (float) m_MinDefaultDepthRangeSpinBox->value());
  m_SurfaceReconViewPreferencesNode->PutFloat(s_DefaultMaxDepthRangePrefsName, (float) m_MaxDefaultDepthRangeSpinBox->value());
  m_SurfaceReconViewPreferencesNode->PutBool(s_DefaultBakeCameraTransformPrefsName, m_BakeWorldTransformCheckBox->isChecked());

  return true;
}


//-----------------------------------------------------------------------------
void SurfaceReconViewPreferencePage::PerformCancel()
{

}


//-----------------------------------------------------------------------------
void SurfaceReconViewPreferencePage::Update()
{
  m_DefaultCalibrationFilePath = m_SurfaceReconViewPreferencesNode->Get(s_DefaultCalibrationFilePathPrefsName, "");
  m_DefaultCalibrationLocationLineEdit->setText(m_DefaultCalibrationFilePath);

  m_UseUndistortPluginDefaultPath = m_SurfaceReconViewPreferencesNode->GetBool(s_UseUndistortionDefaultPathPrefsName, true);
  if (m_UseUndistortPluginDefaultPath)
  {
    m_UseUndistortionDefaultPathRadioButton->click();
  }
  else
  {
    m_UseOwnLocationRadioButton->click();
  }

  m_DefaultTriangulationErrorThresholdSpinBox->setValue(m_SurfaceReconViewPreferencesNode->GetFloat(s_DefaultTriangulationErrorPrefsName, 0.1f));

  m_MinDefaultDepthRangeSpinBox->setValue(m_SurfaceReconViewPreferencesNode->GetFloat(s_DefaultMinDepthRangePrefsName, 1.0f));
  m_MaxDefaultDepthRangeSpinBox->setValue(m_SurfaceReconViewPreferencesNode->GetFloat(s_DefaultMaxDepthRangePrefsName, 1000.0f));

  m_BakeWorldTransformCheckBox->setChecked(m_SurfaceReconViewPreferencesNode->GetBool(s_DefaultBakeCameraTransformPrefsName, true));
}
