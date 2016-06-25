/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "IGIUltrasoundOverlayEditorPreferencePage.h"
#include "IGIUltrasoundOverlayEditor.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QRadioButton>
#include <QColorDialog>
#include <QCheckBox>
#include <ctkPathLineEdit.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

const QString IGIUltrasoundOverlayEditorPreferencePage::FIRST_BACKGROUND_STYLE_SHEET("first background color style sheet");
const QString IGIUltrasoundOverlayEditorPreferencePage::SECOND_BACKGROUND_STYLE_SHEET("second background color style sheet");
const QString IGIUltrasoundOverlayEditorPreferencePage::FIRST_BACKGROUND_COLOUR("first background color");
const QString IGIUltrasoundOverlayEditorPreferencePage::SECOND_BACKGROUND_COLOUR("second background color");
const QString IGIUltrasoundOverlayEditorPreferencePage::CLIP_TO_IMAGE_PLANE("clip to imae plane");

//-----------------------------------------------------------------------------
IGIUltrasoundOverlayEditorPreferencePage::IGIUltrasoundOverlayEditorPreferencePage()
: m_MainControl(0)
, m_ColorButton1(NULL)
, m_ColorButton2(NULL)
, m_ClipToImagePlane(NULL)
{
}


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::Init(berry::IWorkbench::Pointer )
{
}


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  m_IGIUltrasoundOverlayEditorPreferencesNode = prefService->GetSystemPreferences()->Node(IGIUltrasoundOverlayEditor::EDITOR_ID);

  m_MainControl = new QWidget(parent);

  QFormLayout *formLayout = new QFormLayout;

  m_ClipToImagePlane = new QCheckBox();
  formLayout->addRow("image tracking clipping planes", m_ClipToImagePlane);
  
  // gradient background
  QLabel* gBName = new QLabel;
  gBName->setText("gradient background");
  formLayout->addRow(gBName);

  // color
  m_ColorButton1 = new QPushButton;
  m_ColorButton1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

  m_ColorButton2 = new QPushButton;
  m_ColorButton2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

  QPushButton* resetButton = new QPushButton;
  resetButton->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
  resetButton->setText("reset");

  QLabel* colorLabel1 = new QLabel("first color : ");
  colorLabel1->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

  QLabel* colorLabel2 = new QLabel("second color: ");
  colorLabel2->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

  QHBoxLayout* colorWidgetLayout = new QHBoxLayout;
  colorWidgetLayout->setContentsMargins(4,4,4,4);
  colorWidgetLayout->addWidget(colorLabel1);
  colorWidgetLayout->addWidget(m_ColorButton1);
  colorWidgetLayout->addWidget(colorLabel2);
  colorWidgetLayout->addWidget(m_ColorButton2);
  colorWidgetLayout->addWidget(resetButton);

  QWidget* colorWidget = new QWidget;
  colorWidget->setLayout(colorWidgetLayout);

  // spacer
  QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  QVBoxLayout* vBoxLayout = new QVBoxLayout;
  vBoxLayout->addLayout(formLayout);
  vBoxLayout->addWidget(colorWidget);
  vBoxLayout->addSpacerItem(spacer);

  m_MainControl->setLayout(vBoxLayout);

  QObject::connect( m_ColorButton1, SIGNAL( clicked() )
    , this, SLOT( FirstColorChanged() ) );

  QObject::connect( m_ColorButton2, SIGNAL( clicked() )
    , this, SLOT( SecondColorChanged() ) );

  QObject::connect( resetButton, SIGNAL( clicked() )
    , this, SLOT( ResetColors() ) );

  this->Update();
}


//-----------------------------------------------------------------------------
QWidget* IGIUltrasoundOverlayEditorPreferencePage::GetQtControl() const
{
  return m_MainControl;
}


