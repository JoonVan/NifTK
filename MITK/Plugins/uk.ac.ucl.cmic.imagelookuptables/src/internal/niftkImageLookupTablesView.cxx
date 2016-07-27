/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkImageLookupTablesView.h"

#include <QButtonGroup>
#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSignalMapper>
#include <QXmlSimpleReader>

#include <itkCommand.h>
#include <itkEventObject.h>
#include <itkImage.h>
#include <itkStatisticsImageFilter.h>

#include <vtkLookupTable.h>

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkIOUtil.h>
#include <mitkLevelWindowManager.h>
#include <mitkLookupTable.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkRenderingManager.h>
#include <mitkRenderingModeProperty.h>
#include <mitkVtkResliceInterpolationProperty.h>

#include <berryIBerryPreferences.h>
#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleInitialization.h>
#include <usModuleRegistry.h>

#include <niftkDataStorageUtils.h>
#include <niftkLabeledLookupTableProperty.h>
#include <niftkLookupTableContainer.h>
#include <niftkLookupTableSaxHandler.h>
#include <niftkNamedLookupTableProperty.h>

#include <niftkLookupTableManager.h>
#include <niftkLookupTableProviderService.h>
#include <niftkVtkLookupTableUtils.h>

#include "niftkImageLookupTablesViewActivator.h"
#include "niftkImageLookupTablesPreferencePage.h"


namespace niftk
{

const QString ImageLookupTablesView::VIEW_ID = "uk.ac.ucl.cmic.imagelookuptables";

//-----------------------------------------------------------------------------
ImageLookupTablesView::ImageLookupTablesView()
: m_Controls(0)
, m_CurrentNode(0)
, m_CurrentImage(0)
, m_Precision(2)
, m_InUpdate(false)
, m_ThresholdForIntegerBehaviour(50)
, m_LevelWindowPropertyObserverTag(0)
{
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
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // Create UI.
    m_Controls = new Ui::ImageLookupTablesViewControls();
    m_Controls->setupUi(parent);

    // Decide which group boxes are open/closed.
    m_Controls->m_RangeGroupBox->setCollapsed(false);
    m_Controls->m_LimitsGroupBox->setCollapsed(true);
    
    this->EnableScaleControls(false);
    this->EnableLabelControls(false);

    /// This is probably superfluous because the AbstractView::AfterCreateQtPartControl() calls
    /// OnPreferencesChanged that calls RetrievePreferenceValues. It would need testing.
    this->RetrievePreferenceValues();
    this->LoadCachedLookupTables();
    
    this->UpdateLookupTableComboBox();
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
  this->connect(m_Controls->m_SaveButton, SIGNAL(pressed()), this, SLOT(OnSaveButtonPressed()));
  this->connect(m_Controls->m_LoadButton, SIGNAL(pressed()), this, SLOT(OnLoadButtonPressed()));
  this->connect(m_Controls->m_NewButton, SIGNAL(pressed()), this, SLOT(OnNewButtonPressed()));

  this->connect(m_Controls->m_AddLabelButton, SIGNAL(pressed()), this, SLOT(OnAddLabelButtonPressed()));
  this->connect(m_Controls->m_RemoveLabelButton, SIGNAL(pressed()), this, SLOT(OnRemoveLabelButtonPressed()));
  this->connect(m_Controls->m_MoveLabelUpButton, SIGNAL(pressed()), this, SLOT(OnMoveLabelUpButtonPressed()));
  this->connect(m_Controls->m_MoveLabelDownButton, SIGNAL(pressed()), this, SLOT(OnMoveLabelDownButtonPressed()));
  this->connect(m_Controls->widget_LabelTable, SIGNAL(cellChanged(int,int)), this, SLOT(OnLabelMapTableCellChanged(int, int)));
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
  RetrievePreferenceValues();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::RetrievePreferenceValues()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(VIEW_ID))
        .Cast<berry::IBerryPreferences>();
  assert(prefs);

