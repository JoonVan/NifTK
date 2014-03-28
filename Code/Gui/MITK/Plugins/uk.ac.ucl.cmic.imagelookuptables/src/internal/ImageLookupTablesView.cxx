/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "ImageLookupTablesView.h"
#include <QButtonGroup>
#include <QSlider>
#include <QDebug>
#include <itkImage.h>
#include <itkCommand.h>
#include <itkStatisticsImageFilter.h>
#include <itkEventObject.h>
#include <vtkLookupTable.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkNamedLookupTableProperty.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingModeProperty.h>
#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include "QmitkImageLookupTablesPreferencePage.h"
#include <LookupTableManager.h>
#include <LookupTableContainer.h>

#include <mitkLevelWindowManager.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>

const std::string ImageLookupTablesView::VIEW_ID = "uk.ac.ucl.cmic.imagelookuptables";

//-----------------------------------------------------------------------------
ImageLookupTablesView::ImageLookupTablesView()
: m_Controls(0)
, m_LookupTableManager(0)
, m_CurrentNode(0)
, m_CurrentImage(0)
, m_Precision(2)
, m_InUpdate(false)
, m_ThresholdForIntegerBehaviour(50)
, m_LevelWindowPropertyObserverTag(0)
, m_LowestIsOpaquePropertyObserverTag(0)
, m_HighestIsOpaquePropertyObserverTag(0)
{
  m_LookupTableManager = new LookupTableManager();
}


