/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "MIDASMorphologicalSegmentorViewPreferencePage.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QMessageBox>
#include <QPushButton>
#include <QColorDialog>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <QmitkMIDASBaseSegmentationFunctionality.h>

const QString MIDASMorphologicalSegmentorViewPreferencePage::PREFERENCES_NODE_NAME("/uk_ac_ucl_cmic_midasmorphologicalsegmentor");


//-----------------------------------------------------------------------------
MIDASMorphologicalSegmentorViewPreferencePage::MIDASMorphologicalSegmentorViewPreferencePage()
: m_MainControl(0)
, m_Initializing(false)
{

}


//-----------------------------------------------------------------------------
MIDASMorphologicalSegmentorViewPreferencePage::MIDASMorphologicalSegmentorViewPreferencePage(const MIDASMorphologicalSegmentorViewPreferencePage& other)
: berry::Object(), QObject()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
MIDASMorphologicalSegmentorViewPreferencePage::~MIDASMorphologicalSegmentorViewPreferencePage()
{

}


//-----------------------------------------------------------------------------
void MIDASMorphologicalSegmentorViewPreferencePage::Init(berry::IWorkbench::Pointer )
{

}


//-----------------------------------------------------------------------------
void MIDASMorphologicalSegmentorViewPreferencePage::CreateQtControl(QWidget* parent)
{
  m_Initializing = true;
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_MIDASMorphologicalSegmentorViewPreferencesNode = prefService->GetSystemPreferences()->Node(PREFERENCES_NODE_NAME);

  m_MainControl = new QWidget(parent);

  QFormLayout *formLayout = new QFormLayout;

  QPushButton* defaultColorResetButton = new QPushButton;
  defaultColorResetButton->setText("reset");

  m_DefaultColorPushButton = new QPushButton;
  m_DefaultColorPushButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

  QGridLayout* defaultColorLayout = new QGridLayout;
  defaultColorLayout->setContentsMargins(4,4,4,4);
  defaultColorLayout->addWidget(m_DefaultColorPushButton, 0, 0);
  defaultColorLayout->addWidget(defaultColorResetButton, 0, 1);

  formLayout->addRow("default outline colour", defaultColorLayout);

  m_MainControl->setLayout(formLayout);

  QObject::connect( m_DefaultColorPushButton, SIGNAL( clicked() ), this, SLOT( OnDefaultColourChanged() ) );
  QObject::connect( defaultColorResetButton, SIGNAL( clicked() ), this, SLOT( OnResetDefaultColour() ) );

  this->Update();

  m_Initializing = false;
}


//-----------------------------------------------------------------------------
QWidget* MIDASMorphologicalSegmentorViewPreferencePage::GetQtControl() const
{
  return m_MainControl;
}


//-----------------------------------------------------------------------------
bool MIDASMorphologicalSegmentorViewPreferencePage::PerformOk()
{
  m_MIDASMorphologicalSegmentorViewPreferencesNode->Put(QmitkMIDASBaseSegmentationFunctionality::DEFAULT_COLOUR_STYLE_SHEET, m_DefauleColorStyleSheet);
  m_MIDASMorphologicalSegmentorViewPreferencesNode->Put(QmitkMIDASBaseSegmentationFunctionality::DEFAULT_COLOUR, m_DefaultColor);

  return true;
}


//-----------------------------------------------------------------------------
void MIDASMorphologicalSegmentorViewPreferencePage::PerformCancel()
{
}


//-----------------------------------------------------------------------------
void MIDASMorphologicalSegmentorViewPreferencePage::Update()
{
  m_DefauleColorStyleSheet = m_MIDASMorphologicalSegmentorViewPreferencesNode->Get(QmitkMIDASBaseSegmentationFunctionality::DEFAULT_COLOUR_STYLE_SHEET, "");
  m_DefaultColor = m_MIDASMorphologicalSegmentorViewPreferencesNode->GetByteArray(QmitkMIDASBaseSegmentationFunctionality::DEFAULT_COLOUR, "");
  if (m_DefauleColorStyleSheet=="")
  {
    m_DefauleColorStyleSheet = "background-color: rgb(0,255,0)";
  }
  if (m_DefaultColor=="")
  {
    m_DefaultColor = "#00ff00";
  }
  m_DefaultColorPushButton->setStyleSheet(m_DefauleColorStyleSheet);
}


//-----------------------------------------------------------------------------
void MIDASMorphologicalSegmentorViewPreferencePage::OnDefaultColourChanged()
{
  QColor colour = QColorDialog::getColor();
  if (colour.isValid())
  {
    m_DefaultColorPushButton->setAutoFillBackground(true);

    QString styleSheet = "background-color: rgb(";
    styleSheet.append(QString::number(colour.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(colour.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(colour.blue()));
    styleSheet.append(")");

    m_DefaultColorPushButton->setStyleSheet(styleSheet);
    m_DefauleColorStyleSheet = styleSheet;

    QStringList defColor;
    defColor << colour.name();

    m_DefaultColor = defColor.replaceInStrings(";","\\;").join(";");
  }
}


//-----------------------------------------------------------------------------
void MIDASMorphologicalSegmentorViewPreferencePage::OnResetDefaultColour()
{
  m_DefauleColorStyleSheet = "background-color: rgb(0,255,0)";
  m_DefaultColor = "#00ff00";
  m_DefaultColorPushButton->setStyleSheet(m_DefauleColorStyleSheet);
}