  m_Precision = prefs->GetInt(ImageLookupTablesPreferencePage::PRECISION_NAME, 2);

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
void ImageLookupTablesView::LoadCachedLookupTables()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IBerryPreferences::Pointer prefs
    = (prefService->GetSystemPreferences()->Node(VIEW_ID)).Cast<berry::IBerryPreferences>();
  assert(prefs);
  
  QString cachedFileNames = prefs->Get("LABEL_MAP_NAMES", "");
  if (cachedFileNames.isNull() || cachedFileNames.isEmpty())
  {
    return;
  }

  LookupTableProviderService* lutService
    = ImageLookupTablesViewActivator::GetLookupTableProviderService();
  if (lutService == NULL)
  {
    mitkThrow() << "Failed to find LookupTableProviderService." << std::endl;
  }

  prefs->PutBool("InBlockUpdate", true);

  QStringList labelList = cachedFileNames.split(",");
  QStringList removedItems;
  int skippedItems = 0;

  for (int i = 0; i < labelList.count(); i++)
  {
    QString currLabelName = labelList.at(i);

    if (currLabelName.isNull() || currLabelName.isEmpty() || currLabelName == QString(" "))
    {
      skippedItems++;
      continue;
    }
    
    QString filenameWithPath = prefs->Get(currLabelName, "");
    QString lutName = this->LoadLookupTable(filenameWithPath);
    if (lutName.isEmpty())
    {
      removedItems.append(currLabelName);
    }
  }

  if (removedItems.size() > 0 || skippedItems > 0)
  {
    // Tidy up preferences: remove entries that don't exist
    for (int i = 0; i < removedItems.size(); i++)
    {
      prefs->Remove(removedItems.at(i));
    }

    // Update the list of profile names
    prefs->Put("LABEL_MAP_NAMES", cachedFileNames);
  }