//-----------------------------------------------------------------------------
ImageLookupTablesView::ImageLookupTablesView(const ImageLookupTablesView& other)
: berry::Object()
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
ImageLookupTablesView::~ImageLookupTablesView()
{
  this->Unregister();

  if (m_Controls != NULL)
  {
    delete m_Controls;
  }

  if (m_LookupTableManager != NULL)
  {
    delete m_LookupTableManager;
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // Create UI.
    m_Controls = new Ui::ImageLookupTablesViewControls();
    m_Controls->setupUi(parent);

    // Set defaults on controls
    this->EnableControls(false);

    // Decide which group boxes are open/closed.
    m_Controls->m_RangeGroupBox->setCollapsed(false);
    m_Controls->m_LimitsGroupBox->setCollapsed(true);

    // Populate combo box with lookup table names.
    for (unsigned int i = 0; i < m_LookupTableManager->GetNumberOfLookupTables(); i++)
    {
      const LookupTableContainer *container = m_LookupTableManager->GetLookupTableContainer(i);
      m_Controls->m_LookupTableComboBox->insertItem(container->GetOrder(), container->GetDisplayName());
    }

    /// This is probably superfluous because the AbstractView::AfterCreateQtPartControl() calls
    /// OnPreferencesChanged that calls RetrievePreferenceValues. It would need testing.
    this->RetrievePreferenceValues();

    this->CreateConnections();
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::CreateConnections()
{
  this->connect(m_Controls->m_MinSlider, SIGNAL(valueChanged(double)), SLOT(OnWindowBoundsChanged()));
  this->connect(m_Controls->m_MaxSlider, SIGNAL(valueChanged(double)), SLOT(OnWindowBoundsChanged()));
  this->connect(m_Controls->m_LevelSlider, SIGNAL(valueChanged(double)), SLOT(OnLevelWindowChanged()));
  this->connect(m_Controls->m_WindowSlider, SIGNAL(valueChanged(double)), SLOT(OnLevelWindowChanged()));
  this->connect(m_Controls->m_MinLimitDoubleSpinBox, SIGNAL(editingFinished()), SLOT(OnRangeChanged()));
  this->connect(m_Controls->m_MaxLimitDoubleSpinBox, SIGNAL(editingFinished()), SLOT(OnRangeChanged()));
  this->connect(m_Controls->m_LookupTableComboBox, SIGNAL(currentIndexChanged(int)), SLOT(OnLookupTableComboBoxChanged(int)));
  this->connect(m_Controls->m_ResetButton, SIGNAL(pressed()), this, SLOT(OnResetButtonPressed()));
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
  RetrievePreferenceValues();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::RetrievePreferenceValues()
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(VIEW_ID))
        .Cast<berry::IBerryPreferences>();
  assert( prefs );

  m_Precision = prefs->GetInt(QmitkImageLookupTablesPreferencePage::PRECISION_NAME, 2);

  if (m_CurrentNode.IsNull())
  {
    this->BlockSignals(true);
  }

  m_Controls->m_MinSlider->setDecimals(m_Precision);
  m_Controls->m_MaxSlider->setDecimals(m_Precision);
  m_Controls->m_LevelSlider->setDecimals(m_Precision);
  m_Controls->m_WindowSlider->setDecimals(m_Precision);
  m_Controls->m_MinLimitDoubleSpinBox->setDecimals(m_Precision);
  m_Controls->m_MaxLimitDoubleSpinBox->setDecimals(m_Precision);

  if (m_CurrentNode.IsNull())
  {
    this->BlockSignals(false);
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::SetFocus()
{
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::EnableControls(bool b)
{
  m_Controls->m_LookupTableComboBox->setEnabled(b);
  m_Controls->m_MinSlider->setEnabled(b);
  m_Controls->m_MaxSlider->setEnabled(b);
  m_Controls->m_WindowSlider->setEnabled(b);
  m_Controls->m_LevelSlider->setEnabled(b);
  m_Controls->m_MinLimitDoubleSpinBox->setEnabled(b);
  m_Controls->m_MaxLimitDoubleSpinBox->setEnabled(b);
  m_Controls->m_ResetButton->setEnabled(b);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::BlockSignals(bool b)
{
  m_Controls->m_MinSlider->blockSignals(b);
  m_Controls->m_MaxSlider->blockSignals(b);
  m_Controls->m_WindowSlider->blockSignals(b);
  m_Controls->m_LevelSlider->blockSignals(b);
  m_Controls->m_MinLimitDoubleSpinBox->blockSignals(b);
  m_Controls->m_MaxLimitDoubleSpinBox->blockSignals(b);
  m_Controls->m_ResetButton->blockSignals(b);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{

  bool isValid = this->IsSelectionValid(nodes);

  if (!isValid
      || (nodes[0].IsNotNull() && nodes[0] != m_CurrentNode)
     )
  {
    this->Unregister();
  }

  if (isValid)
  {
    this->Register(nodes[0]);
  }

  this->EnableControls(isValid);
}


//-----------------------------------------------------------------------------
bool ImageLookupTablesView::IsSelectionValid(const QList<mitk::DataNode::Pointer>& nodes)
{
  bool isValid = true;

  if (nodes.count() != 1)
  {
    isValid = false;
  }

  // All nodes must be non null, non-helper images.
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if(node.IsNull())
    {
      isValid = false;
    }

    if (node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) == NULL)
    {
      isValid = false;
    }

    bool isHelper(false);
    if (node->GetBoolProperty("helper object", isHelper) && isHelper)
    {
      isValid = false;
    }

    bool isSelected(false);
    node->GetBoolProperty("selected", isSelected);
    if (!isSelected)
    {
      isValid = false;
    }

    if (!node->GetProperty("levelwindow"))
    {
      isValid = false;
    }

  }

  return isValid;
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::Activated()
{
  QmitkBaseView::Activated();

  berry::IWorkbenchPart::Pointer nullPart;
  this->OnSelectionChanged(nullPart, this->GetDataManagerSelection());
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::Register(const mitk::DataNode::Pointer node)
{
  if (node.IsNotNull())
  {
    m_CurrentNode = node;

    this->DifferentImageSelected();
    this->OnRangeChanged();
    this->OnPropertyChanged();

    itk::ReceptorMemberCommand<ImageLookupTablesView>::Pointer command
      = itk::ReceptorMemberCommand<ImageLookupTablesView>::New();
    command->SetCallbackFunction(this, &ImageLookupTablesView::OnPropertyChanged);
    mitk::BaseProperty::Pointer property = node->GetProperty("levelwindow");
    m_LevelWindowPropertyObserverTag = property->AddObserver(itk::ModifiedEvent(), command);

    itk::ReceptorMemberCommand<ImageLookupTablesView>::Pointer lowestIsOpaqueCommand
      = itk::ReceptorMemberCommand<ImageLookupTablesView>::New();
    lowestIsOpaqueCommand->SetCallbackFunction(this, &ImageLookupTablesView::OnLookupTablePropertyChanged);
    mitk::BaseProperty::Pointer lowestIsOpaqueProperty = node->GetProperty("Image Rendering.Lowest Value Is Opaque");
    m_LowestIsOpaquePropertyObserverTag = lowestIsOpaqueProperty->AddObserver(itk::ModifiedEvent(), command);

    itk::ReceptorMemberCommand<ImageLookupTablesView>::Pointer highestIsOpaqueCommand
      = itk::ReceptorMemberCommand<ImageLookupTablesView>::New();
    highestIsOpaqueCommand->SetCallbackFunction(this, &ImageLookupTablesView::OnLookupTablePropertyChanged);
    mitk::BaseProperty::Pointer highestIsOpaqueProperty = node->GetProperty("Image Rendering.Highest Value Is Opaque");
    m_HighestIsOpaquePropertyObserverTag = highestIsOpaqueProperty->AddObserver(itk::ModifiedEvent(), command);
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::Unregister()
{
  if (m_CurrentNode.IsNotNull())
  {
    mitk::BaseProperty::Pointer property = m_CurrentNode->GetProperty("levelwindow");
    property->RemoveObserver(m_LevelWindowPropertyObserverTag);

    mitk::BaseProperty::Pointer lowestIsOpaqueProperty = m_CurrentNode->GetProperty("Image Rendering.Lowest Value Is Opaque");
    lowestIsOpaqueProperty->RemoveObserver(m_LowestIsOpaquePropertyObserverTag);

    mitk::BaseProperty::Pointer highestIsOpaqueProperty = m_CurrentNode->GetProperty("Image Rendering.Highest Value Is Opaque");
    highestIsOpaqueProperty->RemoveObserver(m_HighestIsOpaquePropertyObserverTag);

    m_CurrentNode = NULL;
  }
}

//-----------------------------------------------------------------------------
void ImageLookupTablesView::DifferentImageSelected()
{
  this->BlockSignals(true);

  m_CurrentImage = dynamic_cast<mitk::Image*>(m_CurrentNode->GetData());

  // As the NiftyView application level plugin provides a mitk::LevelWindow, it MUST be present.
  mitk::LevelWindow levelWindow;
  m_CurrentNode->GetLevelWindow(levelWindow);

  float minDataLimit(0);
  float maxDataLimit(0);
  int lookupTableIndex(0);

  m_CurrentNode->GetFloatProperty("image data min", minDataLimit);
  m_CurrentNode->GetFloatProperty("image data max", maxDataLimit);
  bool lookupTableIndexFound = m_CurrentNode->GetIntProperty("LookupTableIndex", lookupTableIndex);

  m_Controls->m_MinLimitDoubleSpinBox->setValue(minDataLimit);
  m_Controls->m_MaxLimitDoubleSpinBox->setValue(maxDataLimit);

  if (lookupTableIndexFound)
  {
    m_Controls->m_LookupTableComboBox->setCurrentIndex(lookupTableIndex);
  }
  else
  {
    m_Controls->m_LookupTableComboBox->setCurrentIndex(0);
  }

  this->BlockSignals(false);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnRangeChanged()
{
  this->BlockSignals(true);

  mitk::LevelWindow levelWindow;
  m_CurrentNode->GetLevelWindow(levelWindow);

  levelWindow.SetRangeMinMax(m_Controls->m_MinLimitDoubleSpinBox->value(), m_Controls->m_MaxLimitDoubleSpinBox->value());

  double rangeMin = levelWindow.GetRangeMin();
  double rangeMax = levelWindow.GetRangeMax();
  double range = levelWindow.GetRange();

  // Trac 1680 - don't forget, MIDAS generally deals with integer images
  // so the user requirements are such that they must be able to change
  // intensity ranges in steps of 1. If however, we are using float images
  // we will need to be able to change intensity values in much smaller stepps.
  double singleStep;
  double pageStep;

  if (fabs(rangeMin - rangeMax) > m_ThresholdForIntegerBehaviour)
  {
    // i.e. we have a large enough range to use integer page step and single step.
    singleStep = 1;
    pageStep = 10;
  }
  else
  {
    // i.e. in this case, use fractions.
    singleStep = range / 100.0;
    pageStep = range / 10.0;
  }

  m_Controls->m_MinSlider->setMinimum(rangeMin);
  m_Controls->m_MinSlider->setMaximum(rangeMax);
  m_Controls->m_MaxSlider->setMinimum(rangeMin);
  m_Controls->m_MaxSlider->setMaximum(rangeMax);
  m_Controls->m_MinSlider->setSingleStep(singleStep);
  m_Controls->m_MinSlider->setTickInterval(singleStep);
  m_Controls->m_MinSlider->setPageStep(pageStep);
  m_Controls->m_MaxSlider->setSingleStep(singleStep);
  m_Controls->m_MaxSlider->setTickInterval(singleStep);
  m_Controls->m_MaxSlider->setPageStep(pageStep);
  m_Controls->m_WindowSlider->setMinimum(0);
  m_Controls->m_WindowSlider->setMaximum(range);
  m_Controls->m_WindowSlider->setSingleStep(singleStep);
  m_Controls->m_LevelSlider->setMinimum(rangeMin);
  m_Controls->m_LevelSlider->setMaximum(rangeMax);
  m_Controls->m_LevelSlider->setSingleStep(singleStep);

  this->BlockSignals(false);
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnPropertyChanged(const itk::EventObject&)
{
  this->OnPropertyChanged();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnPropertyChanged()
{
  this->UpdateGuiFromLevelWindow();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::UpdateGuiFromLevelWindow()
{
  this->BlockSignals(true);

  mitk::LevelWindow levelWindow;
  m_CurrentNode->GetLevelWindow(levelWindow);

  double min = levelWindow.GetLowerWindowBound();
  double max = levelWindow.GetUpperWindowBound();
  double level = levelWindow.GetLevel();
  double window = levelWindow.GetWindow();

  m_Controls->m_MinSlider->setValue(min);
  m_Controls->m_MaxSlider->setValue(max);
  m_Controls->m_LevelSlider->setValue(level);
  m_Controls->m_WindowSlider->setValue(window);

  this->BlockSignals(false);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnWindowBoundsChanged()
{
  // Get the current values
  mitk::LevelWindow levelWindow;
  m_CurrentNode->GetLevelWindow(levelWindow);

  // Note: This method is called when one of the sliders has been moved
  // So, it's purpose is to update the other sliders to match.

  // Update them from controls
  double min = m_Controls->m_MinSlider->value();
  double max = m_Controls->m_MaxSlider->value();

  levelWindow.SetWindowBounds(min, max);
  m_CurrentNode->SetLevelWindow(levelWindow);

  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnLevelWindowChanged()
{
  // Get the current values
  mitk::LevelWindow levelWindow;
  m_CurrentNode->GetLevelWindow(levelWindow);

  // Note: This method is called when one of the sliders has been moved
  // So, it's purpose is to update the other sliders to match.

  // Update them from controls
  double window = m_Controls->m_WindowSlider->value();
  double level = m_Controls->m_LevelSlider->value();

  levelWindow.SetLevelWindow(level, window);
  m_CurrentNode->SetLevelWindow(levelWindow);

  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnLookupTablePropertyChanged(const itk::EventObject&)
{
  int comboIndex;
  m_CurrentNode->GetIntProperty("LookupTableIndex", comboIndex);
  this->OnLookupTableComboBoxChanged(comboIndex);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnLookupTableComboBoxChanged(int comboBoxIndex)
{
  if (m_CurrentNode.IsNotNull())
  {
    // Copy the vtkLookupTable
    const LookupTableContainer* lutContainer = m_LookupTableManager->GetLookupTableContainer(comboBoxIndex);
    vtkLookupTable *vtkLUT = m_LookupTableManager->CloneLookupTable(comboBoxIndex);

    // Set the opacity flags.
    bool lowestIsOpaque = true;
    m_CurrentNode->GetBoolProperty("Image Rendering.Lowest Value Is Opaque", lowestIsOpaque);
    if (!lowestIsOpaque)
    {
      double *rgba = vtkLUT->GetTableValue(0);
      rgba[3] = 0;
    }

    bool highestIsOpaque = true;
    m_CurrentNode->GetBoolProperty("Image Rendering.Highest Value Is Opaque", highestIsOpaque);
    if (!highestIsOpaque)
    {
      double *rgba = vtkLUT->GetTableValue(vtkLUT->GetNumberOfColors()-1);
      rgba[3] = 0;
    }

    mitk::LookupTable::Pointer mitkLUT = mitk::LookupTable::New();
    mitkLUT->SetVtkLookupTable(const_cast<vtkLookupTable*>(vtkLUT));
    const std::string& lutName = lutContainer->GetDisplayName().toStdString();
    mitk::NamedLookupTableProperty::Pointer mitkLUTProperty = mitk::NamedLookupTableProperty::New(lutName, mitkLUT);

    // and give to the node property.
    m_CurrentNode->ReplaceProperty("LookupTable", mitkLUTProperty);
    mitk::RenderingModeProperty::Pointer renderProp = mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
    m_CurrentNode->SetProperty("Image Rendering.Mode", renderProp);
    m_CurrentNode->SetIntProperty("LookupTableIndex", comboBoxIndex);

    // Force redraw.
    m_CurrentNode->Update();
    this->RequestRenderWindowUpdate();
  }
}



//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnResetButtonPressed()
{

  mitk::LevelWindow levelWindow;
  m_CurrentNode->GetLevelWindow(levelWindow);

  float rangeMin(0);
  float rangeMax(0);

  if (m_CurrentNode->GetFloatProperty("image data min", rangeMin)
      && m_CurrentNode->GetFloatProperty("image data max", rangeMax))
  {
    levelWindow.SetRangeMinMax(rangeMin, rangeMax);
    levelWindow.SetWindowBounds(rangeMin, rangeMax);

    m_Controls->m_MinLimitDoubleSpinBox->setValue(rangeMin);
    m_Controls->m_MaxLimitDoubleSpinBox->setValue(rangeMax);

    m_CurrentNode->SetLevelWindow(levelWindow);
    m_CurrentNode->Modified();

    this->UpdateGuiFromLevelWindow();
    this->RequestRenderWindowUpdate();
  }
}