//-----------------------------------------------------------------------------
bool IGIUltrasoundOverlayEditorPreferencePage::PerformOk()
{
  m_IGIUltrasoundOverlayEditorPreferencesNode->Put(IGIUltrasoundOverlayEditorPreferencePage::FIRST_BACKGROUND_STYLE_SHEET, m_FirstColorStyleSheet);
  m_IGIUltrasoundOverlayEditorPreferencesNode->Put(IGIUltrasoundOverlayEditorPreferencePage::SECOND_BACKGROUND_STYLE_SHEET, m_SecondColorStyleSheet);
  m_IGIUltrasoundOverlayEditorPreferencesNode->Put(IGIUltrasoundOverlayEditorPreferencePage::FIRST_BACKGROUND_COLOUR, m_FirstColor);
  m_IGIUltrasoundOverlayEditorPreferencesNode->Put(IGIUltrasoundOverlayEditorPreferencePage::SECOND_BACKGROUND_COLOUR, m_SecondColor);
  m_IGIUltrasoundOverlayEditorPreferencesNode->PutBool(IGIUltrasoundOverlayEditorPreferencePage::CLIP_TO_IMAGE_PLANE, m_ClipToImagePlane->isChecked());
  return true;
}


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::PerformCancel()
{

}


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::Update()
{
  m_FirstColorStyleSheet = m_IGIUltrasoundOverlayEditorPreferencesNode->Get(IGIUltrasoundOverlayEditorPreferencePage::FIRST_BACKGROUND_STYLE_SHEET, "");
  m_SecondColorStyleSheet = m_IGIUltrasoundOverlayEditorPreferencesNode->Get(IGIUltrasoundOverlayEditorPreferencePage::SECOND_BACKGROUND_STYLE_SHEET, "");
  m_FirstColor = m_IGIUltrasoundOverlayEditorPreferencesNode->Get(IGIUltrasoundOverlayEditorPreferencePage::FIRST_BACKGROUND_COLOUR, "");
  m_SecondColor = m_IGIUltrasoundOverlayEditorPreferencesNode->Get(IGIUltrasoundOverlayEditorPreferencePage::SECOND_BACKGROUND_COLOUR, "");
  if (m_FirstColorStyleSheet=="")
  {
    m_FirstColorStyleSheet = "background-color:rgb(0,0,0)";
  }
  if (m_SecondColorStyleSheet=="")
  {
    m_SecondColorStyleSheet = "background-color:rgb(0,0,0)";
  }
  if (m_FirstColor=="")
  {
    m_FirstColor = "#000000";
  }
  if (m_SecondColor=="")
  {
    m_SecondColor = "#000000";
  }
  m_ColorButton1->setStyleSheet(m_FirstColorStyleSheet);
  m_ColorButton2->setStyleSheet(m_SecondColorStyleSheet);

  m_ClipToImagePlane->setChecked(m_IGIUltrasoundOverlayEditorPreferencesNode->GetBool(IGIUltrasoundOverlayEditorPreferencePage::CLIP_TO_IMAGE_PLANE, true));
}


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::FirstColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_ColorButton1->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_ColorButton1->setStyleSheet(styleSheet);

  m_FirstColorStyleSheet = styleSheet;
  QStringList firstColor;
  firstColor << color.name();
  m_FirstColor = firstColor.replaceInStrings(";","\\;").join(";");
 }


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::SecondColorChanged()
{
  QColor color = QColorDialog::getColor();
  m_ColorButton2->setAutoFillBackground(true);
  QString styleSheet = "background-color:rgb(";

  styleSheet.append(QString::number(color.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(color.blue()));
  styleSheet.append(")");
  m_ColorButton2->setStyleSheet(styleSheet);

  m_SecondColorStyleSheet = styleSheet;
  QStringList secondColor;
  secondColor << color.name();
  m_SecondColor = secondColor.replaceInStrings(";","\\;").join(";");
 }


//-----------------------------------------------------------------------------
void IGIUltrasoundOverlayEditorPreferencePage::ResetColors()
{
  m_FirstColorStyleSheet = "background-color:rgb(0,0,0)";
  m_SecondColorStyleSheet = "background-color:rgb(0,0,0)";
  m_FirstColor = "#000000";
  m_SecondColor = "#000000";
  m_ColorButton1->setStyleSheet(m_FirstColorStyleSheet);
  m_ColorButton2->setStyleSheet(m_SecondColorStyleSheet);
}