  // End of block update
  prefs->PutBool("InBlockUpdate", false);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::SetFocus()
{
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::EnableControls(bool b)
{
  m_Controls->m_LookupTableComboBox->setEnabled(b);
  m_Controls->m_LoadButton->setEnabled(b);
  m_Controls->m_NewButton->setEnabled(b);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::EnableScaleControls(bool b)
{
  m_Controls->m_MinSlider->setEnabled(b);
  m_Controls->m_MaxSlider->setEnabled(b);
  m_Controls->m_WindowSlider->setEnabled(b);
  m_Controls->m_LevelSlider->setEnabled(b);
  m_Controls->m_MinLimitDoubleSpinBox->setEnabled(b);
  m_Controls->m_MaxLimitDoubleSpinBox->setEnabled(b);
  m_Controls->m_ResetButton->setEnabled(b);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::EnableLabelControls(bool b)
{
  m_Controls->m_SaveButton->setEnabled(b);
  m_Controls->m_AddLabelButton->setEnabled(b);
  m_Controls->m_RemoveLabelButton->setEnabled(b);
  m_Controls->m_MoveLabelUpButton->setEnabled(b);
  m_Controls->m_MoveLabelDownButton->setEnabled(b);
  m_Controls->widget_LabelTable->setEnabled(b);

  if (b)
  {
    this->UpdateLabelMapTable();
  }
  else
  {
    m_Controls->widget_LabelTable->clearContents();
    m_Controls->widget_LabelTable->setRowCount(0);
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::BlockSignals(bool b)
{
  m_Controls->m_MinSlider->blockSignals(b);
  m_Controls->m_MaxSlider->blockSignals(b);
  m_Controls->m_WindowSlider->blockSignals(b);
  m_Controls->m_LookupTableComboBox->blockSignals(b);
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

  if (!isValid || (nodes[0].IsNotNull() && nodes[0] != m_CurrentNode))
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
  if (nodes.count() != 1)
  {
    return false;
  }

  mitk::DataNode::Pointer node = nodes.at(0);
  // All nodes must be non null, non-helper images.
  if (node.IsNull())
  {
    return false;
  }
  else if ( dynamic_cast<mitk::Image*>(node->GetData()) == NULL)
  {
    return false;
  }

  bool isHelper(false);
  if (node->GetBoolProperty("helper object", isHelper) && isHelper)
  {
    return false;
  }

  bool isSelected(false);
  node->GetBoolProperty("selected", isSelected);
  if (!isSelected)
  {
    return false;
  }

  bool isValid = niftk::IsNodeAGreyScaleImage(node);
  return isValid;
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::Activated()
{
  BaseView::Activated();

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
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::Unregister()
{
  if (m_CurrentNode.IsNotNull())
  {
    mitk::BaseProperty::Pointer property = m_CurrentNode->GetProperty("levelwindow");
    property->RemoveObserver(m_LevelWindowPropertyObserverTag);

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
  std::string lookupTableName("");

  m_CurrentNode->GetFloatProperty("image data min", minDataLimit);
  m_CurrentNode->GetFloatProperty("image data max", maxDataLimit);
  bool lookupTableNameFound = m_CurrentNode->GetStringProperty("LookupTableName", lookupTableName);

  m_Controls->m_MinLimitDoubleSpinBox->setValue(minDataLimit);
  m_Controls->m_MaxLimitDoubleSpinBox->setValue(maxDataLimit);

  signed int lookupTableIndex = -1;
  if (lookupTableNameFound)
  {
    lookupTableIndex = m_Controls->m_LookupTableComboBox->findText(QString::fromStdString(lookupTableName));
  }
  
  if (lookupTableIndex > -1)
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
void ImageLookupTablesView::UpdateLookupTableComboBox()
{
  bool en = m_Controls->m_LookupTableComboBox->blockSignals(true);
  int currentIndex = m_Controls->m_LookupTableComboBox->currentIndex();

  // create a lookup table
  LookupTableProviderService* lutService = ImageLookupTablesViewActivator::GetLookupTableProviderService();
  if (lutService == NULL)
  {
    mitkThrow() << "Failed to find LookupTableProviderService." << std::endl;
  }

  m_Controls->m_LookupTableComboBox->clear();
  m_Controls->m_LookupTableComboBox->addItem(" --- Scaled Lookup Tables --- ");

  std::vector<QString> names = lutService->GetTableNames();
  
  //// Populate combo box with lookup table names.
  for (unsigned int i = 0; i < names.size(); i++)
  {
    if ( lutService->GetIsScaled(names.at(i)) )
    {
      m_Controls->m_LookupTableComboBox->addItem(names.at(i));
    }
  }

  m_Controls->m_LookupTableComboBox->addItem(" ");
  m_Controls->m_LookupTableComboBox->addItem(" --- Labeled Lookup Tables --- ");

  for (unsigned int i = 0; i < names.size(); i++)
  {
    if ( !lutService->GetIsScaled(names.at(i)) )
    {
      m_Controls->m_LookupTableComboBox->addItem(names.at(i));
    }
  }

  m_Controls->m_LookupTableComboBox->setCurrentIndex(currentIndex);
  m_Controls->m_LookupTableComboBox->blockSignals(en);
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
  if (m_CurrentNode.IsNotNull())
  {
    int comboIndex;
    m_CurrentNode->GetIntProperty("LookupTableIndex", comboIndex);
    this->OnLookupTableComboBoxChanged(comboIndex);
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnLookupTableComboBoxChanged(int comboBoxIndex)
{
  if (m_CurrentNode.IsNotNull())
  {
  LookupTableProviderService* lutService = ImageLookupTablesViewActivator::GetLookupTableProviderService();
    if (lutService == NULL)
    {
      mitkThrow() << "Failed to find LookupTableProviderService." << std::endl;
    }

    QString lutName = m_Controls->m_LookupTableComboBox->itemText(comboBoxIndex);

    if( !lutService->CheckName(lutName) )
    {
      return;
    }

    m_CurrentNode->SetStringProperty("LookupTableName", lutName.toStdString().c_str());
    bool isScaled = lutService->GetIsScaled(lutName);

    if (isScaled)
    {
      float lowestOpacity = 1;
      m_CurrentNode->GetFloatProperty("Image Rendering.Lowest Value Opacity", lowestOpacity);

      float highestOpacity = 1;
      m_CurrentNode->GetFloatProperty("Image Rendering.Highest Value Opacity", highestOpacity);

      // Get LUT from Micro Service.
      NamedLookupTableProperty::Pointer mitkLUTProperty = lutService->CreateLookupTableProperty(lutName, lowestOpacity, highestOpacity);
      m_CurrentNode->ReplaceProperty("LookupTable", mitkLUTProperty);

      mitk::RenderingModeProperty::Pointer renderProp = mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR);
      m_CurrentNode->ReplaceProperty("Image Rendering.Mode", renderProp);

      mitk::VtkResliceInterpolationProperty::Pointer resliceProp = mitk::VtkResliceInterpolationProperty::New(VTK_CUBIC_INTERPOLATION);
      m_CurrentNode->ReplaceProperty("reslice interpolation", resliceProp);

      m_CurrentNode->ReplaceProperty("texture interpolation", mitk::BoolProperty::New( true ));
    }
    else
    {
      // Get LUT from Micro Service.
      LabeledLookupTableProperty::Pointer mitkLUTProperty = lutService->CreateLookupTableProperty(lutName);
      m_CurrentNode->ReplaceProperty("LookupTable", mitkLUTProperty);

      mitk::RenderingModeProperty::Pointer renderProp = mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_COLOR);
      m_CurrentNode->ReplaceProperty("Image Rendering.Mode", renderProp);
      
      mitk::VtkResliceInterpolationProperty::Pointer resliceProp = mitk::VtkResliceInterpolationProperty::New(VTK_RESLICE_NEAREST);
      m_CurrentNode->ReplaceProperty("reslice interpolation", resliceProp);

      m_CurrentNode->ReplaceProperty("texture interpolation", mitk::BoolProperty::New( false ));
    }

    this->EnableScaleControls(isScaled);
    this->EnableLabelControls(!isScaled);

    // Force redraw.
    m_CurrentNode->Update();
    m_CurrentNode->Modified();
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


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnLoadButtonPressed()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

   // load a label
  QString filenameWithPath = QFileDialog::getOpenFileName(0, tr("Open File"), "", tr("Text files (*.txt);;XML files (*.xml);;LUT files (*.lut)"));

  if (filenameWithPath.isEmpty())
  {
    return;
  }

  QString lutName = this->LoadLookupTable(filenameWithPath);

  if (lutName.isEmpty()) 
  {
    return;
  }

  this->UpdateLookupTableComboBox();

  // try to set the loaded reader as the selected container
  int index = m_Controls->m_LookupTableComboBox->findText(lutName);

  if (index > -1)
  {
    m_Controls->m_LookupTableComboBox->setCurrentIndex(index); 
  }
  
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(VIEW_ID))
        .Cast<berry::IBerryPreferences>();

  prefs->PutBool("InBlockUpdate", true);

  // save the file to the list of names if not present
  QString cachedFileNames = prefs->Get("LABEL_MAP_NAMES", "");
  QString labelName = QFileInfo(filenameWithPath).baseName();
 
  if (!cachedFileNames.contains(labelName))
  {
    cachedFileNames.append(",");
    cachedFileNames.append(labelName.toStdString().c_str());

    prefs->Put("LABEL_MAP_NAMES", cachedFileNames);
  }

  // update the cached location of the file
  prefs->Put(labelName, filenameWithPath);

  prefs->PutBool("InBlockUpdate", false);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnSaveButtonPressed()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());

  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  QString lutName = m_Controls->m_LookupTableComboBox->currentText();

  if (lutName.isNull() || lutName.isEmpty())
  {
    lutName = QString("lookupTable.txt");
  }
  else if (!lutName.contains(".txt"))
  {
    lutName.append(".txt");
  }

  QFileInfo finfo(lutName);
  QString fileNameAndPath = QFileDialog::getSaveFileName(0, tr("Save File"), finfo.fileName(), tr("Text files (*.txt)"));

  if (fileNameAndPath.isEmpty())
  {
    return;
  }

  LookupTableContainer* newLUT 
    = new LookupTableContainer(labelProperty->GetLookupTable()->GetVtkLookupTable(), labelProperty->GetLabels());
  newLUT->SetDisplayName(labelProperty->GetName());

  MITK_INFO << "fileName " << fileNameAndPath.toStdString().c_str();

  
  mitk::IOUtil::Save(newLUT, fileNameAndPath.toStdString());


  int index = fileNameAndPath.lastIndexOf("/")+1;
  QString labelName = fileNameAndPath.mid(index);
  index = labelName.lastIndexOf(".");
  labelName.truncate(index);

  int comboBoxIndex = -1;
  newLUT->SetOrder(comboBoxIndex);
  m_CurrentNode->GetIntProperty("LookupTableIndex", comboBoxIndex);

  LookupTableProviderService* lutService 
    = ImageLookupTablesViewActivator::GetLookupTableProviderService();

  if (lutService == NULL)
  {
    mitkThrow() << "Failed to find LookupTableProviderService." << std::endl;
  }

  lutService->ReplaceLookupTableContainer(newLUT, newLUT->GetDisplayName());

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IBerryPreferences::Pointer prefs
      = (prefService->GetSystemPreferences()->Node(VIEW_ID)).Cast<berry::IBerryPreferences>();

  prefs->PutBool("InBlockUpdate", true);

  QString cachedFileNames;
  prefs->Get("LABEL_MAP_NAMES", cachedFileNames);

  // save the file to the list of names if not present
  if (!cachedFileNames.contains(labelName))
  {
    cachedFileNames.append(",");
    cachedFileNames.append(labelName.toStdString().c_str());

    prefs->Put("LABEL_MAP_NAMES", cachedFileNames);
  }

  // update the cached location of the file
  prefs->Put(labelName, fileNameAndPath);
  prefs->PutBool("InBlockUpdate", false);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnNewButtonPressed()
{
  // create an empty LookupTable
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  QString newLabelName = QInputDialog::getText(0, tr("Create New Label"),
                                         tr("New label name:"), QLineEdit::Normal );

  if (newLabelName.isEmpty())
  {
    return;
  }

  LookupTableProviderService* lutService = ImageLookupTablesViewActivator::GetLookupTableProviderService();
  if (lutService == NULL)
  {
    mitkThrow() << "Failed to find LookupTableProviderService." << std::endl;
  }

  float lowestOpacity = 1;
  m_CurrentNode->GetFloatProperty("Image Rendering.Lowest Value Opacity", lowestOpacity);

  float highestOpacity = 1;
  m_CurrentNode->GetFloatProperty("Image Rendering.Highest Value Opacity", highestOpacity);

  QColor lowColor(0, 0, 0, lowestOpacity);
  QColor highColor(0, 0, 0, highestOpacity);

  LookupTableContainer * newContainer = new LookupTableContainer(niftk::CreateEmptyLookupTable(lowColor, highColor));
  newContainer->SetDisplayName(newLabelName);
  newContainer->SetIsScaled(false);
  newContainer->SetOrder(lutService->GetNumberOfLookupTables());

  lutService->AddNewLookupTableContainer(newContainer);

  this->UpdateLookupTableComboBox();

  // try to set the loaded reader as the selected container
  int index = m_Controls->m_LookupTableComboBox->findText(newLabelName);
  if (index > -1)
  {
    m_Controls->m_LookupTableComboBox->setCurrentIndex(index); 
  }
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::UpdateLabelMapTable()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  bool en = m_Controls->widget_LabelTable->blockSignals(true);

  // initialize labels widget to empty
  m_Controls->widget_LabelTable->clearContents();
  m_Controls->widget_LabelTable->setRowCount(0);

  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    m_Controls->widget_LabelTable->blockSignals(en);
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());

  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    m_Controls->widget_LabelTable->blockSignals(en);
    return;
  }

  // get labels and LUT
  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  vtkSmartPointer<vtkLookupTable> vtkLUT = labelProperty->GetLookupTable()->GetVtkLookupTable();

  m_Controls->widget_LabelTable->setRowCount(labels.size());

  QSignalMapper* colorMapper = new QSignalMapper(this);

  for (unsigned int i = 0; i < labels.size(); i++)
  {
    // set value
    int value = labels.at(i).first;
    int vtkInd = value - vtkLUT->GetRange()[0] + 1;

    QTableWidgetItem * newValueItem = new QTableWidgetItem();
    newValueItem->setText(QString::number(value));
    m_Controls->widget_LabelTable->setItem(i, 1, newValueItem);

    // set name
    QTableWidgetItem * newNameItem = new QTableWidgetItem();
    newNameItem->setText(labels.at(i).second);
    m_Controls->widget_LabelTable->setItem(i, 2, newNameItem);

    // set color 
    QPushButton* btnColor = new QPushButton;
    btnColor->setFixedWidth(35);
    btnColor->setAutoFillBackground(true);
    
    double rgb[3];
    vtkLUT->GetColor(value, rgb);

    QColor currColor(255 * rgb[0], 255 * rgb[1], 255 * rgb[2]);

    btnColor->setStyleSheet(QString("background-color:rgb(%1,%2, %3)")
                              .arg(currColor.red())
                              .arg(currColor.green())
                              .arg(currColor.blue())
                              );
    m_Controls->widget_LabelTable->setCellWidget(i, 0, btnColor);

    connect(btnColor, SIGNAL(clicked()), colorMapper, SLOT(map()));
    colorMapper->setMapping(btnColor, i);
  }

  connect(colorMapper, SIGNAL(mapped(int)), this, SLOT(OnColorButtonPressed(int)));
  m_Controls->widget_LabelTable->blockSignals(en);

  // Force redraw.
  m_CurrentNode->Update();
  m_CurrentNode->Modified();
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnAddLabelButtonPressed()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());
  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  vtkSmartPointer<vtkLookupTable> oldLUT = labelProperty->GetLookupTable()->GetVtkLookupTable();

  bool en = m_Controls->widget_LabelTable->blockSignals(true);
    
  // get the range

  double* range = oldLUT->GetRange();
  QString newName(" ");

  int newValue = range[1];

  LookupTableContainer::LabelType newLabel = std::make_pair(newValue,newName);
  labels.push_back(newLabel);
  labelProperty->SetLabels(labels);

  // increment the range by 1
  vtkSmartPointer<vtkLookupTable> newLUT;
  newLUT.TakeReference(niftk::ResizeLookupTable(oldLUT,newValue+1));
  labelProperty->GetLookupTable()->SetVtkLookupTable(newLUT);

  UpdateLabelMapTable();
  
  m_Controls->widget_LabelTable->blockSignals(en);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnRemoveLabelButtonPressed()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());
  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  bool en = m_Controls->widget_LabelTable->blockSignals(true);

  QList<QTableWidgetSelectionRange> selectedItems = m_Controls->widget_LabelTable->selectedRanges();
  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  vtkSmartPointer<vtkLookupTable> lut = labelProperty->GetLookupTable()->GetVtkLookupTable();

  QColor nanColor(lut->GetNanColor()[0], lut->GetNanColor()[1], lut->GetNanColor()[2], lut->GetNanColor()[3]);
  for (unsigned int i = 0; i < selectedItems.size(); i++)
  {
    int bottom = selectedItems.at(i).bottomRow()+1;
    int top = selectedItems.at(i).topRow();

    for (unsigned int j = top; j < bottom; j++)
    {
      int value = labels.at(j).first;
      vtkSmartPointer<vtkLookupTable> newLUT;
      newLUT.TakeReference(niftk::ChangeColor(lut, value, nanColor));
      labelProperty->GetLookupTable()->SetVtkLookupTable(newLUT);
    }
    
    labels.erase(labels.begin() + top, labels.begin() + bottom);
  }

  labelProperty->SetLabels(labels);
  UpdateLabelMapTable();
  
  m_Controls->widget_LabelTable->blockSignals(en);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnMoveLabelUpButtonPressed()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());
  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  bool en = m_Controls->widget_LabelTable->blockSignals(true);

  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  QList<QTableWidgetSelectionRange> selectedItems = m_Controls->widget_LabelTable->selectedRanges();

  for (unsigned int i = 0; i < selectedItems.size(); i++)
  {
    int bottom = selectedItems.at(i).bottomRow()+1;
    int top = selectedItems.at(i).topRow();

    if (top == 0)
    {
      continue;
    }

    for (unsigned int j = top; j < bottom; j++)
    {
      std::iter_swap(labels.begin() + j - 1,labels.begin() + j);
    }
  }

  labelProperty->SetLabels(labels);
  UpdateLabelMapTable();

  m_Controls->widget_LabelTable->blockSignals(en);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnMoveLabelDownButtonPressed()
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }

  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());
  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  bool en = m_Controls->widget_LabelTable->blockSignals(true);

  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  QList<QTableWidgetSelectionRange> selectedItems = m_Controls->widget_LabelTable->selectedRanges();

  for (unsigned int i = 0; i < selectedItems.size(); i++)
  {
    int bottom = selectedItems.at(i).bottomRow() + 1;
    int top = selectedItems.at(i).topRow();

    if (bottom == labels.size())
    {
      continue;
    }

    for (unsigned int j = bottom; j > top; j--)
    {
      std::iter_swap(labels.begin()+ j - 1, labels.begin() + j);
    }
  }

  labelProperty->SetLabels(labels);
  UpdateLabelMapTable();

  m_Controls->widget_LabelTable->blockSignals(en);
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnColorButtonPressed(int index)
{
  if (m_CurrentNode.IsNull())
  {
    return;
  }
  
  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());

  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  bool en = m_Controls->widget_LabelTable->blockSignals(true);
  
  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  vtkSmartPointer<vtkLookupTable> lut = labelProperty->GetLookupTable()->GetVtkLookupTable();

  int value = labels.at(index).first;
     
  double rgb[3];
  lut->GetColor(value, rgb);

  QColor initialColor(255 * rgb[0], 255 * rgb[1], 255 * rgb[2]);
  QColor newColor = QColorDialog::getColor(initialColor);

  if (newColor.spec() == 0)
  {  
    m_Controls->widget_LabelTable->blockSignals(en);
    return; 
  }
  
  vtkSmartPointer<vtkLookupTable> newLUT;
  newLUT.TakeReference(niftk::ChangeColor(lut, value,newColor));
  labelProperty->GetLookupTable()->SetVtkLookupTable(newLUT);

  QPushButton* btnColor = qobject_cast<QPushButton*>(m_Controls->widget_LabelTable->cellWidget(index, 0));
  if (btnColor != 0)
  {
    btnColor->setStyleSheet(QString("background-color:rgb(%1,%2, %3)")
                              .arg(newColor.red())
                              .arg(newColor.green())
                              .arg(newColor.blue())
                              );
  }
  
  m_Controls->widget_LabelTable->blockSignals(en);

  // Force redraw.
  m_CurrentNode->Update();
  m_CurrentNode->Modified();
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void ImageLookupTablesView::OnLabelMapTableCellChanged(int row, int column)
{
  if (column == 0)
  {
    return;
  }
  
  if (m_CurrentNode.IsNull())
  {
    return;
  }
  
  // get the labeledlookuptable property
  mitk::BaseProperty::Pointer mitkLUT = m_CurrentNode->GetProperty("LookupTable");
  if (mitkLUT.IsNull())
  {
    MITK_ERROR << "No lookup table assigned to " << m_CurrentNode->GetName();
    return;
  }

  LabeledLookupTableProperty::Pointer labelProperty 
    = dynamic_cast<LabeledLookupTableProperty*>(mitkLUT.GetPointer());

  if (labelProperty.IsNull())
  {
    MITK_ERROR << "LookupTable is not a LabeledLookupTableProperty";
    return;
  }

  
  LabeledLookupTableProperty::LabelListType labels = labelProperty->GetLabels();
  vtkSmartPointer<vtkLookupTable> lut = labelProperty->GetLookupTable()->GetVtkLookupTable();

  if (column == 1)
  {
    QTableWidgetItem* item = m_Controls->widget_LabelTable->item(row,column);
    std::string valStr = item->text().toStdString();
    
    std::string::iterator valItr;
    for (valItr = valStr.begin(); valItr != valStr.end(); valItr++)
    {
      if (!isdigit(*valItr))
      {
        QMessageBox::warning(NULL, "Label Map Editor", QString("Value must be a number. Resetting to old value."));

        QString value = QString::number(labels.at(row).first);
        item->setText(value);
        return;
      }
    }

    int newValue = atoi(valStr.c_str());

    for (unsigned int i = 0; i < labels.size(); i++)
    {
      if (i == row)
      {
        continue;
      }

      if (labels.at(i).first == newValue)
      {
        QMessageBox::warning(NULL, "Label Map Editor", QString("Value is not unique. Resetting to old value."));

        QString oldValueStr = QString::number(labels.at(row).first);
        item->setText(oldValueStr);
        return;
      }
    }

    int oldValue = labels.at(row).first;
    
    vtkSmartPointer<vtkLookupTable> newLUT;
    newLUT.TakeReference(niftk::SwapColors(lut, oldValue, newValue));
    labelProperty->GetLookupTable()->SetVtkLookupTable(newLUT);

    labels.at(row).first = newValue;
  }
  else if (column == 2)
  {
    QTableWidgetItem* item = m_Controls->widget_LabelTable->item(row,column);
    labels.at(row).second = item->text();
  } // change the name

  labelProperty->SetLabels(labels);
  UpdateLabelMapTable();
}


//-----------------------------------------------------------------------------
QString ImageLookupTablesView::LoadLookupTable(QString& fileName)
{
  QString lutName;

  QFileInfo finfo(fileName);
  if (!finfo.exists())
  {
    return lutName;
  }
  
  // create a lookup table
  LookupTableProviderService* lutService = ImageLookupTablesViewActivator::GetLookupTableProviderService(); 
  LookupTableContainer * loadedContainer;

  if (fileName.contains(".lut"))
  {
    QFile file(fileName);
    QXmlInputSource inputSource(&file);

    QXmlSimpleReader reader;
    LookupTableSaxHandler handler;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    if (reader.parse(inputSource))
    {
      loadedContainer = handler.GetLookupTableContainer();
    }
    else
    {
      MITK_ERROR << "niftk::LookupTableManager(): failed to parse XML file (" << fileName.toLocal8Bit().constData()
        << ") so returning null";
    }
  }
  else
  {
    std::vector<mitk::BaseData::Pointer> containerData = mitk::IOUtil::Load(fileName.toStdString());
    if (containerData.empty())
    {
      MITK_ERROR << "Unable to load LookupTableContainer from " << fileName;
    }
    else
    {
      loadedContainer =
        dynamic_cast<LookupTableContainer* >(containerData.at(0).GetPointer());

      if (loadedContainer != NULL)
      {
        loadedContainer->SetDisplayName(loadedContainer->GetDisplayName());
        loadedContainer->SetOrder(lutService->GetNumberOfLookupTables());
      }
    }
  }

  if (loadedContainer != NULL)
  {
    lutService->AddNewLookupTableContainer(loadedContainer);
    lutName = loadedContainer->GetDisplayName();
  }

  return lutName;
}

}
