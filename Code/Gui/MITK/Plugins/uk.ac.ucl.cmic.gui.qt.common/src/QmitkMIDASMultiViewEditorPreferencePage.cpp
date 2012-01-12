/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-12-02 06:46:00 +0000 (Fri, 02 Dec 2011) $
 Revision          : $Revision: 7905 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkMIDASMultiViewEditorPreferencePage.h"
#include "QmitkMIDASMultiViewEditor.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QColorDialog>
#include <QComboBox>
#include <QSpinBox>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

const std::string QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_NUMBER_ROWS("midas default number of rows");
const std::string QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_NUMBER_COLUMNS("midas default number of columns");
const std::string QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_ORIENTATION("midas default orientation");
const std::string QmitkMIDASMultiViewEditorPreferencePage::MIDAS_DEFAULT_IMAGE_INTERPOLATION("midas default image interpolation");
const std::string QmitkMIDASMultiViewEditorPreferencePage::MIDAS_BACKGROUND_COLOUR("midas background colour");
const std::string QmitkMIDASMultiViewEditorPreferencePage::MIDAS_BACKGROUND_COLOUR_STYLESHEET("midas background colour stylesheet");

QmitkMIDASMultiViewEditorPreferencePage::QmitkMIDASMultiViewEditorPreferencePage()
: m_MainControl(0)
, m_DefaultNumberOfRowsSpinBox(NULL)
, m_DefaultNumberOfColumnsSpinBox(NULL)
, m_DefaultOrientationComboBox(NULL)
, m_ImageInterpolationComboBox(NULL)
, m_BackgroundColourButton(NULL)
{

}

QmitkMIDASMultiViewEditorPreferencePage::QmitkMIDASMultiViewEditorPreferencePage(const QmitkMIDASMultiViewEditorPreferencePage& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void QmitkMIDASMultiViewEditorPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void QmitkMIDASMultiViewEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_MIDASMultiViewEditorPreferencesNode = prefService->GetSystemPreferences()->Node(QmitkMIDASMultiViewEditor::EDITOR_ID);

  m_MainControl = new QWidget(parent);

  QFormLayout *formLayout = new QFormLayout;

  m_ImageInterpolationComboBox = new QComboBox(parent);
  formLayout->addRow("image interpolation", m_ImageInterpolationComboBox);
  m_ImageInterpolationComboBox->insertItem(0, "none");
  m_ImageInterpolationComboBox->insertItem(1, "linear");
  m_ImageInterpolationComboBox->insertItem(2, "cubic");

  m_DefaultNumberOfRowsSpinBox = new QSpinBox(parent);
  m_DefaultNumberOfRowsSpinBox->setMinimum(1);
  m_DefaultNumberOfRowsSpinBox->setMaximum(5);
  formLayout->addRow("initial number of rows", m_DefaultNumberOfRowsSpinBox);

  m_DefaultNumberOfColumnsSpinBox = new QSpinBox(parent);
  m_DefaultNumberOfColumnsSpinBox->setMinimum(1);
  m_DefaultNumberOfColumnsSpinBox->setMaximum(5);

  formLayout->addRow("initial number of columns", m_DefaultNumberOfColumnsSpinBox);

  m_DefaultOrientationComboBox = new QComboBox(parent);
  formLayout->addRow("default orientation", m_DefaultOrientationComboBox);
  m_DefaultOrientationComboBox->insertItem(0, "axial");
  m_DefaultOrientationComboBox->insertItem(1, "sagittal");
  m_DefaultOrientationComboBox->insertItem(2, "coronal");
  m_DefaultOrientationComboBox->insertItem(3, "as acquired (XY plane)");

  QPushButton* backgroundColourResetButton = new QPushButton;
  backgroundColourResetButton->setText("reset");

  m_BackgroundColourButton = new QPushButton;
  m_BackgroundColourButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

  QGridLayout* backgroundColourWidgetLayout = new QGridLayout;
  backgroundColourWidgetLayout->setContentsMargins(4,4,4,4);
  backgroundColourWidgetLayout->addWidget(m_BackgroundColourButton, 0, 0);
  backgroundColourWidgetLayout->addWidget(backgroundColourResetButton, 0, 1);

  formLayout->addRow("background colour", backgroundColourWidgetLayout);

  m_MainControl->setLayout(formLayout);

  QObject::connect( m_BackgroundColourButton, SIGNAL( clicked() ), this, SLOT( OnBackgroundColourChanged() ) );
  QObject::connect( backgroundColourResetButton, SIGNAL( clicked() ), this, SLOT( OnResetBackgroundColour() ) );

  this->Update();
}

QWidget* QmitkMIDASMultiViewEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkMIDASMultiViewEditorPreferencePage::PerformOk()
{
  m_MIDASMultiViewEditorPreferencesNode->Put(MIDAS_BACKGROUND_COLOUR_STYLESHEET, m_BackgroundColorStyleSheet.toStdString());
  m_MIDASMultiViewEditorPreferencesNode->PutByteArray(MIDAS_BACKGROUND_COLOUR, m_BackgroundColor);
  m_MIDASMultiViewEditorPreferencesNode->PutInt(MIDAS_DEFAULT_NUMBER_ROWS, m_DefaultNumberOfRowsSpinBox->value());
  m_MIDASMultiViewEditorPreferencesNode->PutInt(MIDAS_DEFAULT_NUMBER_COLUMNS, m_DefaultNumberOfColumnsSpinBox->value());
  m_MIDASMultiViewEditorPreferencesNode->PutInt(MIDAS_DEFAULT_ORIENTATION, m_DefaultOrientationComboBox->currentIndex());
  m_MIDASMultiViewEditorPreferencesNode->PutInt(MIDAS_DEFAULT_IMAGE_INTERPOLATION, m_ImageInterpolationComboBox->currentIndex());
  return true;
}

void QmitkMIDASMultiViewEditorPreferencePage::PerformCancel()
{

}

void QmitkMIDASMultiViewEditorPreferencePage::Update()
{
  m_BackgroundColorStyleSheet = QString::fromStdString(m_MIDASMultiViewEditorPreferencesNode->Get(MIDAS_BACKGROUND_COLOUR_STYLESHEET, ""));
  m_BackgroundColor = m_MIDASMultiViewEditorPreferencesNode->GetByteArray(MIDAS_BACKGROUND_COLOUR, "");
  if (m_BackgroundColorStyleSheet=="")
  {
    m_BackgroundColorStyleSheet = "background-color: rgb(255,250,240)";
  }
  if (m_BackgroundColor=="")
  {
    m_BackgroundColor = "#fffaf0";
  }
  m_BackgroundColourButton->setStyleSheet(m_BackgroundColorStyleSheet);

  m_DefaultNumberOfRowsSpinBox->setValue(m_MIDASMultiViewEditorPreferencesNode->GetInt(MIDAS_DEFAULT_NUMBER_ROWS, 1));
  m_DefaultNumberOfColumnsSpinBox->setValue(m_MIDASMultiViewEditorPreferencesNode->GetInt(MIDAS_DEFAULT_NUMBER_COLUMNS, 1));
  m_DefaultOrientationComboBox->setCurrentIndex(m_MIDASMultiViewEditorPreferencesNode->GetInt(MIDAS_DEFAULT_ORIENTATION, 0));
  m_ImageInterpolationComboBox->setCurrentIndex(m_MIDASMultiViewEditorPreferencesNode->GetInt(MIDAS_DEFAULT_IMAGE_INTERPOLATION, 2));
}

void QmitkMIDASMultiViewEditorPreferencePage::OnBackgroundColourChanged()
{
  QColor colour = QColorDialog::getColor();
  if (colour.isValid())
  {
    m_BackgroundColourButton->setAutoFillBackground(true);

    QString styleSheet = "background-color: rgb(";
    styleSheet.append(QString::number(colour.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(colour.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(colour.blue()));
    styleSheet.append(")");

    m_BackgroundColourButton->setStyleSheet(styleSheet);
    m_BackgroundColorStyleSheet = styleSheet;

    QStringList backgroundColour;
    backgroundColour << colour.name();

    m_BackgroundColor = backgroundColour.replaceInStrings(";","\\;").join(";").toStdString();
  }
 }

void QmitkMIDASMultiViewEditorPreferencePage::OnResetBackgroundColour()
{
  m_BackgroundColorStyleSheet = "background-color: rgb(255,250,240)";
  m_BackgroundColor = "#fffaf0";
  m_BackgroundColourButton->setStyleSheet(m_BackgroundColorStyleSheet);
}
