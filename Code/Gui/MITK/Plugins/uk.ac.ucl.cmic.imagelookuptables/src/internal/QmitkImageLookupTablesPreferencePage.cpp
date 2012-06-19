/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-09-28 10:00:55 +0100 (Wed, 28 Sep 2011) $
 Revision          : $Revision: 7379 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkImageLookupTablesPreferencePage.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QSpinBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

const std::string QmitkImageLookupTablesPreferencePage::INITIALISATION_METHOD_NAME("initialisation method");
const std::string QmitkImageLookupTablesPreferencePage::INITIALISATION_MIDAS("initialisation by MIDAS convention");
const std::string QmitkImageLookupTablesPreferencePage::INITIALISATION_LEVELWINDOW("initialisation by level window");
const std::string QmitkImageLookupTablesPreferencePage::INITIALISATION_PERCENTAGE("initialisation by percentage of data range");
const std::string QmitkImageLookupTablesPreferencePage::PERCENTAGE_NAME("percentage");
const std::string QmitkImageLookupTablesPreferencePage::PRECISION_NAME("precision");

QmitkImageLookupTablesPreferencePage::QmitkImageLookupTablesPreferencePage()
: m_MainControl(0)
, m_UseMidasInitialisationRadioButton(0)
, m_UseLevelWindowRadioButton(0)
, m_UseImageDataRadioButton(0)
, m_PercentageOfDataRangeDoubleSpinBox(0)
, m_Initializing(false)
{

}

QmitkImageLookupTablesPreferencePage::QmitkImageLookupTablesPreferencePage(const QmitkImageLookupTablesPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

QmitkImageLookupTablesPreferencePage::~QmitkImageLookupTablesPreferencePage()
{

}

void QmitkImageLookupTablesPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkImageLookupTablesPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Initializing = true;
  berry::IPreferencesService::Pointer prefService 
    = berry::Platform::GetServiceRegistry()
      .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_ImageLookupTablesPreferencesNode = prefService->GetSystemPreferences()->Node("/uk.ac.ucl.cmic.imagelookuptables");

  m_MainControl = new QWidget(parent);

  QVBoxLayout* initialisationOptionsLayout = new QVBoxLayout;
  m_UseMidasInitialisationRadioButton = new QRadioButton( "MIDAS default", m_MainControl);
  initialisationOptionsLayout->addWidget( m_UseMidasInitialisationRadioButton );
  m_UseLevelWindowRadioButton = new QRadioButton( "from Level/Window widget", m_MainControl);
  initialisationOptionsLayout->addWidget( m_UseLevelWindowRadioButton );
  m_UseImageDataRadioButton = new QRadioButton( "from image data", m_MainControl);
  initialisationOptionsLayout->addWidget( m_UseImageDataRadioButton );

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow( "Initialization:", initialisationOptionsLayout );

  m_PercentageOfDataRangeDoubleSpinBox = new QDoubleSpinBox(m_MainControl );
  m_PercentageOfDataRangeDoubleSpinBox->setMinimum(0);
  m_PercentageOfDataRangeDoubleSpinBox->setMaximum(100);
  m_PercentageOfDataRangeDoubleSpinBox->setSingleStep(0.1);
  formLayout->addRow("Percentage:", m_PercentageOfDataRangeDoubleSpinBox);
  
  QLabel* precisionLabel = new QLabel("Precision:");
  m_Precision = new QSpinBox;
  QString precisionToolTip =
      "Precision of the floating point numbers.";
  precisionLabel->setToolTip(precisionToolTip);
  m_Precision->setToolTip(precisionToolTip);
  formLayout->addRow(precisionLabel, m_Precision);

  connect( m_UseLevelWindowRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnLevelWindowRadioButtonChecked(bool)));
  connect( m_UseImageDataRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnImageDataRadioButtonChecked(bool)));
  connect( m_UseMidasInitialisationRadioButton, SIGNAL(toggled(bool)), this, SLOT(OnMIDASInitialisationRadioButtonChecked(bool)));

  m_MainControl->setLayout(formLayout);
  this->Update();

  m_Initializing = false;
}

QWidget* QmitkImageLookupTablesPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkImageLookupTablesPreferencePage::PerformOk()
{
  std::string method;

  if (m_UseMidasInitialisationRadioButton->isChecked())
  {
    method = INITIALISATION_MIDAS;
  }
  else if (m_UseLevelWindowRadioButton->isChecked())
  {
    method = INITIALISATION_LEVELWINDOW;
  }
  else if (m_UseImageDataRadioButton->isChecked())
  {
    method = INITIALISATION_PERCENTAGE;
  }

  m_ImageLookupTablesPreferencesNode->Put(INITIALISATION_METHOD_NAME, method);
  m_ImageLookupTablesPreferencesNode->PutDouble(PERCENTAGE_NAME, m_PercentageOfDataRangeDoubleSpinBox->value());
  m_ImageLookupTablesPreferencesNode->PutInt(PRECISION_NAME, m_Precision->text().toInt());
  return true;
}

void QmitkImageLookupTablesPreferencePage::PerformCancel()
{

}

void QmitkImageLookupTablesPreferencePage::Update()
{
  std::string method = m_ImageLookupTablesPreferencesNode->Get(INITIALISATION_METHOD_NAME, INITIALISATION_MIDAS);
  if (method == INITIALISATION_MIDAS)
  {
    m_UseMidasInitialisationRadioButton->setChecked(true);
  }
  else if (method == INITIALISATION_LEVELWINDOW)
  {
    m_UseLevelWindowRadioButton->setChecked(true);
  }
  else if (method == INITIALISATION_PERCENTAGE)
  {
    m_UseImageDataRadioButton->setChecked(true);
  }
  m_PercentageOfDataRangeDoubleSpinBox->setValue(m_ImageLookupTablesPreferencesNode->GetDouble(PERCENTAGE_NAME, 50));
  m_Precision->setValue(m_ImageLookupTablesPreferencesNode->GetInt(PRECISION_NAME, 2));
}

void QmitkImageLookupTablesPreferencePage::OnMIDASInitialisationRadioButtonChecked(bool checked)
{
  if (m_Initializing) return;

  if (checked)
  {
    m_PercentageOfDataRangeDoubleSpinBox->setEnabled(false);
  }
  else
  {
    m_PercentageOfDataRangeDoubleSpinBox->setEnabled(true);
  }
}

void QmitkImageLookupTablesPreferencePage::OnLevelWindowRadioButtonChecked(bool checked)
{
  if (m_Initializing) return;

  if (checked)
  {
    m_PercentageOfDataRangeDoubleSpinBox->setEnabled(false);
  }
  else
  {
    m_PercentageOfDataRangeDoubleSpinBox->setEnabled(true);
  }
}

void QmitkImageLookupTablesPreferencePage::OnImageDataRadioButtonChecked(bool checked)
{
  if (m_Initializing) return;

  if (checked)
  {
    m_PercentageOfDataRangeDoubleSpinBox->setEnabled(true);
  }
  else
  {
    m_PercentageOfDataRangeDoubleSpinBox->setEnabled(false);
  }
}