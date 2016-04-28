/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "MIDASGeneralSegmentorView.h"

#include <QButtonGroup>
#include <QMessageBox>
#include <QGridLayout>

#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkColorProperty.h>
#include <mitkDataNodeObject.h>
#include <mitkProperties.h>
#include <mitkIRenderingManager.h>
#include <mitkRenderingManager.h>
#include <mitkSegTool2D.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkPointSet.h>
#include <mitkPointUtils.h>
#include <mitkGlobalInteraction.h>
#include <mitkTool.h>
#include <mitkPointSet.h>
#include <mitkImageAccessByItk.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkITKImageImport.h>
#include <mitkGeometry2D.h>
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>
#include <mitkDataStorageUtils.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkContourModelSet.h>
#include <mitkFocusManager.h>
#include <mitkSegmentationObjectFactory.h>
#include <mitkSurface.h>
#include <itkCommand.h>
#include <itkContinuousIndex.h>
#include <itkImageFileWriter.h>

#include <QmitkRenderWindow.h>

#include "MIDASGeneralSegmentorViewCommands.h"
#include "MIDASGeneralSegmentorViewHelper.h"
#include <mitkMIDASTool.h>
#include <mitkMIDASPosnTool.h>
#include <mitkMIDASSeedTool.h>
#include <mitkMIDASPolyTool.h>
#include <mitkMIDASDrawTool.h>
#include <mitkMIDASOrientationUtils.h>
#include <mitkMIDASImageUtils.h>

/*
#include <sys/time.h>
double timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  return (((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
           ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec))/1000000000.0;
}
*/

const std::string MIDASGeneralSegmentorView::VIEW_ID = "uk.ac.ucl.cmic.midasgeneralsegmentor";

const mitk::OperationType MIDASGeneralSegmentorView::OP_CHANGE_SLICE = 9320411;
const mitk::OperationType MIDASGeneralSegmentorView::OP_PROPAGATE_SEEDS = 9320412;
const mitk::OperationType MIDASGeneralSegmentorView::OP_RETAIN_MARKS = 9320413;
const mitk::OperationType MIDASGeneralSegmentorView::OP_THRESHOLD_APPLY = 9320414;
const mitk::OperationType MIDASGeneralSegmentorView::OP_CLEAN = 9320415;
const mitk::OperationType MIDASGeneralSegmentorView::OP_WIPE = 9320416;
const mitk::OperationType MIDASGeneralSegmentorView::OP_PROPAGATE = 9320417;

/**************************************************************
 * Start of Constructing/Destructing the View stuff.
 *************************************************************/

//-----------------------------------------------------------------------------
MIDASGeneralSegmentorView::MIDASGeneralSegmentorView()
: QmitkMIDASBaseSegmentationFunctionality()
, m_ToolKeyPressStateMachine(NULL)
, m_GeneralControls(NULL)
, m_Layout(NULL)
, m_ContainerForControlsWidget(NULL)
, m_SliceNavigationController(NULL)
, m_SliceNavigationControllerObserverTag(0)
, m_FocusManagerObserverTag(0)
, m_IsUpdating(false)
, m_IsDeleting(false)
, m_IsChangingSlice(false)
, m_PreviousSliceNumber(0)
, m_IsRestarting(false)
{
  m_Interface = MIDASGeneralSegmentorViewEventInterface::New();
  m_Interface->SetMIDASGeneralSegmentorView(this);
}


//-----------------------------------------------------------------------------
MIDASGeneralSegmentorView::MIDASGeneralSegmentorView(
    const MIDASGeneralSegmentorView& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}


//-----------------------------------------------------------------------------
MIDASGeneralSegmentorView::~MIDASGeneralSegmentorView()
{
  if (m_GeneralControls != NULL)
  {
    delete m_GeneralControls;
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::CreateQtPartControl(QWidget *parent)
{
  this->SetParent(parent);

  if (!m_GeneralControls)
  {
    m_Layout = new QGridLayout(parent);
    m_Layout->setContentsMargins(6, 6, 6, 0);
    m_Layout->setSpacing(3);

    QmitkMIDASBaseSegmentationFunctionality::CreateQtPartControl(parent);

    m_ContainerForControlsWidget = new QWidget(parent);
    m_ContainerForControlsWidget->setContentsMargins(0, 0, 0, 0);

    m_GeneralControls = new MIDASGeneralSegmentorViewControlsWidget(m_ContainerForControlsWidget);
    m_GeneralControls->setContentsMargins(0, 0, 0, 0);

    m_Layout->addWidget(m_ContainerForSelectorWidget, 0, 0);
    m_Layout->addWidget(m_ContainerForToolWidget, 1, 0);
    m_Layout->addWidget(m_ContainerForControlsWidget, 2, 0);

    m_Layout->setRowStretch(0, 0);
    m_Layout->setRowStretch(1, 1);
    m_Layout->setRowStretch(2, 0);

    m_GeneralControls->SetThresholdingCheckboxEnabled(false);
    m_GeneralControls->SetThresholdingWidgetsEnabled(false);

    m_ToolSelector->m_ManualToolSelectionBox->SetDisplayedToolGroups("Seed Draw Poly");
    m_ToolSelector->m_ManualToolSelectionBox->SetLayoutColumns(3);
    m_ToolSelector->m_ManualToolSelectionBox->SetShowNames(true);
    m_ToolSelector->m_ManualToolSelectionBox->SetGenerateAccelerators(false);

//    m_ToolKeyPressStateMachine = mitk::MIDASToolKeyPressStateMachine::New("MIDASToolKeyPressStateMachine", this);
    m_ToolKeyPressStateMachine = mitk::MIDASToolKeyPressStateMachine::New(this);

    this->CreateConnections();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::CreateConnections()
{
  QmitkMIDASBaseSegmentationFunctionality::CreateConnections();

  if ( m_GeneralControls )
  {
    this->connect(m_ToolSelector, SIGNAL(ToolSelected(int)), SLOT(OnToolSelected(int)));
    this->connect(m_GeneralControls->m_CleanButton, SIGNAL(clicked()), SLOT(OnCleanButtonClicked()));
    this->connect(m_GeneralControls->m_WipeButton, SIGNAL(clicked()), SLOT(OnWipeButtonClicked()));
    this->connect(m_GeneralControls->m_WipePlusButton, SIGNAL(clicked()), SLOT(OnWipePlusButtonClicked()));
    this->connect(m_GeneralControls->m_WipeMinusButton, SIGNAL(clicked()), SLOT(OnWipeMinusButtonClicked()));
    this->connect(m_GeneralControls->m_PropUpButton, SIGNAL(clicked()), SLOT(OnPropagateUpButtonClicked()));
    this->connect(m_GeneralControls->m_PropDownButton, SIGNAL(clicked()), SLOT(OnPropagateDownButtonClicked()));
    this->connect(m_GeneralControls->m_Prop3DButton, SIGNAL(clicked()), SLOT(OnPropagate3DButtonClicked()));
    this->connect(m_GeneralControls->m_OKButton, SIGNAL(clicked()), SLOT(OnOKButtonClicked()));
    this->connect(m_GeneralControls->m_CancelButton, SIGNAL(clicked()), SLOT(OnCancelButtonClicked()));
    this->connect(m_GeneralControls->m_RestartButton, SIGNAL(clicked()), SLOT(OnRestartButtonClicked()));
    this->connect(m_GeneralControls->m_ResetButton, SIGNAL(clicked()), SLOT(OnResetButtonClicked()));
    this->connect(m_GeneralControls->m_ThresholdApplyButton, SIGNAL(clicked()), SLOT(OnThresholdApplyButtonClicked()));
    this->connect(m_GeneralControls->m_ThresholdingCheckBox, SIGNAL(toggled(bool)), SLOT(OnThresholdingCheckBoxToggled(bool)));
    this->connect(m_GeneralControls->m_SeePriorCheckBox, SIGNAL(toggled(bool)), SLOT(OnSeePriorCheckBoxToggled(bool)));
    this->connect(m_GeneralControls->m_SeeNextCheckBox, SIGNAL(toggled(bool)), SLOT(OnSeeNextCheckBoxToggled(bool)));
    this->connect(m_GeneralControls->m_ThresholdsSlider, SIGNAL(minimumValueChanged(double)), SLOT(OnThresholdValueChanged()));
    this->connect(m_GeneralControls->m_ThresholdsSlider, SIGNAL(maximumValueChanged(double)), SLOT(OnThresholdValueChanged()));
    this->connect(m_ImageAndSegmentationSelector->m_NewSegmentationButton, SIGNAL(clicked()), SLOT(OnCreateNewSegmentationButtonClicked()) );

    /// Transfer the focus back to the main window if any button is pressed.
    /// This is needed so that the key interactions (like 'a'/'z' for changing slice) keep working.
    this->connect(m_GeneralControls->m_CleanButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_WipeButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_WipePlusButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_WipeMinusButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_PropUpButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_PropDownButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_Prop3DButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_OKButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_CancelButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_RestartButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_ResetButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_ThresholdApplyButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_ThresholdingCheckBox, SIGNAL(toggled(bool)), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_SeePriorCheckBox, SIGNAL(toggled(bool)), SLOT(OnAnyButtonClicked()));
    this->connect(m_GeneralControls->m_SeeNextCheckBox, SIGNAL(toggled(bool)), SLOT(OnAnyButtonClicked()));
    this->connect(m_ImageAndSegmentationSelector->m_NewSegmentationButton, SIGNAL(clicked()), SLOT(OnAnyButtonClicked()));
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::Visible()
{
  QmitkMIDASBaseSegmentationFunctionality::Visible();

  /// TODO
//  mitk::GlobalInteraction::GetInstance()->AddListener( m_ToolKeyPressStateMachine );

  // Connect registered tools back to here, so we can do seed processing logic here.
  mitk::ToolManager::Pointer toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::MIDASPolyTool* midasPolyTool = dynamic_cast<mitk::MIDASPolyTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>()));
  midasPolyTool->ContoursHaveChanged += mitk::MessageDelegate<MIDASGeneralSegmentorView>( this, &MIDASGeneralSegmentorView::OnContoursChanged );

  mitk::MIDASDrawTool* midasDrawTool = dynamic_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
  midasDrawTool->ContoursHaveChanged += mitk::MessageDelegate<MIDASGeneralSegmentorView>( this, &MIDASGeneralSegmentorView::OnContoursChanged );

}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::Hidden()
{
  QmitkMIDASBaseSegmentationFunctionality::Hidden();

  if (m_SliceNavigationController.IsNotNull())
  {
    m_SliceNavigationController->RemoveObserver(m_SliceNavigationControllerObserverTag);
  }

  /// TODO
//  mitk::GlobalInteraction::GetInstance()->RemoveListener(m_ToolKeyPressStateMachine);

  mitk::ToolManager::Pointer toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::MIDASPolyTool* midasPolyTool = dynamic_cast<mitk::MIDASPolyTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>()));
  midasPolyTool->ContoursHaveChanged -= mitk::MessageDelegate<MIDASGeneralSegmentorView>( this, &MIDASGeneralSegmentorView::OnContoursChanged );

  mitk::MIDASDrawTool* midasDrawTool = dynamic_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
  midasDrawTool->ContoursHaveChanged -= mitk::MessageDelegate<MIDASGeneralSegmentorView>( this, &MIDASGeneralSegmentorView::OnContoursChanged );

}

/**************************************************************
 * End of Constructing/Destructing the View stuff.
 *************************************************************/

/**************************************************************
 * Start of: Some base class functions we have to implement
 *************************************************************/

//-----------------------------------------------------------------------------
std::string MIDASGeneralSegmentorView::GetViewID() const
{
  return VIEW_ID;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::SetFocus()
{
  // it seems best not to force the focus, and just leave the
  // focus with whatever the user pressed ... i.e. let Qt handle it.
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::CanStartSegmentationForBinaryNode(const mitk::DataNode::Pointer node)
{
  bool canRestart = false;

  if (node.IsNotNull() && mitk::IsNodeABinaryImage(node))
  {
    mitk::DataNode::Pointer parent = mitk::FindFirstParentImage(this->GetDataStorage(), node, false);
    if (parent.IsNotNull())
    {
      if (mitk::IsNodeAGreyScaleImage(parent))
      {
        canRestart = true;
      }
    }
  }

  return canRestart;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::IsNodeASegmentationImage(const mitk::DataNode::Pointer node)
{
  assert(node);
  bool result = false;

  if (mitk::IsNodeABinaryImage(node))
  {

    mitk::DataNode::Pointer parent = mitk::FindFirstParentImage(this->GetDataStorage(), node, false);

    if (parent.IsNotNull())
    {
      mitk::DataNode::Pointer seedsNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::SEEDS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer currentContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer drawContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::DRAW_CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer seePriorContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::PRIOR_CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer seeNextContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::NEXT_CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer regionGrowingImageNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::REGION_GROWING_NAME.c_str(), node, true);

      if (seedsNode.IsNotNull()
          && currentContoursNode.IsNotNull()
          && drawContoursNode.IsNotNull()
          && seePriorContoursNode.IsNotNull()
          && seeNextContoursNode.IsNotNull()
          && regionGrowingImageNode.IsNotNull()
          )
      {
        result = true;
      }
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::ToolManager::DataVectorType MIDASGeneralSegmentorView::GetWorkingDataFromSegmentationNode(const mitk::DataNode::Pointer node)
{
  assert(node);
  mitk::ToolManager::DataVectorType result;

  if (mitk::IsNodeABinaryImage(node))
  {
    mitk::DataNode::Pointer parent = mitk::FindFirstParentImage(this->GetDataStorage(), node, false);

    if (parent.IsNotNull())
    {
      mitk::DataNode::Pointer seedsNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::SEEDS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer currentContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer drawContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::DRAW_CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer seePriorContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::PRIOR_CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer seeNextContoursNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::NEXT_CONTOURS_NAME.c_str(), node, true);
      mitk::DataNode::Pointer regionGrowingImageNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::REGION_GROWING_NAME.c_str(), node, true);
      mitk::DataNode::Pointer initialSegmentationImageNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::INITIAL_SEGMENTATION_NAME.c_str(), node, true);
      mitk::DataNode::Pointer initialSeedsNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::INITIAL_SEEDS_NAME.c_str(), node, true);

      if (seedsNode.IsNotNull()
          && currentContoursNode.IsNotNull()
          && drawContoursNode.IsNotNull()
          && seePriorContoursNode.IsNotNull()
          && seeNextContoursNode.IsNotNull()
          && regionGrowingImageNode.IsNotNull()
          && initialSegmentationImageNode.IsNotNull()
          && initialSeedsNode.IsNotNull()
          )
      {
        // The order of this list must match the order they were created in.
        result.push_back(node);
        result.push_back(seedsNode);
        result.push_back(currentContoursNode);
        result.push_back(drawContoursNode);
        result.push_back(seePriorContoursNode);
        result.push_back(seeNextContoursNode);
        result.push_back(regionGrowingImageNode);
        result.push_back(initialSegmentationImageNode);
        result.push_back(initialSeedsNode);
      }
    }
  }
  return result;
}

/**************************************************************
 * End of: Some base class functions we have to implement
 *************************************************************/

/**************************************************************
 * Start of: Functions to create reference data (hidden nodes)
 *************************************************************/

//-----------------------------------------------------------------------------
mitk::DataNode::Pointer MIDASGeneralSegmentorView::CreateContourSet(mitk::DataNode::Pointer segmentationNode, float r, float g, float b, std::string name, bool visible, int layer)
{
  mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();

  mitk::DataNode::Pointer contourSetNode = mitk::DataNode::New();

  contourSetNode->SetProperty("color", mitk::ColorProperty::New(r, g, b));
  contourSetNode->SetProperty("contour.color", mitk::ColorProperty::New(r, g, b));
  contourSetNode->SetFloatProperty("opacity", 1.0f);
  contourSetNode->SetProperty("name", mitk::StringProperty::New(name));
  contourSetNode->SetBoolProperty("helper object", true);
  contourSetNode->SetBoolProperty("visible", visible);
  contourSetNode->SetProperty("layer", mitk::IntProperty::New(layer));
  contourSetNode->SetData(contourSet);

  return contourSetNode;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer MIDASGeneralSegmentorView::CreateHelperImage(mitk::Image::Pointer referenceImage, mitk::DataNode::Pointer segmentationNode, float r, float g, float b, std::string name, bool visible, int layer)
{
  mitk::ToolManager::Pointer toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::Tool* drawTool = toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>());
  assert(drawTool);

  mitk::ColorProperty::Pointer col = mitk::ColorProperty::New(r, g, b);

  mitk::DataNode::Pointer helperImageNode = drawTool->CreateEmptySegmentationNode( referenceImage, name, col->GetColor());
  helperImageNode->SetColor(col->GetColor());
  helperImageNode->SetProperty("binaryimage.selectedcolor", col);
  helperImageNode->SetBoolProperty("helper object", true);
  helperImageNode->SetBoolProperty("visible", visible);
  helperImageNode->SetProperty("layer", mitk::IntProperty::New(layer));

  this->ApplyDisplayOptions(helperImageNode);

  return helperImageNode;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnCreateNewSegmentationButtonClicked()
{
  // Create the new segmentation, either using a previously selected one, or create a new volume.
  mitk::DataNode::Pointer newSegmentation = NULL;
  bool isRestarting = false;

  // Make sure we have a reference images... which should always be true at this point.
  mitk::Image* image = this->GetReferenceImageFromToolManager();
  if (image != NULL)
  {
    mitk::ToolManager::Pointer toolManager = this->GetToolManager();
    assert(toolManager);

    if (mitk::IsNodeABinaryImage(m_SelectedNode)
        && this->CanStartSegmentationForBinaryNode(m_SelectedNode)
        && !this->IsNodeASegmentationImage(m_SelectedNode)
        )
    {
      newSegmentation =  m_SelectedNode;
      isRestarting = true;
    }
    else
    {
      newSegmentation = this->CreateNewSegmentation(m_DefaultSegmentationColor);

      // The above method returns NULL if the user exited the colour selection dialog box.
      if (newSegmentation.IsNull())
      {
        return;
      }
    }

    this->WaitCursorOn();

    // Override the base colour to be orange, and we revert this when OK pressed at the end.
    mitk::Color tmpColor;
    tmpColor[0] = 1.0;
    tmpColor[1] = 0.65;
    tmpColor[2] = 0.0;
    mitk::ColorProperty::Pointer tmpColorProperty = mitk::ColorProperty::New(tmpColor);
    newSegmentation->SetColor(tmpColor);
    newSegmentation->SetProperty("binaryimage.selectedcolor", tmpColorProperty);

    // Set initial properties.
    newSegmentation->SetProperty("layer", mitk::IntProperty::New(90));
    newSegmentation->SetFloatProperty("opacity", 1.0f);
    newSegmentation->SetBoolProperty(mitk::MIDASContourTool::EDITING_PROPERTY_NAME.c_str(), false);

    // Make sure these are up to date, even though we don't use them right now.
    image->GetStatistics()->GetScalarValueMin();
    image->GetStatistics()->GetScalarValueMax();

    // This creates the point set for the seeds.
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
    pointSetNode->SetData(pointSet);
    pointSetNode->SetProperty("name", mitk::StringProperty::New(mitk::MIDASTool::SEEDS_NAME));
    pointSetNode->SetFloatProperty("opacity", 1.0f);
    pointSetNode->SetProperty("point line width", mitk::IntProperty::New(1));
    pointSetNode->SetProperty("point 2D size", mitk::IntProperty::New(5));
    pointSetNode->SetBoolProperty("helper object", true);
    pointSetNode->SetBoolProperty("show distant lines", false);
    pointSetNode->SetFloatProperty("Pointset.2D.distance to plane", 0.1);
    pointSetNode->SetBoolProperty("show distances", false);
    pointSetNode->SetProperty("layer", mitk::IntProperty::New(99));
    pointSetNode->SetColor(1.0, 0.0, 0.0);

    // Create all the contours.
    mitk::DataNode::Pointer currentContours = this->CreateContourSet(newSegmentation, 0,1,0, mitk::MIDASTool::CONTOURS_NAME, true, 97);
    mitk::DataNode::Pointer drawContours = this->CreateContourSet(newSegmentation, 0,1,0, mitk::MIDASTool::DRAW_CONTOURS_NAME, true, 98);
    mitk::DataNode::Pointer seeNextNode = this->CreateContourSet(newSegmentation, 0,1,1, mitk::MIDASTool::NEXT_CONTOURS_NAME, false, 95);
    mitk::DataNode::Pointer seePriorNode = this->CreateContourSet(newSegmentation, 0.68,0.85,0.90, mitk::MIDASTool::PRIOR_CONTOURS_NAME, false, 96);

    // Create the region growing image.
    mitk::DataNode::Pointer regionGrowingImageNode = this->CreateHelperImage(image, newSegmentation, 0,0,1, mitk::MIDASTool::REGION_GROWING_NAME, false, 94);

    // Create nodes to store the original segmentation and seeds, so that it can be restored if the Restart button is pressed.
    mitk::DataNode::Pointer initialSegmentationNode = mitk::DataNode::New();
    initialSegmentationNode->SetProperty("name", mitk::StringProperty::New(mitk::MIDASTool::INITIAL_SEGMENTATION_NAME));
    initialSegmentationNode->SetBoolProperty("helper object", true);
    initialSegmentationNode->SetBoolProperty("visible", false);
    initialSegmentationNode->SetProperty("layer", mitk::IntProperty::New(99));
    initialSegmentationNode->SetFloatProperty("opacity", 1.0f);
    initialSegmentationNode->SetColor(tmpColor);
    initialSegmentationNode->SetProperty("binaryimage.selectedcolor", tmpColorProperty);

    mitk::DataNode::Pointer initialSeedsNode = mitk::DataNode::New();
    initialSeedsNode->SetProperty("name", mitk::StringProperty::New(mitk::MIDASTool::INITIAL_SEEDS_NAME));
    initialSeedsNode->SetBoolProperty("helper object", true);
    initialSeedsNode->SetBoolProperty("visible", false);
    initialSeedsNode->SetBoolProperty("show distant lines", false);
    initialSeedsNode->SetFloatProperty("Pointset.2D.distance to plane", 0.1);
    initialSeedsNode->SetBoolProperty("show distances", false);
    initialSeedsNode->SetProperty("layer", mitk::IntProperty::New(99));
    initialSeedsNode->SetColor(1.0, 0.0, 0.0);

    /// TODO
    /// We should not refer to mitk::RenderingManager::GetInstance() because the DnD display uses its
    /// own rendering manager, not this one, like the MITK display.
    mitk::IRenderingManager* renderingManager = 0;
    mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
    if (renderWindowPart)
    {
      renderingManager = renderWindowPart->GetRenderingManager();
    }
    if (renderingManager)
    {
      // Make sure these points and contours are not rendered in 3D, as there can be many of them if you "propagate",
      // and furthermore, there seem to be several seg faults rendering contour code in 3D. Haven't investigated yet.
      QList<vtkRenderWindow*> renderWindows = renderingManager->GetAllRegisteredVtkRenderWindows();
      for (QList<vtkRenderWindow*>::const_iterator iter = renderWindows.begin(); iter != renderWindows.end(); ++iter)
      {
        if ( mitk::BaseRenderer::GetInstance((*iter))->GetMapperID() == mitk::BaseRenderer::Standard3D )
        {
          pointSetNode->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
          seePriorNode->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
          seeNextNode->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
          currentContours->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
          drawContours->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
          initialSegmentationNode->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
          initialSeedsNode->SetBoolProperty("visible", false, mitk::BaseRenderer::GetInstance((*iter)));
        }
      }
    }

    // Adding to data storage, where the ordering affects the layering.
    this->GetDataStorage()->Add(seePriorNode, newSegmentation);
    this->GetDataStorage()->Add(seeNextNode, newSegmentation);
    this->GetDataStorage()->Add(regionGrowingImageNode, newSegmentation);
    this->GetDataStorage()->Add(currentContours, newSegmentation);
    this->GetDataStorage()->Add(drawContours, newSegmentation);
    this->GetDataStorage()->Add(pointSetNode, newSegmentation);
    this->GetDataStorage()->Add(initialSegmentationNode, newSegmentation);
    this->GetDataStorage()->Add(initialSeedsNode, newSegmentation);

    // Set working data. See header file, as the order here is critical, and should match the documented order.
    mitk::ToolManager::DataVectorType workingData(9);
    workingData[mitk::MIDASTool::SEGMENTATION] = newSegmentation;
    workingData[mitk::MIDASTool::SEEDS] = pointSetNode;
    workingData[mitk::MIDASTool::CONTOURS] = currentContours;
    workingData[mitk::MIDASTool::DRAW_CONTOURS] = drawContours;
    workingData[mitk::MIDASTool::PRIOR_CONTOURS] = seePriorNode;
    workingData[mitk::MIDASTool::NEXT_CONTOURS] = seeNextNode;
    workingData[mitk::MIDASTool::REGION_GROWING] = regionGrowingImageNode;
    workingData[mitk::MIDASTool::INITIAL_SEGMENTATION] = initialSegmentationNode;
    workingData[mitk::MIDASTool::INITIAL_SEEDS] = initialSeedsNode;
    toolManager->SetWorkingData(workingData);

    if (isRestarting)
    {
      this->InitialiseSeedsForWholeVolume();
      this->UpdateCurrentSliceContours();
    }

    this->StoreInitialSegmentation();

    // Setup GUI.
    m_GeneralControls->SetAllWidgetsEnabled(true);
    m_GeneralControls->SetThresholdingWidgetsEnabled(false);
    m_GeneralControls->SetThresholdingCheckboxEnabled(true);
    m_GeneralControls->m_ThresholdingCheckBox->setChecked(false);

    this->FocusOnCurrentWindow();
    this->OnFocusChanged();
    this->RequestRenderWindowUpdate();

    this->WaitCursorOff();

  } // end if we have a reference image

  m_IsRestarting = isRestarting;

  // Finally, select the new segmentation node.
  this->SetCurrentSelection(newSegmentation);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::StoreInitialSegmentation()
{
  mitk::ToolManager::Pointer toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::ToolManager::DataVectorType workingData = toolManager->GetWorkingData();

  mitk::DataNode* segmentationNode = workingData[mitk::MIDASTool::SEGMENTATION];
  mitk::DataNode* seedsNode = workingData[mitk::MIDASTool::SEEDS];
  mitk::DataNode* initialSegmentationNode = workingData[mitk::MIDASTool::INITIAL_SEGMENTATION];
  mitk::DataNode* initialSeedsNode = workingData[mitk::MIDASTool::INITIAL_SEEDS];

  initialSegmentationNode->SetData(dynamic_cast<mitk::Image*>(segmentationNode->GetData())->Clone());
  initialSeedsNode->SetData(dynamic_cast<mitk::PointSet*>(seedsNode->GetData())->Clone());
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::onVisibilityChanged(const mitk::DataNode* node)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  std::vector<mitk::DataNode*> workingData = this->GetWorkingData();
  if (!workingData.empty() && node == workingData[mitk::MIDASTool::SEGMENTATION])
  {
    bool segmentationNodeVisibility;
    if (node->GetVisibility(segmentationNodeVisibility, 0) && segmentationNodeVisibility)
    {
      workingData[mitk::MIDASTool::SEEDS]->SetVisibility(true);
      workingData[mitk::MIDASTool::CONTOURS]->SetVisibility(true);
      workingData[mitk::MIDASTool::DRAW_CONTOURS]->SetVisibility(true);
      if (m_GeneralControls->m_SeePriorCheckBox->isChecked())
      {
        workingData[mitk::MIDASTool::PRIOR_CONTOURS]->SetVisibility(true);
      }
      if (m_GeneralControls->m_SeeNextCheckBox->isChecked())
      {
        workingData[mitk::MIDASTool::NEXT_CONTOURS]->SetVisibility(true);
      }
      if (m_GeneralControls->m_ThresholdingCheckBox->isChecked())
      {
        workingData[mitk::MIDASTool::REGION_GROWING]->SetVisibility(true);
      }
      workingData[mitk::MIDASTool::INITIAL_SEGMENTATION]->SetVisibility(false);
      workingData[mitk::MIDASTool::INITIAL_SEEDS]->SetVisibility(false);

      mitk::ToolManager::Pointer toolManager = this->GetToolManager();
      mitk::MIDASPolyTool* polyTool = static_cast<mitk::MIDASPolyTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>()));
      assert(polyTool);
      polyTool->SetFeedbackContourVisible(toolManager->GetActiveTool() == polyTool);
    }
    else
    {
      for (std::size_t i = 1; i < workingData.size(); ++i)
      {
        workingData[i]->SetVisibility(false);
      }
    }
  }
}


/**************************************************************
 * End of: Functions to create reference data (hidden nodes)
 *************************************************************/


/**************************************************************
 * Start of: Utility functions
 *************************************************************/

//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::EnableSegmentationWidgets(bool enabled)
{
  m_GeneralControls->SetAllWidgetsEnabled(enabled);
  bool thresholdingIsOn = m_GeneralControls->m_ThresholdingCheckBox->isChecked();
  m_GeneralControls->SetThresholdingWidgetsEnabled(thresholdingIsOn);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::RecalculateMinAndMaxOfImage()
{
  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    double min = referenceImage->GetStatistics()->GetScalarValueMinNoRecompute();
    double max = referenceImage->GetStatistics()->GetScalarValueMaxNoRecompute();
    m_GeneralControls->SetLowerAndUpperIntensityRanges(min, max);
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::RecalculateMinAndMaxOfSeedValues()
{
  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  mitk::PointSet::Pointer seeds = this->GetSeeds();

  if (referenceImage.IsNotNull() && seeds.IsNotNull())
  {
    double min = 0;
    double max = 0;

    int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
    int axisNumber = this->GetViewAxis();

    if (sliceNumber != -1 && axisNumber != -1)
    {
      try
      {
        AccessFixedDimensionByItk_n(referenceImage, ITKRecalculateMinAndMaxOfSeedValues, 3, (*(seeds.GetPointer()), axisNumber, sliceNumber, min, max));
        m_GeneralControls->SetSeedMinAndMaxValues(min, max);
      }
      catch(const mitk::AccessByItkException& e)
      {
        MITK_ERROR << "Caught exception, so abandoning recalculating min and max of seeds values, due to:" << e.what();
      }
    }
  }
}


//-----------------------------------------------------------------------------
mitk::PointSet* MIDASGeneralSegmentorView::GetSeeds()
{

  mitk::PointSet* result = NULL;

  mitk::ToolManager::Pointer toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::DataNode::Pointer seedsNode = toolManager->GetWorkingData(mitk::MIDASTool::SEEDS);
  if (seedsNode.IsNotNull())
  {
    result = dynamic_cast<mitk::PointSet*>(seedsNode->GetData());
  }

  return result;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::HasInitialisedWorkingData()
{
  bool result = false;

  mitk::ToolManager::DataVectorType nodes = this->GetWorkingData();
  if (nodes.size() > 0)
  {
    result = true;
  }

  return result;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::UpdateSegmentationImageVisibility(bool overrideToGlobal)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  /*
   * Work in progress.
   *
   * I'm removing this because:
   *   If we are using the MIDAS editor, then we have renderer specific visibility flags.
   *   If we are using the MITK editor, then we only bother with global flags.
   *   So, at this stage, test with an orange outline, leaving the current segmentation as a
   *   green outline and don't mess around with the visibility.
   */

/*
 * This bit is wrong, as it does not set the visibility to "true" at any point.
 * So, successive clicks on different windows means eventually, the contours
 * are invisible in all windows.
 *
  mitk::DataNode::Pointer segmentationNode = nodes[0];

  if (this->GetPreviouslyFocusedRenderer() != NULL)
  {
    mitk::PropertyList* list = segmentationNode->GetPropertyList(this->GetPreviouslyFocusedRenderer());
    if (list != NULL)
    {
      list->DeleteProperty("visible");
    }
  }

  if (this->GetFocusedRenderer() != NULL)
  {
    if (overrideToGlobal)
    {
      mitk::PropertyList* list = segmentationNode->GetPropertyList(GetFocusedRenderer());
      if (list != NULL)
      {
        list->DeleteProperty("visible");
      }
    }
    else
    {
      segmentationNode->SetVisibility(false, this->GetFocusedRenderer());
    }
  }
*/
}

//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::GenerateOutlineFromBinaryImage(mitk::Image::Pointer image,
    int axisNumber,
    int sliceNumber,
    int projectedSliceNumber,
    mitk::ContourModelSet::Pointer outputContourSet
    )
{
  try
  {
    AccessFixedTypeByItk_n(image,
        ITKGenerateOutlineFromBinaryImage,
        (unsigned char),
        (3),
        (axisNumber,
         sliceNumber,
         projectedSliceNumber,
         outputContourSet
        )
      );
  }
  catch(const mitk::AccessByItkException& e)
  {
    MITK_ERROR << "Failed in ITKGenerateOutlineFromBinaryImage due to:" << e.what();
    outputContourSet->Clear();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::FilterSeedsToCurrentSlice(
    mitk::PointSet& inputPoints,
    int& axisNumber,
    int& sliceNumber,
    mitk::PointSet& outputPoints
    )
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    try
    {
      AccessFixedDimensionByItk_n(referenceImage,
          ITKFilterSeedsToCurrentSlice, 3,
          (inputPoints,
           axisNumber,
           sliceNumber,
           outputPoints
          )
        );
    }
    catch(const mitk::AccessByItkException& e)
    {
      MITK_ERROR << "Caught exception, so abandoning FilterSeedsToCurrentSlice, caused by:" << e.what();
    }
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::FilterSeedsToEnclosedSeedsOnCurrentSlice(
    mitk::PointSet& inputPoints,
    bool& thresholdOn,
    int& sliceNumber,
    mitk::PointSet& outputPoints
    )
{
  outputPoints.Clear();

  mitk::PointSet::Pointer singleSeedPointSet = mitk::PointSet::New();

  mitk::PointSet::PointsConstIterator inputPointsIt = inputPoints.Begin();
  mitk::PointSet::PointsConstIterator inputPointsEnd = inputPoints.End();
  for ( ; inputPointsIt != inputPointsEnd; ++inputPointsIt)
  {
    mitk::PointSet::PointType point = inputPointsIt->Value();
    mitk::PointSet::PointIdentifier pointID = inputPointsIt->Index();

    singleSeedPointSet->Clear();
    singleSeedPointSet->InsertPoint(0, point);

    bool unenclosed = this->DoesSliceHaveUnenclosedSeeds(thresholdOn, sliceNumber, *(singleSeedPointSet.GetPointer()));

    if (!unenclosed)
    {
      outputPoints.InsertPoint(pointID, point);
    }
  }
}


/**************************************************************
 * End of: Utility functions
 *************************************************************/

/**************************************************************
 * Start of: Functions for OK/Reset/Cancel/Close.
 * i.e. finishing a segmentation, and destroying stuff.
 *************************************************************/


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::DestroyPipeline()
{
  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    m_IsDeleting = true;
    try
    {
      AccessFixedDimensionByItk(referenceImage, ITKDestroyPipeline, 3);
    }
    catch(const mitk::AccessByItkException& e)
    {
      MITK_ERROR << "Caught exception, so abandoning destroying the ITK pipeline, caused by:" << e.what();
    }
    m_IsDeleting = false;
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::RemoveWorkingData()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  m_IsDeleting = true;

  mitk::ToolManager* toolManager = this->GetToolManager();
  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();

  // We don't do the first image, as thats the final segmentation.
  for (unsigned int i = 1; i < workingData.size(); i++)
  {
    this->GetDataStorage()->Remove(workingData[i]);
  }

  mitk::ToolManager::DataVectorType emptyWorkingDataArray;
  toolManager->SetWorkingData(emptyWorkingDataArray);
  toolManager->ActivateTool(-1);

  m_IsDeleting = false;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::RestoreInitialSegmentation()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::DataNode::Pointer segmentationNode = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::SEGMENTATION);
  assert(segmentationNode);

  mitk::DataNode::Pointer seedsNode = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::SEEDS);
  assert(seedsNode);

  try
  {
    /// Originally, this function cleared the segmentation and the pointset, but
    /// now we rather restore the initial state of the segmentation as it was
    /// when we pressed the Create/restart segmentation button.

//    mitk::Image::Pointer segmentationImage = dynamic_cast<mitk::Image*>(segmentationNode->GetData());
//    assert(segmentationImage);
//    AccessFixedDimensionByItk(segmentationImage.GetPointer(), ITKClearImage, 3);
//    segmentationImage->Modified();
//    segmentationNode->Modified();

//    mitk::PointSet::Pointer seeds = this->GetSeeds();
//    seeds->Clear();

    mitk::DataNode::Pointer initialSegmentationNode = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::INITIAL_SEGMENTATION);
    mitk::DataNode::Pointer initialSeedsNode = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::INITIAL_SEEDS);

    segmentationNode->SetData(dynamic_cast<mitk::Image*>(initialSegmentationNode->GetData())->Clone());
    seedsNode->SetData(dynamic_cast<mitk::PointSet*>(initialSeedsNode->GetData())->Clone());

    // This will cause OnSliceNumberChanged to be called, forcing refresh of all contours.
    if (m_SliceNavigationController)
    {
      m_SliceNavigationController->SendSlice();
    }
  }
  catch(const mitk::AccessByItkException& e)
  {
    MITK_ERROR << "Caught exception during ITKClearImage, caused by:" << e.what();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnOKButtonClicked()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  // Set the colour to that which the user selected in the first place.
  mitk::DataNode::Pointer workingData = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::SEGMENTATION);
  workingData->SetProperty("color", workingData->GetProperty("midas.tmp.selectedcolor"));
  workingData->SetProperty("binaryimage.selectedcolor", workingData->GetProperty("midas.tmp.selectedcolor"));

  /// Apply the thresholds if we are thresholding, and chunk out the contour segments that
  /// do not close any region with seed.
  this->OnCleanButtonClicked();

  this->DestroyPipeline();
  this->RemoveWorkingData();
  this->UpdateSegmentationImageVisibility(true);
  this->EnableSegmentationWidgets(false);
  this->SetReferenceImageSelected();

  this->RequestRenderWindowUpdate();
  mitk::UndoController::GetCurrentUndoModel()->Clear();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnResetButtonClicked()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  int returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                            tr("Clear all slices ? \n This is not Undo-able! \n Are you sure?"),
                                                            QMessageBox::Yes | QMessageBox::No);
  if (returnValue == QMessageBox::No)
  {
    return;
  }

  this->ClearWorkingData();
  this->UpdateRegionGrowing();
  this->UpdatePriorAndNext();
  this->UpdateCurrentSliceContours();
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnCancelButtonClicked()
{
  this->DiscardSegmentation();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::ClosePart()
{
  this->DiscardSegmentation();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::DiscardSegmentation()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::DataNode::Pointer segmentationNode = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::SEGMENTATION);
  assert(segmentationNode);

  this->DestroyPipeline();
  if (m_IsRestarting)
  {
    this->RestoreInitialSegmentation();
    this->RemoveWorkingData();
  }
  else
  {
    this->RemoveWorkingData();
    this->GetDataStorage()->Remove(segmentationNode);
  }
  this->EnableSegmentationWidgets(false);
  this->SetReferenceImageSelected();
  this->RequestRenderWindowUpdate();
  mitk::UndoController::GetCurrentUndoModel()->Clear();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnRestartButtonClicked()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  int returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                            tr("Discard all changes?\nThis is not Undo-able!\nAre you sure?"),
                                                            QMessageBox::Yes | QMessageBox::No);
  if (returnValue == QMessageBox::No)
  {
    return;
  }

  this->RestoreInitialSegmentation();
  this->UpdateRegionGrowing();
  this->UpdatePriorAndNext();
  this->UpdateCurrentSliceContours();
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::ClearWorkingData()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::DataNode::Pointer workingData = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::SEGMENTATION);
  assert(workingData);

  mitk::Image::Pointer segmentationImage = dynamic_cast<mitk::Image*>(workingData->GetData());
  assert(segmentationImage);

  try
  {
    AccessFixedDimensionByItk(segmentationImage.GetPointer(), ITKClearImage, 3);
    segmentationImage->Modified();
    workingData->Modified();

    mitk::PointSet::Pointer seeds = this->GetSeeds();
    seeds->Clear();

    // This will cause OnSliceNumberChanged to be called, forcing refresh of all contours.
    if (m_SliceNavigationController)
    {
      m_SliceNavigationController->SendSlice();
    }
  }
  catch(const mitk::AccessByItkException& e)
  {
    MITK_ERROR << "Caught exception during ITKClearImage, caused by:" << e.what();
  }
}


/**************************************************************
 * End of: Functions for OK/Reset/Cancel/Close.
 *************************************************************/

/**************************************************************
 * Start of: Functions for simply tool toggling
 *************************************************************/

//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::ToggleTool(int toolId)
{
  mitk::ToolManager* toolManager = this->GetToolManager();
  int activeToolId = toolManager->GetActiveToolID();

  if (toolId == activeToolId)
  {
    toolManager->ActivateTool(-1);
  }
  else
  {
    toolManager->ActivateTool(toolId);
  }
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::SelectSeedTool()
{
  /// Note:
  /// If the tool selection box is disabled then the tools are not registered to
  /// the tool manager ( RegisterClient() ). Then if you activate a tool and another
  /// tool was already active, then its interaction event observer service tries to
  /// be unregistered. But since the tools was not registered into the tool manager,
  /// the observer service is still null, and the attempt to unregister it causes crash.
  ///
  /// Consequence:
  /// We should not do anything with the tools until they are registered to the
  /// tool manager.

  if (m_ToolSelector->m_ManualToolSelectionBox->isEnabled())
  {
    mitk::ToolManager* toolManager = this->GetToolManager();
    int activeToolId = toolManager->GetActiveToolID();
    int seedToolId = toolManager->GetToolIdByToolType<mitk::MIDASSeedTool>();

    if (seedToolId != activeToolId)
    {
      toolManager->ActivateTool(seedToolId);
    }

    return true;
  }

  return false;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::SelectDrawTool()
{
  /// Note: see comment in SelectSeedTool().
  if (m_ToolSelector->m_ManualToolSelectionBox->isEnabled())
  {
    mitk::ToolManager* toolManager = this->GetToolManager();
    int activeToolId = toolManager->GetActiveToolID();
    int drawToolId = toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>();

    if (drawToolId != activeToolId)
    {
      toolManager->ActivateTool(drawToolId);
    }

    return true;
  }

  return false;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::SelectPolyTool()
{
  /// Note: see comment in SelectSeedTool().
  if (m_ToolSelector->m_ManualToolSelectionBox->isEnabled())
  {
    mitk::ToolManager* toolManager = this->GetToolManager();
    int activeToolId = toolManager->GetActiveToolID();
    int polyToolId = toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>();

    if (polyToolId != activeToolId)
    {
      toolManager->ActivateTool(polyToolId);
    }

    return true;
  }

  return false;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::UnselectTools()
{
  if (m_ToolSelector->m_ManualToolSelectionBox->isEnabled())
  {
    mitk::ToolManager* toolManager = this->GetToolManager();

    if (toolManager->GetActiveToolID() != -1)
    {
      toolManager->ActivateTool(-1);
    }

    return true;
  }

  return false;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::SelectViewMode()
{
  /// Note: see comment in SelectSeedTool().
  if (m_ToolSelector->m_ManualToolSelectionBox->isEnabled())
  {
    if (!this->HasInitialisedWorkingData())
    {
      QList<mitk::DataNode::Pointer> selectedNodes = this->GetDataManagerSelection();
      foreach (mitk::DataNode::Pointer selectedNode, selectedNodes)
      {
        selectedNode->SetVisibility(!selectedNode->IsVisible(0));
      }
      this->RequestRenderWindowUpdate();

      return true;
    }

    mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();
    bool segmentationNodeIsVisible = workingData[mitk::MIDASTool::SEGMENTATION]->IsVisible(0);
    workingData[mitk::MIDASTool::SEGMENTATION]->SetVisibility(!segmentationNodeIsVisible);
    this->RequestRenderWindowUpdate();

    return true;
  }

  return false;
}


/**************************************************************
 * End of: Functions for simply tool toggling
 *************************************************************/

/**************************************************************
 * Start of: The main MIDAS business logic.
 *************************************************************/

//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::InitialiseSeedsForWholeVolume()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  MIDASOrientation orientation = this->GetOrientationAsEnum();
  if (orientation == MIDAS_ORIENTATION_UNKNOWN)
  {
    orientation = MIDAS_ORIENTATION_CORONAL;
  }

  int axis = this->GetAxisFromReferenceImage(orientation);
  if (axis == -1)
  {
    axis = 0;
  }

  mitk::PointSet *seeds = this->GetSeeds();
  assert(seeds);

  mitk::Image::Pointer workingImage = this->GetWorkingImageFromToolManager(0);
  assert(workingImage);

  try
  {
    AccessFixedDimensionByItk_n(workingImage,
        ITKInitialiseSeedsForVolume, 3,
        (*seeds,
         axis
        )
      );
  }
  catch(const mitk::AccessByItkException& e)
  {
    MITK_ERROR << "Caught exception during ITKInitialiseSeedsForVolume, so have not initialised seeds correctly, caused by:" << e.what();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnFocusChanged()
{
  QmitkBaseView::OnFocusChanged();

  mitk::BaseRenderer* focusedRenderer = this->GetFocusedRenderer();

  if (focusedRenderer != NULL)
  {

    if (m_SliceNavigationController.IsNotNull())
    {
      m_SliceNavigationController->RemoveObserver(m_SliceNavigationControllerObserverTag);
    }

    m_SliceNavigationController = this->GetSliceNavigationController();

    if (m_SliceNavigationController.IsNotNull())
    {
      itk::ReceptorMemberCommand<MIDASGeneralSegmentorView>::Pointer onSliceChangedCommand =
        itk::ReceptorMemberCommand<MIDASGeneralSegmentorView>::New();

      onSliceChangedCommand->SetCallbackFunction( this, &MIDASGeneralSegmentorView::OnSliceChanged );


      m_PreviousSliceNumber = -1;
      m_PreviousFocusPoint.Fill(0);
      m_CurrentFocusPoint.Fill(0);

      m_SliceNavigationControllerObserverTag =
          m_SliceNavigationController->AddObserver(
              mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), onSliceChangedCommand);

      m_SliceNavigationController->SendSlice();
    }

    this->UpdateSegmentationImageVisibility(false);
    this->UpdatePriorAndNext();
    this->OnThresholdingCheckBoxToggled(m_GeneralControls->m_ThresholdingCheckBox->isChecked());
    this->RequestRenderWindowUpdate();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::UpdateCurrentSliceContours(bool updateRendering)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
  int axisNumber = this->GetViewAxis();

  mitk::Image::Pointer workingImage = this->GetWorkingImageFromToolManager(0);
  assert(workingImage);

  mitk::ToolManager::Pointer toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();
  mitk::ContourModelSet::Pointer contourSet = dynamic_cast<mitk::ContourModelSet*>(workingData[mitk::MIDASTool::CONTOURS]->GetData());

  // TODO
  // This assertion fails sometimes if both the morphological and irregular (this) volume editor is
  // switched on and you are using the paintbrush tool of the morpho editor.
//  assert(contourSet);

  if (contourSet)
  {
    if (sliceNumber >= 0 && axisNumber >= 0)
    {
      Self::GenerateOutlineFromBinaryImage(workingImage, axisNumber, sliceNumber, sliceNumber, contourSet);

      if (contourSet->GetSize() > 0)
      {
        workingData[mitk::MIDASTool::CONTOURS]->Modified();

        if (updateRendering)
        {
          this->RequestRenderWindowUpdate();
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::DoesSliceHaveUnenclosedSeeds(const bool& thresholdOn, const int& sliceNumber)
{
  mitk::PointSet* seeds = this->GetSeeds();
  assert(seeds);

  return this->DoesSliceHaveUnenclosedSeeds(thresholdOn, sliceNumber, *seeds);
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::DoesSliceHaveUnenclosedSeeds(const bool& thresholdOn, const int& sliceNumber, mitk::PointSet& seeds)
{
  bool sliceDoesHaveUnenclosedSeeds = false;

  if (!this->HasInitialisedWorkingData())
  {
    return sliceDoesHaveUnenclosedSeeds;
  }

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(0);

  mitk::ToolManager *toolManager = this->GetToolManager();
  assert(toolManager);

  mitk::MIDASPolyTool *polyTool = static_cast<mitk::MIDASPolyTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>()));
  assert(polyTool);

  mitk::ContourModelSet::Pointer polyToolContours = mitk::ContourModelSet::New();
  mitk::ContourModel* polyToolContour = polyTool->GetContour();
  if (polyToolContour != NULL && polyToolContour->GetNumberOfVertices() >= 2)
  {
    polyToolContours->AddContourModel(polyToolContour);
  }

  mitk::ContourModelSet* segmentationContours = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::CONTOURS]->GetData());
  mitk::ContourModelSet* drawToolContours = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::DRAW_CONTOURS]->GetData());

  double lowerThreshold = m_GeneralControls->m_ThresholdsSlider->minimumValue();
  double upperThreshold = m_GeneralControls->m_ThresholdsSlider->maximumValue();

  int axisNumber = this->GetViewAxis();

  if (axisNumber != -1 && sliceNumber != -1)
  {
    try
    {
      AccessFixedDimensionByItk_n(referenceImage, // The reference image is the grey scale image (read only).
        ITKSliceDoesHaveUnEnclosedSeeds, 3,
          (seeds,
           *segmentationContours,
           *polyToolContours,
           *drawToolContours,
           *segmentationImage,
            lowerThreshold,
            upperThreshold,
            thresholdOn,
            axisNumber,
            sliceNumber,
            sliceDoesHaveUnenclosedSeeds
          )
      );
    }
    catch(const mitk::AccessByItkException& e)
    {
      MITK_ERROR << "Caught exception during ITKSliceDoesHaveUnEnclosedSeeds, so will return false, caused by:" << e.what();
    }
  }

  return sliceDoesHaveUnenclosedSeeds;
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::CleanSlice()
{
  /// Note: see comment in SelectSeedTool().
  if (m_ToolSelector->m_ManualToolSelectionBox->isEnabled())
  {
    this->OnCleanButtonClicked();
    return true;
  }

  return false;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnSeePriorCheckBoxToggled(bool checked)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();

  if (checked)
  {
    this->UpdatePriorAndNext();
  }
  workingData[mitk::MIDASTool::PRIOR_CONTOURS]->SetVisibility(checked);
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnSeeNextCheckBoxToggled(bool checked)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();

  if (checked)
  {
    this->UpdatePriorAndNext();
  }
  workingData[mitk::MIDASTool::NEXT_CONTOURS]->SetVisibility(checked);
  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::UpdatePriorAndNext(bool updateRendering)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
  int axisNumber = this->GetViewAxis();

  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();
  mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(0);

  if (m_GeneralControls->m_SeePriorCheckBox->isChecked())
  {
    mitk::ContourModelSet::Pointer contourSet = dynamic_cast<mitk::ContourModelSet*>(workingData[mitk::MIDASTool::PRIOR_CONTOURS]->GetData());
    Self::GenerateOutlineFromBinaryImage(segmentationImage, axisNumber, sliceNumber-1, sliceNumber, contourSet);

    if (contourSet->GetSize() > 0)
    {
      workingData[mitk::MIDASTool::PRIOR_CONTOURS]->Modified();

      if (updateRendering)
      {
        this->RequestRenderWindowUpdate();
      }
    }
  }

  if (m_GeneralControls->m_SeeNextCheckBox->isChecked())
  {
    mitk::ContourModelSet::Pointer contourSet = dynamic_cast<mitk::ContourModelSet*>(workingData[mitk::MIDASTool::NEXT_CONTOURS]->GetData());
    Self::GenerateOutlineFromBinaryImage(segmentationImage, axisNumber, sliceNumber+1, sliceNumber, contourSet);

    if (contourSet->GetSize() > 0)
    {
      workingData[mitk::MIDASTool::NEXT_CONTOURS]->Modified();

      if (updateRendering)
      {
        this->RequestRenderWindowUpdate();
      }
    }
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnThresholdingCheckBoxToggled(bool checked)
{
  if (!this->HasInitialisedWorkingData())
  {
    // So, if there is NO working data, we leave the widgets disabled regardless.
    m_GeneralControls->SetThresholdingWidgetsEnabled(false);
    return;
  }

  this->RecalculateMinAndMaxOfImage();
  this->RecalculateMinAndMaxOfSeedValues();

  m_GeneralControls->SetThresholdingWidgetsEnabled(checked);

  if (checked)
  {
    this->UpdateRegionGrowing();
  }

  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();
  workingData[mitk::MIDASTool::REGION_GROWING]->SetVisibility(checked);

  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnThresholdValueChanged()
{
  this->UpdateRegionGrowing();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::UpdateRegionGrowing(bool updateRendering)
{
  bool isVisible = m_GeneralControls->m_ThresholdingCheckBox->isChecked();
  int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
  double lowerThreshold = m_GeneralControls->m_ThresholdsSlider->minimumValue();
  double upperThreshold = m_GeneralControls->m_ThresholdsSlider->maximumValue();
  bool skipUpdate = !isVisible;

  if (isVisible)
  {
    this->UpdateRegionGrowing(isVisible, sliceNumber, lowerThreshold, upperThreshold, skipUpdate);

    if (updateRendering)
    {
      this->RequestRenderWindowUpdate();
    }
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::UpdateRegionGrowing(
    bool isVisible,
    int sliceNumber,
    double lowerThreshold,
    double upperThreshold,
    bool skipUpdate
    )
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
    mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);

    if (segmentationImage.IsNotNull() && segmentationNode.IsNotNull())
    {

      mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();
      workingData[mitk::MIDASTool::REGION_GROWING]->SetVisibility(isVisible);

      m_IsUpdating = true;

      mitk::DataNode::Pointer regionGrowingNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::REGION_GROWING_NAME.c_str(), segmentationNode, true);
      assert(regionGrowingNode);

      mitk::Image::Pointer regionGrowingImage = dynamic_cast<mitk::Image*>(regionGrowingNode->GetData());
      assert(regionGrowingImage);

      mitk::PointSet* seeds = this->GetSeeds();
      assert(seeds);

      mitk::ToolManager *toolManager = this->GetToolManager();
      assert(toolManager);

      mitk::MIDASPolyTool *polyTool = static_cast<mitk::MIDASPolyTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>()));
      assert(polyTool);

      mitk::ContourModelSet::Pointer polyToolContours = mitk::ContourModelSet::New();

      mitk::ContourModel* polyToolContour = polyTool->GetContour();
      if (polyToolContour != NULL && polyToolContour->GetNumberOfVertices() >= 2)
      {
        polyToolContours->AddContourModel(polyToolContour);
      }

      mitk::ContourModelSet* segmentationContours = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::CONTOURS]->GetData());
      mitk::ContourModelSet* drawToolContours = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::DRAW_CONTOURS]->GetData());

      int axisNumber = this->GetViewAxis();

      if (axisNumber != -1 && sliceNumber != -1)
      {
        try
        {
          AccessFixedDimensionByItk_n(referenceImage, // The reference image is the grey scale image (read only).
              ITKUpdateRegionGrowing, 3,
              (skipUpdate,
               *segmentationImage,
               *seeds,
               *segmentationContours,
               *drawToolContours,
               *polyToolContours,
               sliceNumber,
               axisNumber,
               lowerThreshold,
               upperThreshold,
               regionGrowingNode,  // This is the node for the image we are writing to.
               regionGrowingImage  // This is the image we are writing to.
              )
            );

          regionGrowingImage->Modified();
          regionGrowingNode->Modified();
        }
        catch(const mitk::AccessByItkException& e)
        {
          MITK_ERROR << "Could not do region growing: Caught exception, so abandoning ITK pipeline update:" << e.what();
        }
      }
      else
      {
        MITK_ERROR << "Could not do region growing: Error axisNumber=" << axisNumber << ", sliceNumber=" << sliceNumber << std::endl;
      }

      m_IsUpdating = false;

    }
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnThresholdApplyButtonClicked()
{
  int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
  this->DoThresholdApply(sliceNumber, sliceNumber, true, false, false);
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::DoThresholdApply(
    int oldSliceNumber,
    int newSliceNumber,
    bool optimiseSeeds,
    bool newSliceEmpty,
    bool newCheckboxStatus)
{
  if (!this->HasInitialisedWorkingData())
  {
    return false;
  }

  bool updateWasApplied = false;

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
    mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);

    if (segmentationImage.IsNotNull() && segmentationNode.IsNotNull())
    {
      mitk::DataNode::Pointer regionGrowingNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::REGION_GROWING_NAME.c_str(), segmentationNode, true);
      assert(regionGrowingNode);

      mitk::Image::Pointer regionGrowingImage = dynamic_cast<mitk::Image*>(regionGrowingNode->GetData());
      assert(regionGrowingImage);

      mitk::PointSet* seeds = this->GetSeeds();
      assert(seeds);

      mitk::ToolManager *toolManager = this->GetToolManager();
      assert(toolManager);

      mitk::MIDASDrawTool *drawTool = static_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
      assert(drawTool);

      int axisNumber = this->GetViewAxis();

      mitk::PointSet::Pointer copyOfInputSeeds = mitk::PointSet::New();
      mitk::PointSet::Pointer outputSeeds = mitk::PointSet::New();
      std::vector<int> outputRegion;

      if (axisNumber != -1 && oldSliceNumber != -1)
      {
        m_IsUpdating = true;

        try
        {
          AccessFixedDimensionByItk_n(regionGrowingImage,
              ITKPreProcessingOfSeedsForChangingSlice, 3,
              (*seeds,
               oldSliceNumber,
               axisNumber,
               newSliceNumber,
               optimiseSeeds,
               newSliceEmpty,
               *(copyOfInputSeeds.GetPointer()),
               *(outputSeeds.GetPointer()),
               outputRegion
              )
            );

          bool currentCheckboxStatus = m_GeneralControls->m_ThresholdingCheckBox->isChecked();

          if (toolManager->GetActiveToolID() == toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>())
          {
            toolManager->ActivateTool(-1);
          }

          mitk::UndoStackItem::IncCurrObjectEventId();
          mitk::UndoStackItem::IncCurrGroupEventId();
          mitk::UndoStackItem::ExecuteIncrement();

          QString message = tr("Apply threshold on slice %1").arg(oldSliceNumber);
          mitk::OpThresholdApply::ProcessorPointer processor = mitk::OpThresholdApply::ProcessorType::New();
          mitk::OpThresholdApply *doThresholdOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, true, outputRegion, processor, newCheckboxStatus);
          mitk::OpThresholdApply *undoThresholdOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, false, outputRegion, processor, currentCheckboxStatus);
          mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doThresholdOp, undoThresholdOp, message.toStdString());
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
          this->ExecuteOperation(doThresholdOp);

          message = tr("Propagate seeds on slice %1").arg(oldSliceNumber);
          mitk::OpPropagateSeeds *doPropOp = new mitk::OpPropagateSeeds(OP_PROPAGATE_SEEDS, true, newSliceNumber, axisNumber, outputSeeds);
          mitk::OpPropagateSeeds *undoPropOp = new mitk::OpPropagateSeeds(OP_PROPAGATE_SEEDS, false, oldSliceNumber, axisNumber, copyOfInputSeeds);
          mitk::OperationEvent* operationPropEvent = new mitk::OperationEvent( m_Interface, doPropOp, undoPropOp, message.toStdString());
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationPropEvent );
          this->ExecuteOperation(doPropOp);

          drawTool->ClearWorkingData();

          bool updateRendering(false);
          this->UpdatePriorAndNext(updateRendering);
          this->UpdateRegionGrowing(updateRendering);
          this->UpdateCurrentSliceContours(updateRendering);

          updateWasApplied = true;
        }
        catch(const mitk::AccessByItkException& e)
        {
          MITK_ERROR << "Could not do threshold apply command: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        }
        catch( itk::ExceptionObject &err )
        {
          MITK_ERROR << "Could not do threshold apply command: Caught itk::ExceptionObject:" << err.what() << std::endl;
        }

        m_IsUpdating = false;

      } // end if we have valid axis / slice
    } // end if we have working data
  }// end if we have a reference image

  if (updateWasApplied)
  {
    this->RequestRenderWindowUpdate();
  }
  return updateWasApplied;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnSliceChanged(const itk::EventObject & geometrySliceEvent)
{
  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  if (renderWindowPart != NULL &&  !m_IsChangingSlice)
  {
    int previousSlice = m_PreviousSliceNumber;

    int currentSlice = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
    mitk::Point3D currentFocus = renderWindowPart->GetSelectedPosition();

    if (previousSlice == -1)
    {
      previousSlice = currentSlice;
      m_PreviousFocusPoint = currentFocus;
      m_CurrentFocusPoint = currentFocus;
    }

    this->OnSliceNumberChanged(previousSlice, currentSlice);

    m_PreviousSliceNumber = currentSlice;
    m_PreviousFocusPoint = currentFocus;
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnSliceNumberChanged(int beforeSliceNumber, int afterSliceNumber)
{
  if (  !this->HasInitialisedWorkingData()
      || m_IsUpdating
      || m_IsChangingSlice
      || beforeSliceNumber == -1
      || afterSliceNumber == -1
      || abs(beforeSliceNumber - afterSliceNumber) != 1
      )
  {
    m_PreviousSliceNumber = afterSliceNumber;
    m_PreviousFocusPoint = m_CurrentFocusPoint;

    bool updateRendering(false);
    this->UpdateCurrentSliceContours(updateRendering);
    this->UpdateRegionGrowing(updateRendering);
    this->RequestRenderWindowUpdate();

    return;
  }

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
    mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);

    if (segmentationNode.IsNotNull() && segmentationImage.IsNotNull())
    {
      int axisNumber = this->GetViewAxis();
      MIDASOrientation tmpOrientation = this->GetOrientationAsEnum();
      itk::Orientation orientation = mitk::GetItkOrientation(tmpOrientation);

      mitk::ToolManager *toolManager = this->GetToolManager();
      assert(toolManager);

      mitk::MIDASDrawTool *drawTool = static_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
      assert(drawTool);

      if (   axisNumber != -1
          && beforeSliceNumber != -1
          && afterSliceNumber != -1
          && beforeSliceNumber != afterSliceNumber)
      {
        std::vector<int> outputRegion;
        mitk::PointSet::Pointer copyOfCurrentSeeds = mitk::PointSet::New();
        mitk::PointSet::Pointer propagatedSeeds = mitk::PointSet::New();
        mitk::PointSet* seeds = this->GetSeeds();
        bool nextSliceIsEmpty(true);
        bool thisSliceIsEmpty(false);

        m_IsUpdating = true;

        try
        {
          ///////////////////////////////////////////////////////
          // See: https://cmiclab.cs.ucl.ac.uk/CMIC/NifTK/issues/1742
          //      for the whole logic surrounding changing slice.
          ///////////////////////////////////////////////////////

          AccessFixedDimensionByItk_n(segmentationImage,
              ITKSliceIsEmpty, 3,
              (axisNumber,
               afterSliceNumber,
               nextSliceIsEmpty
              )
            );

          if (m_GeneralControls->m_RetainMarksCheckBox->isChecked())
          {
            int returnValue(QMessageBox::NoButton);

            if (!m_GeneralControls->m_ThresholdingCheckBox->isChecked())
            {
              AccessFixedDimensionByItk_n(segmentationImage,
                  ITKSliceIsEmpty, 3,
                  (axisNumber,
                   beforeSliceNumber,
                   thisSliceIsEmpty
                  )
                );
            }

            if (thisSliceIsEmpty)
            {
              returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                      tr("The current slice is empty - retain marks cannot be performed.\n"
                                                         "Use the 'wipe' functionality to erase slices instead"),
                                                      QMessageBox::Ok
                                   );
            }
            else if (!nextSliceIsEmpty)
            {
              returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                      tr("The new slice is not empty - retain marks will overwrite the slice.\n"
                                                         "Are you sure?"),
                                                      QMessageBox::Yes | QMessageBox::No);
            }

            if (returnValue == QMessageBox::Ok || returnValue == QMessageBox::No )
            {
              m_IsUpdating = false;
              m_PreviousSliceNumber = afterSliceNumber;
              m_PreviousFocusPoint = m_CurrentFocusPoint;
              this->UpdatePriorAndNext();
              this->UpdateRegionGrowing();
              this->UpdateCurrentSliceContours();
              this->RequestRenderWindowUpdate();

              return;
            }

            AccessFixedDimensionByItk_n(segmentationImage,
                ITKPreProcessingOfSeedsForChangingSlice, 3,
                (*seeds,
                 beforeSliceNumber,
                 axisNumber,
                 afterSliceNumber,
                 false, // We propagate seeds at current position, so no optimisation
                 nextSliceIsEmpty,
                 *(copyOfCurrentSeeds.GetPointer()),
                 *(propagatedSeeds.GetPointer()),
                 outputRegion
                )
              );

            if (m_GeneralControls->m_ThresholdingCheckBox->isChecked())
            {
              QString message = tr("Thresholding slice %1 before copying marks to slice %2").arg(beforeSliceNumber).arg(afterSliceNumber);
              mitk::OpThresholdApply::ProcessorPointer processor = mitk::OpThresholdApply::ProcessorType::New();
              mitk::OpThresholdApply *doThresholdOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, true, outputRegion, processor, true);
              mitk::OpThresholdApply *undoThresholdOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, false, outputRegion, processor, true);
              mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doThresholdOp, undoThresholdOp, message.toStdString());
              mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
              this->ExecuteOperation(doThresholdOp);

              drawTool->ClearWorkingData();
              this->UpdateCurrentSliceContours();
            }

            // Do retain marks, which copies slice from beforeSliceNumber to afterSliceNumber
            QString message = tr("Retaining marks in slice %1 and copying to %2").arg(beforeSliceNumber).arg(afterSliceNumber);
            mitk::OpRetainMarks::ProcessorPointer processor = mitk::OpRetainMarks::ProcessorType::New();
            mitk::OpRetainMarks *doOp = new mitk::OpRetainMarks(OP_RETAIN_MARKS, true, beforeSliceNumber, afterSliceNumber, axisNumber, orientation, outputRegion, processor);
            mitk::OpRetainMarks *undoOp = new mitk::OpRetainMarks(OP_RETAIN_MARKS, false, beforeSliceNumber, afterSliceNumber, axisNumber, orientation, outputRegion, processor);
            mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, message.toStdString());
            mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
            this->ExecuteOperation(doOp);
          }
          else // so, "Retain Marks" is Off.
          {
            AccessFixedDimensionByItk_n(segmentationImage,
                ITKPreProcessingOfSeedsForChangingSlice, 3,
                (*seeds,
                 beforeSliceNumber,
                 axisNumber,
                 afterSliceNumber,
                 true, // optimise seed position on current slice.
                 nextSliceIsEmpty,
                 *(copyOfCurrentSeeds.GetPointer()),
                 *(propagatedSeeds.GetPointer()),
                 outputRegion
                )
              );

            if (m_GeneralControls->m_ThresholdingCheckBox->isChecked())
            {
              mitk::OpThresholdApply::ProcessorPointer processor = mitk::OpThresholdApply::ProcessorType::New();
              mitk::OpThresholdApply *doApplyOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, true, outputRegion, processor, m_GeneralControls->m_ThresholdingCheckBox->isChecked());
              mitk::OpThresholdApply *undoApplyOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, false, outputRegion, processor, m_GeneralControls->m_ThresholdingCheckBox->isChecked());
              mitk::OperationEvent* operationApplyEvent = new mitk::OperationEvent( m_Interface, doApplyOp, undoApplyOp, "Apply threshold");
              mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationApplyEvent );
              this->ExecuteOperation(doApplyOp);

              drawTool->ClearWorkingData();
              this->UpdateCurrentSliceContours();
            }
            else // threshold box not checked
            {
              bool thisSliceHasUnenclosedSeeds = this->DoesSliceHaveUnenclosedSeeds(false, beforeSliceNumber);

              if (thisSliceHasUnenclosedSeeds)
              {
                mitk::OpWipe::ProcessorPointer processor = mitk::OpWipe::ProcessorType::New();
                mitk::OpWipe *doWipeOp = new mitk::OpWipe(OP_WIPE, true, beforeSliceNumber, axisNumber, outputRegion, propagatedSeeds, processor);
                mitk::OpWipe *undoWipeOp = new mitk::OpWipe(OP_WIPE, false, beforeSliceNumber, axisNumber, outputRegion, copyOfCurrentSeeds, processor);
                mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doWipeOp, undoWipeOp, "Wipe command");
                mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
                this->ExecuteOperation(doWipeOp);
              }
              else // so, we don't have unenclosed seeds
              {
                // There may be the case where the user has simply drawn a region, and put a seed in the middle.
                // So, we do a region growing, without intensity limits. (we already know there are no unenclosed seeds).

                this->UpdateRegionGrowing(false,
                                          beforeSliceNumber,
                                          referenceImage->GetStatistics()->GetScalarValueMinNoRecompute(),
                                          referenceImage->GetStatistics()->GetScalarValueMaxNoRecompute(),
                                          false);

                // Then we "apply" this region growing.
                mitk::OpThresholdApply::ProcessorPointer processor = mitk::OpThresholdApply::ProcessorType::New();
                mitk::OpThresholdApply *doApplyOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, true, outputRegion, processor, false);
                mitk::OpThresholdApply *undoApplyOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, false, outputRegion, processor, false);
                mitk::OperationEvent* operationApplyEvent = new mitk::OperationEvent( m_Interface, doApplyOp, undoApplyOp, "Apply threshold");
                mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationApplyEvent );
                this->ExecuteOperation(doApplyOp);

                drawTool->ClearWorkingData();

              } // end if/else unenclosed seeds
            } // end if/else thresholding on
          } // end if/else retain marks.

          mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
          if (renderWindowPart != NULL)
          {
            m_CurrentFocusPoint = renderWindowPart->GetSelectedPosition();
          }

          QString message = tr("Propagate seeds from slice %1 to %2").arg(beforeSliceNumber).arg(afterSliceNumber);
          mitk::OpPropagateSeeds *doPropOp = new mitk::OpPropagateSeeds(OP_PROPAGATE_SEEDS, true, afterSliceNumber, axisNumber, propagatedSeeds);
          mitk::OpPropagateSeeds *undoPropOp = new mitk::OpPropagateSeeds(OP_PROPAGATE_SEEDS, false, beforeSliceNumber, axisNumber, copyOfCurrentSeeds);
          mitk::OperationEvent* operationPropEvent = new mitk::OperationEvent( m_Interface, doPropOp, undoPropOp, message.toStdString());
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationPropEvent );
          this->ExecuteOperation(doPropOp);

          message = tr("Change slice from %1 to %2").arg(beforeSliceNumber).arg(afterSliceNumber);
          mitk::OpChangeSliceCommand *doOp = new mitk::OpChangeSliceCommand(OP_CHANGE_SLICE, true, beforeSliceNumber, afterSliceNumber, m_PreviousFocusPoint, m_CurrentFocusPoint);
          mitk::OpChangeSliceCommand *undoOp = new mitk::OpChangeSliceCommand(OP_CHANGE_SLICE, false, beforeSliceNumber, afterSliceNumber, m_PreviousFocusPoint, m_CurrentFocusPoint);
          mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, message.toStdString());
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
          this->ExecuteOperation(doOp);
        }
        catch(const mitk::AccessByItkException& e)
        {
          MITK_ERROR << "Could not change slice: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        }
        catch( itk::ExceptionObject &err )
        {
          MITK_ERROR << "Could not change slice: Caught itk::ExceptionObject:" << err.what() << std::endl;
        }

        m_IsUpdating = false;

        if (mitk::MIDASPolyTool* polyTool = dynamic_cast<mitk::MIDASPolyTool*>(toolManager->GetActiveTool()))
        {
//          toolManager->ActivateTool(-1);
          /// This makes the poly tool save its result to the working data nodes and stay it open.
          polyTool->Deactivated();
          polyTool->Activated();
        }

        bool updateRendering(false);
        this->UpdatePriorAndNext(updateRendering);
        this->UpdateRegionGrowing(updateRendering);
        this->UpdateCurrentSliceContours(updateRendering);
        this->RequestRenderWindowUpdate();

      } // end if, slice number, axis ok.
    } // end have working image
  } // end have reference image
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnCleanButtonClicked()
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  bool thresholdCheckBox = m_GeneralControls->m_ThresholdingCheckBox->isChecked();
  int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();

  if (!thresholdCheckBox)
  {
    bool hasUnenclosedSeeds = this->DoesSliceHaveUnenclosedSeeds(thresholdCheckBox, sliceNumber);
    if (hasUnenclosedSeeds)
    {
      int returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                       tr("There are unenclosed seeds - slice will be wiped\n"
                                                          "Are you sure?"),
                                                       QMessageBox::Yes | QMessageBox::No);
      if (returnValue == QMessageBox::Yes)
      {
        this->DoWipe(0);
      }
      return;
    }
  }

  bool cleanWasPerformed = false;

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {
    mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
    mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);

    if (segmentationImage.IsNotNull() && segmentationNode.IsNotNull())
    {
      mitk::PointSet* seeds = this->GetSeeds();
      assert(seeds);

      mitk::ToolManager *toolManager = this->GetToolManager();
      assert(toolManager);

      mitk::MIDASPolyTool *polyTool = static_cast<mitk::MIDASPolyTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>()));
      assert(polyTool);

      mitk::MIDASDrawTool *drawTool = static_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
      assert(drawTool);

      mitk::ContourModelSet::Pointer polyToolContours = mitk::ContourModelSet::New();

      mitk::ContourModel* polyToolContour = polyTool->GetContour();
      if (polyToolContour != NULL && polyToolContour->GetNumberOfVertices() >= 2)
      {
        polyToolContours->AddContourModel(polyToolContour);
      }

      mitk::ContourModelSet* segmentationContours = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::CONTOURS]->GetData());
      assert(segmentationContours);

      mitk::ContourModelSet* drawToolContours = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::DRAW_CONTOURS]->GetData());
      assert(drawToolContours);

      mitk::DataNode::Pointer regionGrowingNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::REGION_GROWING_NAME.c_str(), segmentationNode, true);
      assert(regionGrowingNode);

      mitk::Image::Pointer regionGrowingImage = dynamic_cast<mitk::Image*>(regionGrowingNode->GetData());
      assert(regionGrowingImage);

      double lowerThreshold = m_GeneralControls->m_ThresholdsSlider->minimumValue();
      double upperThreshold = m_GeneralControls->m_ThresholdsSlider->maximumValue();
      int axisNumber = this->GetViewAxis();

      mitk::ContourModelSet::Pointer copyOfInputContourSet = mitk::ContourModelSet::New();
      mitk::ContourModelSet::Pointer outputContourSet = mitk::ContourModelSet::New();

      if (axisNumber != -1 && sliceNumber != -1)
      {
        m_IsUpdating = true;

        try
        {
          // Calculate the region of interest for this slice.
          std::vector<int> outputRegion;
          AccessFixedDimensionByItk_n(segmentationImage,
              ITKCalculateSliceRegionAsVector, 3,
              (axisNumber,
               sliceNumber,
               outputRegion
              )
            );

          if (thresholdCheckBox)
          {
            bool useThresholdsWhenCalculatingEnclosedSeeds = false;

            this->DoThresholdApply(sliceNumber, sliceNumber, true, false, true);

            // Get seeds just on the current slice
            mitk::PointSet::Pointer seedsForCurrentSlice = mitk::PointSet::New();
            this->FilterSeedsToCurrentSlice(
                *seeds,
                axisNumber,
                sliceNumber,
                *(seedsForCurrentSlice.GetPointer())
                );

            // Reduce the list just down to those that are fully enclosed.
            mitk::PointSet::Pointer enclosedSeeds = mitk::PointSet::New();
            this->FilterSeedsToEnclosedSeedsOnCurrentSlice(
                *seedsForCurrentSlice,
                useThresholdsWhenCalculatingEnclosedSeeds,
                sliceNumber,
                *(enclosedSeeds.GetPointer())
                );

            // Do region growing, using only enclosed seeds.
            AccessFixedDimensionByItk_n(referenceImage, // The reference image is the grey scale image (read only).
                ITKUpdateRegionGrowing, 3,
                (false,
                 *segmentationImage,
                 *enclosedSeeds,
                 *segmentationContours,
                 *drawToolContours,
                 *polyToolContours,
                 sliceNumber,
                 axisNumber,
                 lowerThreshold,
                 upperThreshold,
                 regionGrowingNode,  // This is the node for the image we are writing to.
                 regionGrowingImage  // This is the image we are writing to.
                )
            );

            // Copy to segmentation image.
            typedef itk::Image<unsigned char, 3> ImageType;
            typedef mitk::ImageToItk< ImageType > ImageToItkType;

            ImageToItkType::Pointer regionGrowingToItk = ImageToItkType::New();
            regionGrowingToItk->SetInput(regionGrowingImage);
            regionGrowingToItk->Update();

            ImageToItkType::Pointer outputToItk = ImageToItkType::New();
            outputToItk->SetInput(segmentationImage);
            outputToItk->Update();

            this->ITKCopyRegion<unsigned char, 3>(
                regionGrowingToItk->GetOutput(),
                axisNumber,
                sliceNumber,
                outputToItk->GetOutput()
                );

            regionGrowingToItk = NULL;
            outputToItk = NULL;

            // Update the current slice contours, to regenerate cleaned orange contours
            // around just the regions of interest that have a valid seed.
            this->UpdateCurrentSliceContours();
          }
          else
          {
            // Here we are not thresholding.

            // However, we can assume that all seeds are enclosed.
            // If the seeds were not all enclosed, the user received warning earlier,
            // and either abandoned this method, or accepted the warning and wiped the slice.

            AccessFixedDimensionByItk_n(referenceImage, // The reference image is the grey scale image (read only).
                ITKUpdateRegionGrowing, 3,
                (false,
                 *segmentationImage,
                 *seeds,
                 *segmentationContours,
                 *drawToolContours,
                 *polyToolContours,
                 sliceNumber,
                 axisNumber,
                 referenceImage->GetStatistics()->GetScalarValueMinNoRecompute(),
                 referenceImage->GetStatistics()->GetScalarValueMaxNoRecompute(),
                 regionGrowingNode,  // This is the node for the image we are writing to.
                 regionGrowingImage  // This is the image we are writing to.
                )
            );

          }

          // Then create filtered contours for the current slice.
          // So, if we are thresholding, we fit them round the current region growing image,
          // which if we have just used enclosed seeds above, will not include regions defined
          // by a seed and a threshold, but that have not been "applied" yet.

          AccessFixedDimensionByItk_n(referenceImage, // The reference image is the grey scale image (read only).
              ITKFilterContours, 3,
              (*segmentationImage,
               *seeds,
               *segmentationContours,
               *drawToolContours,
               *polyToolContours,
               axisNumber,
               sliceNumber,
               lowerThreshold,
               upperThreshold,
               m_GeneralControls->m_ThresholdingCheckBox->isChecked(),
               *(copyOfInputContourSet.GetPointer()),
               *(outputContourSet.GetPointer())
              )
            );

          mitk::UndoStackItem::IncCurrObjectEventId();
          mitk::UndoStackItem::IncCurrGroupEventId();
          mitk::UndoStackItem::ExecuteIncrement();

          mitk::OpClean *doOp = new mitk::OpClean(OP_CLEAN, true, outputContourSet);
          mitk::OpClean *undoOp = new mitk::OpClean(OP_CLEAN, false, copyOfInputContourSet);
          mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, "Clean: Filtering contours");
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
          this->ExecuteOperation(doOp);

          // Then we update the region growing to get up-to-date contours.
          this->UpdateRegionGrowing();

          if (!m_GeneralControls->m_ThresholdingCheckBox->isChecked())
          {
            // Then we "apply" this region growing.
            mitk::OpThresholdApply::ProcessorPointer processor = mitk::OpThresholdApply::ProcessorType::New();
            mitk::OpThresholdApply *doApplyOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, true, outputRegion, processor, m_GeneralControls->m_ThresholdingCheckBox->isChecked());
            mitk::OpThresholdApply *undoApplyOp = new mitk::OpThresholdApply(OP_THRESHOLD_APPLY, false, outputRegion, processor, m_GeneralControls->m_ThresholdingCheckBox->isChecked());
            mitk::OperationEvent* operationApplyEvent = new mitk::OperationEvent( m_Interface, doApplyOp, undoApplyOp, "Clean: Calculate new image");
            mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationApplyEvent );
            this->ExecuteOperation(doApplyOp);

            // We should update the current slice contours, as the green contours
            // are the current segmentation that will be applied when we change slice.
            this->UpdateCurrentSliceContours();
          }

          drawTool->Clean(sliceNumber, axisNumber);

          segmentationImage->Modified();
          segmentationNode->Modified();

          cleanWasPerformed = true;

        }
        catch(const mitk::AccessByItkException& e)
        {
          MITK_ERROR << "Could not do clean command: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        }
        catch( itk::ExceptionObject &err )
        {
          MITK_ERROR << "Could not do clean command: Caught itk::ExceptionObject:" << err.what() << std::endl;
        }

        m_IsUpdating = false;

      }
      else
      {
        MITK_ERROR << "Could not do clean operation: Error axisNumber=" << axisNumber << ", sliceNumber=" << sliceNumber << std::endl;
      }
    }
  }

  if (cleanWasPerformed)
  {
    this->RequestRenderWindowUpdate();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnWipeButtonClicked()
{
  this->DoWipe(0);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnWipePlusButtonClicked()
{

  MIDASOrientation midasOrientation = this->GetOrientationAsEnum();

  QString orientationText;
  QString messageWithOrientation = "All slices %1 the present will be cleared \nAre you sure?";

  if (midasOrientation == MIDAS_ORIENTATION_AXIAL)
  {
    orientationText = "superior to";
  }
  else if (midasOrientation == MIDAS_ORIENTATION_SAGITTAL)
  {
    orientationText = "right of";
  }
  else if (midasOrientation == MIDAS_ORIENTATION_CORONAL)
  {
    orientationText = "anterior to";
  }
  else
  {
    orientationText = "up from";
  }

  int returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                            tr(messageWithOrientation.toStdString().c_str()).arg(orientationText),
                                                            QMessageBox::Yes | QMessageBox::No);
  if (returnValue == QMessageBox::No)
  {
    return;
  }

  this->DoWipe(1);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnWipeMinusButtonClicked()
{
  MIDASOrientation midasOrientation = this->GetOrientationAsEnum();

  QString orientationText;
  QString messageWithOrientation = "All slices %1 the present will be cleared \nAre you sure?";

  if (midasOrientation == MIDAS_ORIENTATION_AXIAL)
  {
    orientationText = "inferior to";
  }
  else if (midasOrientation == MIDAS_ORIENTATION_SAGITTAL)
  {
    orientationText = "left of";
  }
  else if (midasOrientation == MIDAS_ORIENTATION_CORONAL)
  {
    orientationText = "posterior to";
  }
  else
  {
    orientationText = "down from";
  }

  int returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                            tr(messageWithOrientation.toStdString().c_str()).arg(orientationText),
                                                            QMessageBox::Yes | QMessageBox::No);
  if (returnValue == QMessageBox::No)
  {
    return;
  }

  this->DoWipe(-1);
}


//-----------------------------------------------------------------------------
bool MIDASGeneralSegmentorView::DoWipe(int direction)
{
  bool wipeWasPerformed = false;

  if (!this->HasInitialisedWorkingData())
  {
    return wipeWasPerformed;
  }

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {

    mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
    mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);

    if (segmentationImage.IsNotNull() && segmentationNode.IsNotNull())
    {
      mitk::PointSet* seeds = this->GetSeeds();
      assert(seeds);

      int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
      int axisNumber = this->GetViewAxis();
      int upDirection = this->GetUpDirection();

      if (direction != 0) // zero means, current slice.
      {
        direction = direction*upDirection;
      }

      mitk::PointSet::Pointer copyOfInputSeeds = mitk::PointSet::New();
      mitk::PointSet::Pointer outputSeeds = mitk::PointSet::New();
      std::vector<int> outputRegion;

      if (axisNumber != -1 && sliceNumber != -1)
      {

        m_IsUpdating = true;

        try
        {

          mitk::ToolManager *toolManager = this->GetToolManager();
          assert(toolManager);

          mitk::MIDASDrawTool *drawTool = static_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
          assert(drawTool);

          if (toolManager->GetActiveToolID() == toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>())
          {
            toolManager->ActivateTool(-1);
          }


          if (direction == 0)
          {
            mitk::CopyPointSets(*seeds, *copyOfInputSeeds);
            mitk::CopyPointSets(*seeds, *outputSeeds);

            AccessFixedDimensionByItk_n(segmentationImage,
                ITKCalculateSliceRegionAsVector, 3,
                (axisNumber,
                 sliceNumber,
                 outputRegion
                )
              );

          }
          else
          {
            AccessFixedDimensionByItk_n(segmentationImage, // The binary image = current segmentation
                ITKPreProcessingForWipe, 3,
                (*seeds,
                 sliceNumber,
                 axisNumber,
                 direction,
                 *(copyOfInputSeeds.GetPointer()),
                 *(outputSeeds.GetPointer()),
                 outputRegion
                )
              );
          }

          mitk::UndoStackItem::IncCurrObjectEventId();
          mitk::UndoStackItem::IncCurrGroupEventId();
          mitk::UndoStackItem::ExecuteIncrement();

          mitk::OpWipe::ProcessorPointer processor = mitk::OpWipe::ProcessorType::New();
          mitk::OpWipe *doOp = new mitk::OpWipe(OP_WIPE, true, sliceNumber, axisNumber, outputRegion, outputSeeds, processor);
          mitk::OpWipe *undoOp = new mitk::OpWipe(OP_WIPE, false, sliceNumber, axisNumber, outputRegion, copyOfInputSeeds, processor);
          mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, "Wipe command");
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
          this->ExecuteOperation(doOp);

          drawTool->ClearWorkingData();
          this->UpdateCurrentSliceContours();

          // Successful outcome.
          wipeWasPerformed = true;
        }
        catch(const mitk::AccessByItkException& e)
        {
          MITK_ERROR << "Could not do wipe command: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        }
        catch( itk::ExceptionObject &err )
        {
          MITK_ERROR << "Could not do wipe command: Caught itk::ExceptionObject:" << err.what() << std::endl;
        }

        m_IsUpdating = false;
      }
      else
      {
        MITK_ERROR << "Could not wipe: Error, axisNumber=" << axisNumber << ", sliceNumber=" << sliceNumber << std::endl;
      }
    }
  }

  if (wipeWasPerformed)
  {
    this->RequestRenderWindowUpdate();
  }

  return wipeWasPerformed;
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnPropagate3DButtonClicked()
{
  this->DoPropagate(false, true);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnPropagateUpButtonClicked()
{
  this->DoPropagate(true, false);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnPropagateDownButtonClicked()
{
  this->DoPropagate(false, false);
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::DoPropagate(bool isUp, bool is3D)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  MIDASOrientation midasOrientation = this->GetOrientationAsEnum();
  itk::Orientation orientation = mitk::GetItkOrientation(midasOrientation);

  QString message;

  if (is3D)
  {
    message = "All slices will be over-written";
  }
  else
  {
    QString orientationText;
    QString messageWithOrientation = "All slices %1 the present will be over-written";

    if (isUp)
    {
      if (midasOrientation == MIDAS_ORIENTATION_AXIAL)
      {
        orientationText = "superior to";
      }
      else if (midasOrientation == MIDAS_ORIENTATION_SAGITTAL)
      {
        orientationText = "right of";
      }
      else if (midasOrientation == MIDAS_ORIENTATION_CORONAL)
      {
        orientationText = "anterior to";
      }
      else
      {
        orientationText = "up from";
      }
    }
    else if (!isUp)
    {
      if (midasOrientation == MIDAS_ORIENTATION_AXIAL)
      {
        orientationText = "inferior to";
      }
      else if (midasOrientation == MIDAS_ORIENTATION_SAGITTAL)
      {
        orientationText = "left of";
      }
      else if (midasOrientation == MIDAS_ORIENTATION_CORONAL)
      {
        orientationText = "posterior to";
      }
      else
      {
        orientationText = "up from";
      }
    }

    message = tr(messageWithOrientation.toStdString().c_str()).arg(orientationText);
  }

  int returnValue = QMessageBox::warning(this->GetParent(), tr("NiftyView"),
                                                   tr("%1.\n"
                                                      "Are you sure?").arg(message),
                                                   QMessageBox::Yes | QMessageBox::No);
  if (returnValue == QMessageBox::No)
  {
    return;
  }

  mitk::Image::Pointer referenceImage = this->GetReferenceImageFromToolManager();
  if (referenceImage.IsNotNull())
  {

    mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
    mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);

    if (segmentationImage.IsNotNull() && segmentationNode.IsNotNull())
    {

      mitk::DataNode::Pointer regionGrowingNode = this->GetDataStorage()->GetNamedDerivedNode(mitk::MIDASTool::REGION_GROWING_NAME.c_str(), segmentationNode, true);
      assert(regionGrowingNode);

      mitk::Image::Pointer regionGrowingImage = dynamic_cast<mitk::Image*>(regionGrowingNode->GetData());
      assert(regionGrowingImage);

      mitk::PointSet* seeds = this->GetSeeds();
      assert(seeds);

      mitk::ToolManager *toolManager = this->GetToolManager();
      assert(toolManager);

      mitk::MIDASDrawTool *drawTool = static_cast<mitk::MIDASDrawTool*>(toolManager->GetToolById(toolManager->GetToolIdByToolType<mitk::MIDASDrawTool>()));
      assert(drawTool);

      double lowerThreshold = m_GeneralControls->m_ThresholdsSlider->minimumValue();
      double upperThreshold = m_GeneralControls->m_ThresholdsSlider->maximumValue();
      int sliceNumber = this->GetSliceNumberFromSliceNavigationControllerAndReferenceImage();
      int axisNumber = this->GetViewAxis();
      int direction = this->GetUpDirection();
      if (!is3D && !isUp)
      {
        direction *= -1;
      }
      else if (is3D)
      {
        direction = 0;
      }

      mitk::PointSet::Pointer copyOfInputSeeds = mitk::PointSet::New();
      mitk::PointSet::Pointer outputSeeds = mitk::PointSet::New();
      std::vector<int> outputRegion;

      if (axisNumber != -1 && sliceNumber != -1 && orientation != itk::ORIENTATION_UNKNOWN)
      {

        m_IsUpdating = true;

        try
        {
          AccessFixedDimensionByItk_n(referenceImage, // The reference image is the grey scale image (read only).
              ITKPropagateToRegionGrowingImage, 3,
              (*seeds,
               sliceNumber,
               axisNumber,
               direction,
               lowerThreshold,
               upperThreshold,
               *(copyOfInputSeeds.GetPointer()),
               *(outputSeeds.GetPointer()),
               outputRegion,
               regionGrowingNode,  // This is the node for the image we are writing to.
               regionGrowingImage  // This is the image we are writing to.
              )
            );

          if (toolManager->GetActiveToolID() == toolManager->GetToolIdByToolType<mitk::MIDASPolyTool>())
          {
            toolManager->ActivateTool(-1);
          }

          mitk::UndoStackItem::IncCurrObjectEventId();
          mitk::UndoStackItem::IncCurrGroupEventId();
          mitk::UndoStackItem::ExecuteIncrement();

          QString message = tr("Propagate: copy region growing");
          mitk::OpPropagate::ProcessorPointer processor = mitk::OpPropagate::ProcessorType::New();
          mitk::OpPropagate *doPropOp = new mitk::OpPropagate(OP_PROPAGATE, true, outputRegion, processor);
          mitk::OpPropagate *undoPropOp = new mitk::OpPropagate(OP_PROPAGATE, false, outputRegion, processor);
          mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doPropOp, undoPropOp, message.toStdString());
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
          this->ExecuteOperation(doPropOp);

          message = tr("Propagate: copy seeds");
          mitk::OpPropagateSeeds *doPropSeedsOp = new mitk::OpPropagateSeeds(OP_PROPAGATE_SEEDS, true, sliceNumber, axisNumber, outputSeeds);
          mitk::OpPropagateSeeds *undoPropSeedsOp = new mitk::OpPropagateSeeds(OP_PROPAGATE_SEEDS, false, sliceNumber, axisNumber, copyOfInputSeeds);
          mitk::OperationEvent* operationPropEvent = new mitk::OperationEvent( m_Interface, doPropSeedsOp, undoPropSeedsOp, message.toStdString());
          mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationPropEvent );
          this->ExecuteOperation(doPropOp);

          drawTool->ClearWorkingData();
          this->UpdateCurrentSliceContours(false);
          this->UpdateRegionGrowing(false);
        }
        catch(const mitk::AccessByItkException& e)
        {
          MITK_ERROR << "Could not propagate: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        }
        catch( itk::ExceptionObject &err )
        {
          MITK_ERROR << "Could not propagate: Caught itk::ExceptionObject:" << err.what() << std::endl;
        }

        m_IsUpdating = false;
      }
      else
      {
        MITK_ERROR << "Could not propagate: Error axisNumber=" << axisNumber << ", sliceNumber=" << sliceNumber << ", orientation=" << orientation << ", direction=" << direction << std::endl;
      }
    }
  }

  this->RequestRenderWindowUpdate();
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnAnyButtonClicked()
{
  /// Set the focus back to the main window. This is needed so that the keyboard shortcuts
  /// (like 'a' and 'z' for changing slice) keep on working.
  if (QmitkRenderWindow* mainWindow = this->GetSelectedRenderWindow())
  {
    mainWindow->setFocus();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::NodeChanged(const mitk::DataNode* node)
{
  if (   m_IsDeleting
      || m_IsUpdating
      || !this->HasInitialisedWorkingData()
      )
  {
    return;
  }

  mitk::ToolManager::DataVectorType workingData = this->GetWorkingData();
  if (workingData.size() > 0)
  {
    bool seedsChanged(false);
    bool drawContoursChanged(false);

    if (workingData[mitk::MIDASTool::SEEDS] != NULL && workingData[mitk::MIDASTool::SEEDS] == node)
    {
      seedsChanged = true;
    }
    if (workingData[mitk::MIDASTool::DRAW_CONTOURS] != NULL && workingData[mitk::MIDASTool::DRAW_CONTOURS] == node)
    {
      drawContoursChanged = true;
    }

    if (!seedsChanged && !drawContoursChanged)
    {
      return;
    }

    mitk::DataNode::Pointer segmentationImageNode = workingData[mitk::MIDASTool::SEGMENTATION];
    if (segmentationImageNode.IsNotNull())
    {
      mitk::PointSet* seeds = this->GetSeeds();
      if (seeds != NULL && seeds->GetSize() > 0)
      {

        bool contourIsBeingEdited(false);
        if (segmentationImageNode.GetPointer() == node)
        {
          segmentationImageNode->GetBoolProperty(mitk::MIDASContourTool::EDITING_PROPERTY_NAME.c_str(), contourIsBeingEdited);
        }

        if (!contourIsBeingEdited)
        {
          if (seedsChanged)
          {
            this->RecalculateMinAndMaxOfSeedValues();
          }

          if (seedsChanged || drawContoursChanged)
          {
            this->UpdateRegionGrowing();
          }
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::NodeRemoved(const mitk::DataNode* removedNode)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  mitk::DataNode::Pointer segmentationNode = this->GetToolManager()->GetWorkingData(mitk::MIDASTool::SEGMENTATION);

  if (segmentationNode.GetPointer() == removedNode)
  {
    this->DiscardSegmentation();
  }
}


//-----------------------------------------------------------------------------
void MIDASGeneralSegmentorView::OnContoursChanged()
{
  this->UpdateRegionGrowing();
}


/**************************************************************
 * End of: The main MIDAS business logic.
 *************************************************************/

/******************************************************************
 * Start of ExecuteOperation - main method in Undo/Redo framework.
 *
 * Notes: In this method, we update items, using the given
 * operation. We do not know if this is a "Undo" or a "Redo"
 * type of operation. We can set the modified field.
 * But do not be tempted to put things like:
 *
 * this->RequestRenderWindowUpdate();
 *
 * or
 *
 * this->UpdateRegionGrowing() etc.
 *
 * as these methods may be called multiple times during one user
 * operation. So the methods creating the mitk::Operation objects
 * should also be the ones deciding when we update the display.
 ******************************************************************/

void MIDASGeneralSegmentorView::ExecuteOperation(mitk::Operation* operation)
{
  if (!this->HasInitialisedWorkingData())
  {
    return;
  }

  if (!operation) return;

  mitk::Image::Pointer segmentationImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::SEGMENTATION);
  assert(segmentationImage);

  mitk::DataNode::Pointer segmentationNode = this->GetWorkingData()[mitk::MIDASTool::SEGMENTATION];
  assert(segmentationNode);

  mitk::Image* referenceImage = this->GetReferenceImageFromToolManager();
  assert(referenceImage);

  mitk::Image* regionGrowingImage = this->GetWorkingImageFromToolManager(mitk::MIDASTool::REGION_GROWING);
  assert(regionGrowingImage);

  mitk::PointSet* seeds = this->GetSeeds();
  assert(seeds);

  mitk::DataNode::Pointer seedsNode = this->GetWorkingData()[mitk::MIDASTool::SEEDS];
  assert(seedsNode);

  mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
  assert(renderWindowPart);

  switch (operation->GetOperationType())
  {
  case OP_CHANGE_SLICE:
    {
      // Simply to make sure we can switch slice, and undo/redo it.
      mitk::OpChangeSliceCommand* op = dynamic_cast<mitk::OpChangeSliceCommand*>(operation);
      assert(op);

      mitk::Point3D currentPoint = renderWindowPart->GetSelectedPosition();

      mitk::Point3D beforePoint = op->GetBeforePoint();
      mitk::Point3D afterPoint = op->GetAfterPoint();
      int beforeSlice = op->GetBeforeSlice();
      int afterSlice = op->GetAfterSlice();

      mitk::Point3D selectedPoint;

      if (op->IsRedo())
      {
        selectedPoint = afterPoint;
      }
      else
      {
        selectedPoint = beforePoint;
      }

      // Only move if we are not already on this slice.
      // Better to compare integers than floating point numbers.
      if (beforeSlice != afterSlice)
      {
        m_IsChangingSlice = true;
        renderWindowPart->SetSelectedPosition(selectedPoint);
        m_IsChangingSlice = false;
      }

      break;
    }
  case OP_PROPAGATE_SEEDS:
    {
      mitk::OpPropagateSeeds* op = dynamic_cast<mitk::OpPropagateSeeds*>(operation);
      assert(op);

      mitk::PointSet* newSeeds = op->GetSeeds();
      assert(newSeeds);

      mitk::CopyPointSets(*newSeeds, *seeds);

      seeds->Modified();
      seedsNode->Modified();

      break;
    }
  case OP_RETAIN_MARKS:
    {
      try
      {
        mitk::OpRetainMarks* op = static_cast<mitk::OpRetainMarks*>(operation);
        assert(op);

        mitk::OpRetainMarks::ProcessorType::Pointer processor = op->GetProcessor();
        bool redo = op->IsRedo();
        int fromSlice = op->GetFromSlice();
        int toSlice = op->GetToSlice();
        itk::Orientation orientation = op->GetOrientation();

        typedef mitk::ImageToItk< BinaryImage3DType > SegmentationImageToItkType;
        SegmentationImageToItkType::Pointer targetImageToItk = SegmentationImageToItkType::New();
        targetImageToItk->SetInput(segmentationImage);
        targetImageToItk->Update();

        processor->SetSourceImage(targetImageToItk->GetOutput());
        processor->SetDestinationImage(targetImageToItk->GetOutput());
        processor->SetSlices(orientation, fromSlice, toSlice);

        if (redo)
        {
          processor->Redo();
        }
        else
        {
          processor->Undo();
        }

        targetImageToItk = NULL;

        mitk::Image::Pointer outputImage = mitk::ImportItkImage( processor->GetDestinationImage());

        processor->SetSourceImage(NULL);
        processor->SetDestinationImage(NULL);

        segmentationNode->SetData(outputImage);
        segmentationNode->Modified();
      }
      catch( itk::ExceptionObject &err )
      {
        MITK_ERROR << "Could not do retain marks: Caught itk::ExceptionObject:" << err.what() << std::endl;
        return;
      }

      break;
    }
  case OP_THRESHOLD_APPLY:
    {
      mitk::OpThresholdApply *op = dynamic_cast<mitk::OpThresholdApply*>(operation);
      assert(op);

      try
      {
        AccessFixedDimensionByItk_n(referenceImage, ITKPropagateToSegmentationImage, 3,
              (
                segmentationImage,
                regionGrowingImage,
                op
              )
            );

        m_GeneralControls->m_ThresholdingCheckBox->blockSignals(true);
        m_GeneralControls->m_ThresholdingCheckBox->setChecked(op->GetThresholdFlag());
        m_GeneralControls->m_ThresholdingCheckBox->blockSignals(false);
        m_GeneralControls->SetThresholdingWidgetsEnabled(op->GetThresholdFlag());

        segmentationImage->Modified();
        segmentationNode->Modified();

        regionGrowingImage->Modified();

      }
      catch(const mitk::AccessByItkException& e)
      {
        MITK_ERROR << "Could not do threshold: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        return;
      }
      catch( itk::ExceptionObject &err )
      {
        MITK_ERROR << "Could not do threshold: Caught itk::ExceptionObject:" << err.what() << std::endl;
        return;
      }

      break;
    }
  case OP_CLEAN:
    {
      try
      {
        mitk::OpClean* op = dynamic_cast<mitk::OpClean*>(operation);
        assert(op);

        mitk::ContourModelSet* newContours = op->GetContourSet();
        assert(newContours);

        mitk::ContourModelSet* contoursToReplace = dynamic_cast<mitk::ContourModelSet*>(this->GetWorkingData()[mitk::MIDASTool::CONTOURS]->GetData());
        assert(contoursToReplace);

        mitk::MIDASContourTool::CopyContourSet(*newContours, *contoursToReplace);
        contoursToReplace->Modified();
        this->GetWorkingData()[mitk::MIDASTool::CONTOURS]->Modified();

        segmentationImage->Modified();
        segmentationNode->Modified();

      }
      catch( itk::ExceptionObject &err )
      {
        MITK_ERROR << "Could not do clean: Caught itk::ExceptionObject:" << err.what() << std::endl;
        return;
      }

      break;
    }
  case OP_WIPE:
    {
      mitk::OpWipe *op = dynamic_cast<mitk::OpWipe*>(operation);
      assert(op);

      try
      {
        AccessFixedTypeByItk_n(segmentationImage,
            ITKDoWipe,
            (unsigned char),
            (3),
              (
                seeds,
                op
              )
            );

        segmentationImage->Modified();
        segmentationNode->Modified();

      }
      catch(const mitk::AccessByItkException& e)
      {
        MITK_ERROR << "Could not do wipe: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        return;
      }
      catch( itk::ExceptionObject &err )
      {
        MITK_ERROR << "Could not do wipe: Caught itk::ExceptionObject:" << err.what() << std::endl;
        return;
      }

      break;
    }
  case OP_PROPAGATE:
    {
      mitk::OpPropagate *op = dynamic_cast<mitk::OpPropagate*>(operation);
      assert(op);

      try
      {
        AccessFixedDimensionByItk_n(referenceImage, ITKPropagateToSegmentationImage, 3,
              (
                segmentationImage,
                regionGrowingImage,
                op
              )
            );

        segmentationImage->Modified();
        segmentationNode->Modified();

      }
      catch(const mitk::AccessByItkException& e)
      {
        MITK_ERROR << "Could not do propagation: Caught mitk::AccessByItkException:" << e.what() << std::endl;
        return;
      }
      catch( itk::ExceptionObject &err )
      {
        MITK_ERROR << "Could not do propagation: Caught itk::ExceptionObject:" << err.what() << std::endl;
        return;
      }
      break;
    }
  default:;
  }
}

/******************************************************************
 * End of ExecuteOperation - main method in Undo/Redo framework.
 ******************************************************************/

/**************************************************************
 * Start of ITK stuff.
 *
 * Notes: All code below this should never set the Modified
 * flag. The ITK layer, just does basic iterating, basic
 * low level image processing. It knows nothing of node
 * properties, or undo/redo.
 *************************************************************/

//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKFillRegion(
    itk::Image<TPixel, VImageDimension>* itkImage,
    typename itk::Image<TPixel, VImageDimension>::RegionType &region,
    TPixel fillValue
    )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  itk::ImageRegionIterator<ImageType> iter(itkImage, region);

  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    iter.Set(fillValue);
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKClearImage(itk::Image<TPixel, VImageDimension>* itkImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;

  RegionType largestPossibleRegion = itkImage->GetLargestPossibleRegion();
  Self::ITKFillRegion(itkImage, largestPossibleRegion, (TPixel)0);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void MIDASGeneralSegmentorView::ITKCopyImage(
    itk::Image<TPixel, VImageDimension>* input,
    itk::Image<TPixel, VImageDimension>* output
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  itk::ImageRegionConstIterator<ImageType> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<ImageType> outputIterator(output, output->GetLargestPossibleRegion());

  for (inputIterator.GoToBegin(), outputIterator.GoToBegin();
      !inputIterator.IsAtEnd() && !outputIterator.IsAtEnd();
      ++inputIterator, ++outputIterator
      )
  {
    outputIterator.Set(inputIterator.Get());
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKCopyRegion(
    itk::Image<TPixel, VImageDimension>* input,
    int axis,
    int slice,
    itk::Image<TPixel, VImageDimension>* output
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;

  RegionType sliceRegion;
  Self::ITKCalculateSliceRegion(input, axis, slice, sliceRegion);

  itk::ImageRegionConstIterator<ImageType> inputIterator(input, sliceRegion);
  itk::ImageRegionIterator<ImageType> outputIterator(output, sliceRegion);

  for (inputIterator.GoToBegin(), outputIterator.GoToBegin(); !inputIterator.IsAtEnd(); ++inputIterator, ++outputIterator)
  {
    outputIterator.Set(inputIterator.Get());
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKCalculateSliceRegion(
    itk::Image<TPixel, VImageDimension>* itkImage,
    int axis,
    int slice,
    typename itk::Image<TPixel, VImageDimension>::RegionType &outputRegion
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::SizeType SizeType;
  typedef typename ImageType::RegionType RegionType;

  RegionType region = itkImage->GetLargestPossibleRegion();
  SizeType regionSize = region.GetSize();
  IndexType regionIndex = region.GetIndex();

  regionSize[axis] = 1;
  regionIndex[axis] = slice;

  outputRegion.SetSize(regionSize);
  outputRegion.SetIndex(regionIndex);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKCalculateSliceRegionAsVector(
    itk::Image<TPixel, VImageDimension>* itkImage,
    int axis,
    int slice,
    std::vector<int>& outputRegion
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;
  typedef typename ImageType::SizeType SizeType;
  typedef typename ImageType::IndexType IndexType;

  RegionType region;
  Self::ITKCalculateSliceRegion(itkImage, axis, slice, region);

  SizeType regionSize = region.GetSize();
  IndexType regionIndex = region.GetIndex();

  outputRegion.clear();
  outputRegion.push_back(regionIndex[0]);
  outputRegion.push_back(regionIndex[1]);
  outputRegion.push_back(regionIndex[2]);
  outputRegion.push_back(regionSize[0]);
  outputRegion.push_back(regionSize[1]);
  outputRegion.push_back(regionSize[2]);
}

//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKClearSlice(itk::Image<TPixel, VImageDimension>* itkImage,
    int axis,
    int slice
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;

  RegionType sliceRegion;
  TPixel pixelValue = 0;

  Self::ITKCalculateSliceRegion(itkImage, axis, slice, sliceRegion);
  Self::ITKFillRegion(itkImage, sliceRegion, pixelValue);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void MIDASGeneralSegmentorView
::ITKFilterSeedsToCurrentSlice(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet &inputSeeds,
    int axis,
    int slice,
    mitk::PointSet &outputSeeds
    )
{
  outputSeeds.Clear();

  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::PointType PointType;

  mitk::PointSet::PointsConstIterator inputSeedsIt = inputSeeds.Begin();
  mitk::PointSet::PointsConstIterator inputSeedsEnd = inputSeeds.End();
  for ( ; inputSeedsIt != inputSeedsEnd; ++inputSeedsIt)
  {
    mitk::PointSet::PointType inputSeed = inputSeedsIt->Value();
    mitk::PointSet::PointIdentifier inputSeedID = inputSeedsIt->Index();
    IndexType inputSeedIndex;
    itkImage->TransformPhysicalPointToIndex(inputSeed, inputSeedIndex);

    if (inputSeedIndex[axis] == slice)
    {
      outputSeeds.InsertPoint(inputSeedID, inputSeed);
    }
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKRecalculateMinAndMaxOfSeedValues(
    itk::Image<TPixel, VImageDimension>* itkImage,
    mitk::PointSet &inputSeeds,
    int axis,
    int slice,
    double &min,
    double &max
    )
{
  if (inputSeeds.GetSize() == 0)
  {
    min = 0;
    max = 0;
  }
  else
  {
    typedef itk::Image<TPixel, VImageDimension> ImageType;
    typedef typename ImageType::PointType PointType;
    typedef typename ImageType::IndexType IndexType;

    mitk::PointSet::Pointer filteredSeeds = mitk::PointSet::New();
    Self::ITKFilterSeedsToCurrentSlice(itkImage, inputSeeds, axis, slice, *(filteredSeeds.GetPointer()));

    if (filteredSeeds->GetSize() == 0)
    {
      min = 0;
      max = 0;
    }
    else
    {
      min = std::numeric_limits<double>::max();
      max = std::numeric_limits<double>::min();

      // Iterate through each point, get voxel value, keep running total of min/max.
      mitk::PointSet::PointsConstIterator filteredSeedsIt = filteredSeeds->Begin();
      mitk::PointSet::PointsConstIterator filteredSeedsEnd = filteredSeeds->End();
      for ( ; filteredSeedsIt != filteredSeedsEnd; ++filteredSeedsIt)
      {
        mitk::PointSet::PointType point = filteredSeedsIt->Value();

        PointType millimetreCoordinate;
        IndexType voxelCoordinate;

        millimetreCoordinate[0] = point[0];
        millimetreCoordinate[1] = point[1];
        millimetreCoordinate[2] = point[2];

        if (itkImage->TransformPhysicalPointToIndex(millimetreCoordinate, voxelCoordinate))
        {
          TPixel voxelValue = itkImage->GetPixel(voxelCoordinate);
          if (voxelValue < min)
          {
            min = voxelValue;
          }
          if (voxelValue > max)
          {
            max = voxelValue;
          }
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKFilterInputPointSetToExcludeRegionOfInterest(
    itk::Image<TPixel, VImageDimension> *itkImage,
    typename itk::Image<TPixel, VImageDimension>::RegionType regionOfInterest,
    mitk::PointSet &inputSeeds,
    mitk::PointSet &outputCopyOfInputSeeds,
    mitk::PointSet &outputNewSeedsNotInRegionOfInterest
    )
{
  // Copy inputSeeds to outputCopyOfInputSeeds seeds, so that they can be passed on to
  // Redo/Undo framework for Undo purposes. Additionally, copy any input seed that is not
  // within the regionOfInterest. Seed locations are all in millimetres.

  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::PointType PointType;

  mitk::PointSet::PointsConstIterator inputSeedsIt = inputSeeds.Begin();
  mitk::PointSet::PointsConstIterator inputSeedsEnd = inputSeeds.End();
  for ( ; inputSeedsIt != inputSeedsEnd; ++inputSeedsIt)
  {
    mitk::PointSet::PointType inputPoint = inputSeedsIt->Value();
    mitk::PointSet::PointIdentifier inputPointID = inputSeedsIt->Index();

    // Copy every point to outputCopyOfInputSeeds.
    outputCopyOfInputSeeds.InsertPoint(inputPointID, inputPoint);

    // Only copy points outside of ROI.
    PointType voxelIndexInMillimetres = inputPoint;
    IndexType voxelIndex;
    itkImage->TransformPhysicalPointToIndex(voxelIndexInMillimetres, voxelIndex);

    if (!regionOfInterest.IsInside(voxelIndex))
    {
      outputNewSeedsNotInRegionOfInterest.InsertPoint(inputPointID, inputPoint);
    }
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
bool MIDASGeneralSegmentorView
::ITKSliceDoesHaveSeeds(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet* seeds,
    int axis,
    int slice
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::PointType PointType;

  bool hasSeeds = false;
  mitk::PointSet::PointsConstIterator seedsIt = seeds->Begin();
  mitk::PointSet::PointsConstIterator seedsEnd = seeds->End();
  for ( ; seedsIt != seedsEnd; ++seedsIt)
  {
    PointType voxelIndexInMillimetres = seedsIt->Value();
    IndexType voxelIndex;
    itkImage->TransformPhysicalPointToIndex(voxelIndexInMillimetres, voxelIndex);

    if (voxelIndex[axis] ==  slice)
    {
      hasSeeds = true;
      break;
    }
  }

  return hasSeeds;
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
bool
MIDASGeneralSegmentorView
::ITKSliceIsEmpty(
    itk::Image<TPixel, VImageDimension> *itkImage,
    int axis,
    int slice,
    bool &outputSliceIsEmpty
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;

  RegionType region;
  Self::ITKCalculateSliceRegion(itkImage, axis, slice, region);

  outputSliceIsEmpty = true;

  itk::ImageRegionConstIterator<ImageType> iterator(itkImage, region);
  for (iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
  {
    if (iterator.Get() != 0)
    {
      outputSliceIsEmpty = false;
      break;
    }
  }

  return outputSliceIsEmpty;
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKUpdateRegionGrowing(
    itk::Image<TPixel, VImageDimension>* itkImage,  // Grey scale image (read only).
    bool skipUpdate,
    mitk::Image &workingImage,
    mitk::PointSet &seeds,
    mitk::ContourModelSet &segmentationContours,
    mitk::ContourModelSet &drawContours,
    mitk::ContourModelSet &polyContours,
    int sliceNumber,
    int axisNumber,
    double lowerThreshold,
    double upperThreshold,
    mitk::DataNode::Pointer &outputRegionGrowingNode,
    mitk::Image::Pointer &outputRegionGrowingImage
    )
{
  typedef itk::Image<unsigned char, VImageDimension> ImageType;
  typedef mitk::ImageToItk< ImageType > ImageToItkType;

  typename ImageToItkType::Pointer regionGrowingToItk = ImageToItkType::New();
  regionGrowingToItk->SetInput(outputRegionGrowingImage);
  regionGrowingToItk->Update();

  typename ImageToItkType::Pointer workingImageToItk = ImageToItkType::New();
  workingImageToItk->SetInput(&workingImage);
  workingImageToItk->Update();

  std::stringstream key;
  key << typeid(TPixel).name() << VImageDimension;

  GeneralSegmentorPipeline<TPixel, VImageDimension>* pipeline = NULL;
  GeneralSegmentorPipelineInterface* myPipeline = NULL;

  std::map<std::string, GeneralSegmentorPipelineInterface*>::iterator iter;
  iter = m_TypeToPipelineMap.find(key.str());

  if (iter == m_TypeToPipelineMap.end())
  {
    pipeline = new GeneralSegmentorPipeline<TPixel, VImageDimension>();
    myPipeline = pipeline;
    m_TypeToPipelineMap.insert(StringAndPipelineInterfacePair(key.str(), myPipeline));
  }
  else
  {
    myPipeline = iter->second;
    pipeline = static_cast<GeneralSegmentorPipeline<TPixel, VImageDimension>*>(myPipeline);
  }

  GeneralSegmentorPipelineParams params;
  params.m_SliceNumber = sliceNumber;
  params.m_AxisNumber = axisNumber;
  params.m_LowerThreshold = lowerThreshold;
  params.m_UpperThreshold = upperThreshold;
  params.m_Seeds = &seeds;
  params.m_SegmentationContours = &segmentationContours;
  params.m_DrawContours = &drawContours;
  params.m_PolyContours = &polyContours;
  params.m_EraseFullSlice = true;

  // Update pipeline.
  if (!skipUpdate)
  {
    // First wipe whole 3D volume
    regionGrowingToItk->GetOutput()->FillBuffer(0);

    // Configure pipeline.
    pipeline->SetParam(itkImage, workingImageToItk->GetOutput(), params);

    // Setting the pointer to the output image, then calling update on the pipeline
    // will mean that the pipeline will copy its data to the output image.
    pipeline->m_OutputImage = regionGrowingToItk->GetOutput();
    pipeline->Update(params);

    //mitk::Image::Pointer segmentationContourImage = mitk::ImportItkImage(pipeline->m_SegmentationContourImage);
    //mitk::Image::Pointer manualContourImage = mitk::ImportItkImage(pipeline->m_ManualContourImage);

    //mitk::DataNode::Pointer segmentationContourImageNode = this->CreateNewSegmentation(m_DefaultSegmentationColor);
    //segmentationContourImageNode->SetData(segmentationContourImage);
    //mitk::DataNode::Pointer manualContourImageNode = this->CreateNewSegmentation(m_DefaultSegmentationColor);
    //manualContourImageNode->SetData(manualContourImage);

    // To make sure we release all smart pointers.
    pipeline->DisconnectPipeline();
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKPropagateToRegionGrowingImage
 (itk::Image<TPixel, VImageDimension>* itkImage,
  mitk::PointSet& inputSeeds,
  int sliceNumber,
  int axisNumber,
  int direction,
  double lowerThreshold,
  double upperThreshold,
  mitk::PointSet &outputCopyOfInputSeeds,
  mitk::PointSet &outputNewSeeds,
  std::vector<int> &outputRegion,
  mitk::DataNode::Pointer &outputRegionGrowingNode,
  mitk::Image::Pointer &outputRegionGrowingImage
 )
{
  typedef typename itk::Image<TPixel, VImageDimension> GreyScaleImageType;
  typedef typename itk::Image<unsigned char, VImageDimension> BinaryImageType;

  // First take a copy of input seeds, as we need to store them for Undo/Redo purposes.
  mitk::CopyPointSets(inputSeeds, outputCopyOfInputSeeds);

  // Work out the output region of interest that will be affected.
  // We want the region upstream/downstream/both of the slice of interest
  // which also includes the slice of interest.

  typename GreyScaleImageType::RegionType outputITKRegion = itkImage->GetLargestPossibleRegion();
  typename GreyScaleImageType::SizeType outputRegionSize = outputITKRegion.GetSize();
  typename GreyScaleImageType::IndexType outputRegionIndex = outputITKRegion.GetIndex();

  if (direction == 1)
  {
    outputRegionSize[axisNumber] = outputRegionSize[axisNumber] - sliceNumber;
    outputRegionIndex[axisNumber] = sliceNumber;
  }
  else if (direction == -1)
  {
    outputRegionSize[axisNumber] = sliceNumber + 1;
    outputRegionIndex[axisNumber] = 0;
  }
  outputITKRegion.SetSize(outputRegionSize);
  outputITKRegion.SetIndex(outputRegionIndex);

  outputRegion.push_back(outputRegionIndex[0]);
  outputRegion.push_back(outputRegionIndex[1]);
  outputRegion.push_back(outputRegionIndex[2]);
  outputRegion.push_back(outputRegionSize[0]);
  outputRegion.push_back(outputRegionSize[1]);
  outputRegion.push_back(outputRegionSize[2]);

  mitk::PointSet::Pointer temporaryPointSet = mitk::PointSet::New();
  Self::ITKFilterSeedsToCurrentSlice(itkImage, inputSeeds, axisNumber, sliceNumber, *(temporaryPointSet.GetPointer()));

  if (direction == 1 || direction == -1)
  {
    Self::ITKPropagateUpOrDown(itkImage, *(temporaryPointSet.GetPointer()), sliceNumber, axisNumber, direction, lowerThreshold, upperThreshold, outputRegionGrowingNode, outputRegionGrowingImage);
  }
  else if (direction == 0)
  {
    Self::ITKPropagateUpOrDown(itkImage, *(temporaryPointSet.GetPointer()), sliceNumber, axisNumber, 1, lowerThreshold, upperThreshold, outputRegionGrowingNode, outputRegionGrowingImage);
    Self::ITKPropagateUpOrDown(itkImage, *(temporaryPointSet.GetPointer()), sliceNumber, axisNumber, -1, lowerThreshold, upperThreshold, outputRegionGrowingNode, outputRegionGrowingImage);
  }

  // Get hold of ITK version of MITK image.

  typedef mitk::ImageToItk< BinaryImageType > ImageToItkType;
  typename ImageToItkType::Pointer outputToItk = ImageToItkType::New();
  outputToItk->SetInput(outputRegionGrowingImage);
  outputToItk->UpdateLargestPossibleRegion();

  // For each slice in the region growing output, calculate new seeds on a per slice basis.
  Self::ITKAddNewSeedsToPointSet(
      outputToItk->GetOutput(),
      outputITKRegion,
      sliceNumber,
      axisNumber,
      outputNewSeeds
      );
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKPropagateUpOrDown(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet &seeds,
    int sliceNumber,
    int axis,
    int direction,
    double lowerThreshold,
    double upperThreshold,
    mitk::DataNode::Pointer &outputRegionGrowingNode,
    mitk::Image::Pointer &outputRegionGrowingImage
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> GreyScaleImageType;
  typedef typename itk::Image<unsigned char, VImageDimension> BinaryImageType;
  typedef typename itk::MIDASRegionGrowingImageFilter<GreyScaleImageType, BinaryImageType, PointSetType> RegionGrowingFilterType;

  // Convert MITK seeds to ITK seeds.
  PointSetPointer itkSeeds = PointSetType::New();
  ConvertMITKSeedsAndAppendToITKSeeds(&seeds, itkSeeds);

  // This mask is used to control the propagation in the region growing filter.
  typename GreyScaleImageType::IndexType propagationMask;
  propagationMask.Fill(0);
  propagationMask[axis] = direction;

  // Calculate the appropriate region
  typename GreyScaleImageType::RegionType region = itkImage->GetLargestPossibleRegion();
  typename GreyScaleImageType::SizeType regionSize = region.GetSize();
  typename GreyScaleImageType::IndexType regionIndex = region.GetIndex();

  if (direction == 1)
  {
    regionSize[axis] = regionSize[axis] - sliceNumber;
    regionIndex[axis] = sliceNumber;
  }
  else if (direction == -1)
  {
    regionSize[axis] = sliceNumber + 1;
    regionIndex[axis] = 0;
  }
  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  // Perform 3D region growing.
  typename RegionGrowingFilterType::Pointer regionGrowingFilter = RegionGrowingFilterType::New();
  regionGrowingFilter->SetInput(itkImage);
  regionGrowingFilter->SetRegionOfInterest(region);
  regionGrowingFilter->SetUseRegionOfInterest(true);
  regionGrowingFilter->SetPropMask(propagationMask);
  regionGrowingFilter->SetUsePropMaskMode(true);
  regionGrowingFilter->SetProjectSeedsIntoRegion(false);
  regionGrowingFilter->SetEraseFullSlice(false);
  regionGrowingFilter->SetForegroundValue(1);
  regionGrowingFilter->SetBackgroundValue(0);
  regionGrowingFilter->SetSegmentationContourImageInsideValue(0);
  regionGrowingFilter->SetSegmentationContourImageBorderValue(1);
  regionGrowingFilter->SetSegmentationContourImageOutsideValue(2);
  regionGrowingFilter->SetManualContourImageNonBorderValue(0);
  regionGrowingFilter->SetManualContourImageBorderValue(1);
  regionGrowingFilter->SetLowerThreshold(static_cast<TPixel>(lowerThreshold));
  regionGrowingFilter->SetUpperThreshold(static_cast<TPixel>(upperThreshold));
  regionGrowingFilter->SetSeedPoints(*(itkSeeds.GetPointer()));
  regionGrowingFilter->Update();

  // Aim: Make sure all smart pointers to the input reference (grey scale T1 image) are released.
  regionGrowingFilter->SetInput(NULL);

  // Write output of region growing filter directly back to the supplied region growing image

  typedef mitk::ImageToItk< BinaryImageType > ImageToItkType;
  typename ImageToItkType::Pointer outputToItk = ImageToItkType::New();
  outputToItk->SetInput(outputRegionGrowingImage);
  outputToItk->UpdateLargestPossibleRegion();

  typename itk::ImageRegionIterator< BinaryImageType > outputIter(outputToItk->GetOutput(), region);
  typename itk::ImageRegionConstIterator< BinaryImageType > regionIter(regionGrowingFilter->GetOutput(), region);

  for (outputIter.GoToBegin(), regionIter.GoToBegin(); !outputIter.IsAtEnd(); ++outputIter, ++regionIter)
  {
    outputIter.Set(regionIter.Get());
  }
}


//-----------------------------------------------------------------------------
template <typename TGreyScalePixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKPropagateToSegmentationImage(
    itk::Image<TGreyScalePixel, VImageDimension>* referenceGreyScaleImage,
    mitk::Image* segmentedImage,
    mitk::Image* regionGrowingImage,
    mitk::OpPropagate *op)
{
  typedef typename itk::Image<TGreyScalePixel, VImageDimension> GreyScaleImageType;
  typedef typename itk::Image<unsigned char, VImageDimension> BinaryImageType;

  typedef mitk::ImageToItk< BinaryImageType > ImageToItkType;
  typename ImageToItkType::Pointer segmentedImageToItk = ImageToItkType::New();
  segmentedImageToItk->SetInput(segmentedImage);
  segmentedImageToItk->Update();

  typename ImageToItkType::Pointer regionGrowingImageToItk = ImageToItkType::New();
  regionGrowingImageToItk->SetInput(regionGrowingImage);
  regionGrowingImageToItk->Update();

  mitk::OpPropagate::ProcessorPointer processor = op->GetProcessor();
  std::vector<int> region = op->GetRegion();
  bool redo = op->IsRedo();

  processor->SetSourceImage(regionGrowingImageToItk->GetOutput());
  processor->SetDestinationImage(segmentedImageToItk->GetOutput());
  processor->SetSourceRegionOfInterest(region);
  processor->SetDestinationRegionOfInterest(region);

  if (redo)
  {
    processor->Redo();
  }
  else
  {
    processor->Undo();
  }

  processor->SetSourceImage(NULL);
  processor->SetDestinationImage(NULL);

  // Clear the region growing image, as this was only used for temporary space.
  typename BinaryImageType::RegionType regionOfInterest = processor->GetSourceRegionOfInterest();
  typename itk::ImageRegionIterator<BinaryImageType> iter(regionGrowingImageToItk->GetOutput(), regionOfInterest);
  for (iter.GoToBegin(); !iter.IsAtEnd(); ++iter)
  {
    iter.Set(0);
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKGenerateOutlineFromBinaryImage(
    itk::Image<TPixel, VImageDimension>* itkImage,
    int axisNumber,
    int sliceNumber,
    int projectedSliceNumber,
    mitk::ContourModelSet::Pointer outputContourSet
    )
{
  // NOTE: This function is only meant to be called on binary images,
  // so we are assuming that TPixel is only ever unsigned char.

  outputContourSet->Clear();

  // Get the largest possible region of the input 3D image.
  Region3DType region = itkImage->GetLargestPossibleRegion();
  Size3DType regionSize = region.GetSize();
  Index3DType regionIndex = region.GetIndex();
  Index3DType projectedRegionIndex = region.GetIndex();

  // Collapse this 3D region down to 2D. So along the specified axis, the size=0.
  regionSize[axisNumber] = 0;
  regionIndex[axisNumber] = sliceNumber;
  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  // Also, we setup an index for the "Projected" slice.
  // Here, the terminology "Projected" means which slice we are projecting the contour on to.
  // So, the input sliceNumber controls which slice of data we actually extract, but the "Projected"
  // slice determines the output coordinates of the contours. The contours are "projected" onto that slice.

  projectedRegionIndex[axisNumber] = projectedSliceNumber;

  // To convert 2D voxel coordinates, to 3D coordinates, we need to map the
  // X and Y axes of the 2D image into a 3D vector in the original 3D space.
  Index3DType axes[2];

  // From this point forward, in this method, by X axis we mean, the first axis that
  // is not the through plane direction in the 2D slice. Similarly for Y, the second axis.
  axes[0] = regionIndex;
  axes[1] = regionIndex;
  int axisCounter = 0;
  for (int i = 0; i < 3; i++)
  {
    if (i != axisNumber)
    {
      axes[axisCounter][i] += 1;
      axisCounter++;
    }
  }

  // Calculate the 3D origin of the extracted slice and the projected slice,
  // and hence an offset that must be applied to each coordinate to project it.
  Point3DType originOfSlice;
  itkImage->TransformIndexToPhysicalPoint(regionIndex, originOfSlice);

  Point3DType originOfProjectedSlice;
  Point3DType offsetToProject;
  Point3DType axesInMm[2];

  itkImage->TransformIndexToPhysicalPoint(projectedRegionIndex, originOfProjectedSlice);
  itkImage->TransformIndexToPhysicalPoint(axes[0], axesInMm[0]);
  itkImage->TransformIndexToPhysicalPoint(axes[1], axesInMm[1]);

  for (int i = 0; i < 3; i++)
  {
    axesInMm[0][i] -= originOfSlice[i];
    axesInMm[1][i] -= originOfSlice[i];
    offsetToProject[i] = originOfProjectedSlice[i] - originOfSlice[i];
  }

  // Extract 2D slice, and the contours, using ITK pipelines.
  typename ExtractSliceFilterType::Pointer extractSliceFilter = ExtractSliceFilterType::New();
  extractSliceFilter->SetDirectionCollapseToIdentity();
  extractSliceFilter->SetInput(itkImage);
  extractSliceFilter->SetExtractionRegion(region);

  typename ExtractContoursFilterType::Pointer extractContoursFilter = ExtractContoursFilterType::New();
  extractContoursFilter->SetInput(extractSliceFilter->GetOutput());
  extractContoursFilter->SetContourValue(0.5);
  extractContoursFilter->Update();

  // Aim: Make sure all smart pointers to the input reference (grey scale T1 image) are released.
  extractSliceFilter->SetInput(NULL);
  extractContoursFilter->SetInput(NULL);

  // Now extract the contours, and convert to millimetre coordinates.
  unsigned int numberOfContours = extractContoursFilter->GetNumberOfOutputs();
  for (unsigned int i = 0; i < numberOfContours; i++)
  {
    mitk::ContourModel::Pointer contour = mitk::ContourModel::New();
    contour->SetClosed(false);

    typename PathType::Pointer path = extractContoursFilter->GetOutput(i);
    const typename PathType::VertexListType* list = path->GetVertexList();

    mitk::Point3D pointInMm;
    for (unsigned long int j = 0; j < list->Size(); j++)
    {
      typename PathType::VertexType vertex = list->ElementAt(j);

      /// We keep only the corner points. If one of the coordinates is a round number, we skip it.
      /// See the comment in MIDASGeneralSegmentorViewHelper.cxx in the ConvertMITKContoursAndAppendToITKContours
      /// function.
      if ((vertex[0] == std::floor(vertex[0])) || (vertex[1] == std::floor(vertex[1])))
      {
        continue;
      }

      pointInMm[0] = originOfSlice[0] + (vertex[0] * axesInMm[0][0]) + (vertex[1] * axesInMm[1][0]) + offsetToProject[0];
      pointInMm[1] = originOfSlice[1] + (vertex[0] * axesInMm[0][1]) + (vertex[1] * axesInMm[1][1]) + offsetToProject[1];
      pointInMm[2] = originOfSlice[2] + (vertex[0] * axesInMm[0][2]) + (vertex[1] * axesInMm[1][2]) + offsetToProject[2];

      contour->AddVertex(pointInMm);
    }

    // Note that the original contour has to be closed, i.e. its start and end point must be the same.
    // We can assume that the start point is always on the side of a pixel, i.e. not a corner point.
    // Since we removed the pixel-side points, the contour is not closed any more. Therefore,
    // we have to connect the last corner point to the first one.
    pointInMm = contour->GetVertexAt(0)->Coordinates;
    contour->AddVertex(pointInMm);

    outputContourSet->AddContourModel(contour);
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void MIDASGeneralSegmentorView
::ITKGetLargestMinimumDistanceSeedLocation(
  itk::Image<TPixel, VImageDimension>* itkImage,
  TPixel& foregroundPixelValue,
  typename itk::Image<TPixel, VImageDimension>::IndexType &outputSeedIndex,
  int &outputDistance)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::PixelType       PixelType;
  typedef typename ImageType::IndexType       IndexType;
  typedef typename ImageType::SizeType        SizeType;
  typedef typename ImageType::RegionType      RegionType;

  // For the given input image, will return the voxel location that has the
  // largest minimum distance (in x,y direction only) from the edge.
  // For each non-background pixel, we find the minimum distance to the edge for each of the
  // x,y axes in both directions. i.e. we iterate along +x, -x, +y, -y, and find the minimum
  // distance to the edge, and we do this for each non-background voxel, and return the voxel
  // with the largest minimum distance. The input is assumed to be binary ... or more specifically,
  // zero=background and anything else=foreground.

  // In MIDAS terms, this is only called on 2D images, so efficiency is not a problem.
  int workingDistance = -1;
  int minimumDistance = -1;
  int bestDistance = -1;
  IndexType bestIndex;
  bestIndex.Fill(0);
  IndexType workingIndex;
  IndexType currentIndex;
  PixelType currentPixel = 0;
  RegionType imageRegion = itkImage->GetLargestPossibleRegion();
  SizeType imageSize = imageRegion.GetSize();

  // Work out the largest number of steps we will need along each axis.
  int distanceLimitInVoxels = imageSize[0];
  for (unsigned int i = 1; i < IndexType::GetIndexDimension(); i++)
  {
    distanceLimitInVoxels = std::max((int)distanceLimitInVoxels, (int)imageSize[i]);
  }

  // Iterate through each pixel in image.
  itk::ImageRegionConstIteratorWithIndex<ImageType> imageIterator(itkImage, imageRegion);
  for (imageIterator.GoToBegin(); !imageIterator.IsAtEnd(); ++imageIterator)
  {
    // Check that the current pixel is not background.
    currentPixel = imageIterator.Get();
    if (currentPixel == foregroundPixelValue)
    {
      currentIndex = imageIterator.GetIndex();
      minimumDistance = distanceLimitInVoxels;

      // If this is the first non-zero voxel, assume this is the best so far.
      if (bestDistance == -1)
      {
        bestDistance = 0;
        bestIndex = currentIndex;
      }

      // and for each of the image axes.
      for (unsigned int i = 0; i < IndexType::GetIndexDimension(); i++)
      {
        // Only iterate over the x,y,z, axis if the size of the axis is > 1
        if (imageSize[i] > 1)
        {
          // For each direction +/-
          for (int j = -1; j <= 1; j+=2)
          {
            // Reset the workingIndex to the current position.
            workingIndex = currentIndex;
            workingDistance = 0;
            do
            {
              // Calculate an offset.
              workingDistance++;
              workingIndex[i] = currentIndex[i] + j*workingDistance;

            } // And check we are still in the image on non-background.
            while (workingDistance < minimumDistance
                   && imageRegion.IsInside(workingIndex)
                   && itkImage->GetPixel(workingIndex) == foregroundPixelValue
                   );

            minimumDistance = workingDistance;

            if (minimumDistance < bestDistance)
            {
              break;
            }
          } // end for j
        } // end if image size > 1.
      } // end for i

      // If this voxel has a larger minimum distance, than the bestDistance so far, we chose this one.
      if (minimumDistance > bestDistance)
      {
        bestIndex = currentIndex;
        bestDistance = minimumDistance;
      }
    }
  }
  // Output the largest minimumDistance and the corresponding voxel location.
  outputSeedIndex = bestIndex;
  outputDistance = bestDistance;
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKAddNewSeedsToPointSet(
    itk::Image<TPixel, VImageDimension> *itkImage,
    typename itk::Image<TPixel, VImageDimension>::RegionType region,
    int sliceNumber,
    int axisNumber,
    mitk::PointSet &outputNewSeeds
    )
{
  // Note, although templated over TPixel, input should only ever be unsigned char binary images.
  typedef typename itk::Image<TPixel, VImageDimension>        BinaryImageType;
  typedef typename BinaryImageType::PointType                 BinaryPointType;
  typedef typename BinaryImageType::IndexType                 BinaryIndexType;
  typedef typename itk::Image<unsigned int, VImageDimension>  IntegerImageType;
  typedef typename itk::ExtractImageFilter<BinaryImageType, BinaryImageType> ExtractImageFilterType;
  typedef typename itk::ConnectedComponentImageFilter<BinaryImageType, IntegerImageType> ConnectedComponentFilterType;

  // Some working data.
  typename IntegerImageType::PixelType voxelValue = 0;
  BinaryIndexType voxelIndex;
  BinaryPointType voxelIndexInMillimetres;
  Size3DType regionSize = region.GetSize();
  Index3DType regionIndex = region.GetIndex();

  // We are going to repeatedly extract each slice, and calculate new seeds on a per slice basis.
  typename ExtractImageFilterType::Pointer extractSliceFilter = ExtractImageFilterType::New();
  extractSliceFilter->SetDirectionCollapseToIdentity();
  extractSliceFilter->SetInput(itkImage);

  typename ConnectedComponentFilterType::Pointer connectedComponentsFilter = ConnectedComponentFilterType::New();
  connectedComponentsFilter->SetInput(extractSliceFilter->GetOutput());
  connectedComponentsFilter->SetBackgroundValue(0);
  connectedComponentsFilter->SetFullyConnected(false);

  typename BinaryImageType::RegionType perSliceRegion;
  typename BinaryImageType::SizeType   perSliceRegionSize;
  typename BinaryImageType::IndexType  perSliceRegionStartIndex;

  perSliceRegionSize = regionSize;
  perSliceRegionStartIndex = regionIndex;
  perSliceRegionSize[axisNumber] = 1;
  perSliceRegion.SetSize(perSliceRegionSize);

  for (unsigned int i = 0; i < regionSize[axisNumber]; i++)
  {
    perSliceRegionStartIndex[axisNumber] = regionIndex[axisNumber] + i;
    perSliceRegion.SetIndex(perSliceRegionStartIndex);

    // Extract slice, and get connected components.
    extractSliceFilter->SetExtractionRegion(perSliceRegion);
    connectedComponentsFilter->UpdateLargestPossibleRegion();

    // For each distinct region, on each 2D slice, we calculate a new seed.
    typename IntegerImageType::Pointer ccImage = connectedComponentsFilter->GetOutput();
    typename itk::ImageRegionConstIteratorWithIndex<IntegerImageType> ccImageIterator(ccImage, ccImage->GetLargestPossibleRegion());
    std::set<typename IntegerImageType::PixelType> setOfLabels;

    int notUsed;
    mitk::PointSet::PointType point;
    int numberOfPoints = outputNewSeeds.GetSize();

    for (ccImageIterator.GoToBegin(); !ccImageIterator.IsAtEnd(); ++ccImageIterator)
    {
      voxelValue = ccImageIterator.Get();

      if (voxelValue != 0 && setOfLabels.find(voxelValue) == setOfLabels.end())
      {
        setOfLabels.insert(voxelValue);

        // Work out the best seed position.
        Self::ITKGetLargestMinimumDistanceSeedLocation<typename IntegerImageType::PixelType, VImageDimension>(connectedComponentsFilter->GetOutput(), voxelValue, voxelIndex, notUsed);

        // And convert that seed position to a 3D point.
        itkImage->TransformIndexToPhysicalPoint(voxelIndex, point);
        outputNewSeeds.InsertPoint(numberOfPoints, point);
        numberOfPoints++;
      } // end if new label
    } // end for each label
  } // end for each slice

  // Aim: Make sure all smart pointers to the input reference (grey scale T1 image) are released.
  extractSliceFilter->SetInput(NULL);
  connectedComponentsFilter->SetInput(NULL);
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKPreProcessingOfSeedsForChangingSlice(
    itk::Image<TPixel, VImageDimension> *itkImage, // Note: the itkImage input should be the binary region growing image.
    mitk::PointSet &inputSeeds,
    int sliceNumber,
    int axisNumber,
    int newSliceNumber,
    bool optimiseSeedPosition,
    bool newSliceIsEmpty,
    mitk::PointSet &outputCopyOfInputSeeds,
    mitk::PointSet &outputNewSeeds,
    std::vector<int> &outputRegion
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> BinaryImageType;

  // Work out the region of the current slice.

  typename BinaryImageType::RegionType region = itkImage->GetLargestPossibleRegion();
  typename BinaryImageType::SizeType regionSize = region.GetSize();
  typename BinaryImageType::IndexType regionIndex = region.GetIndex();

  regionSize[axisNumber] = 1;
  regionIndex[axisNumber] = sliceNumber;

  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  outputRegion.push_back(regionIndex[0]);
  outputRegion.push_back(regionIndex[1]);
  outputRegion.push_back(regionIndex[2]);
  outputRegion.push_back(regionSize[0]);
  outputRegion.push_back(regionSize[1]);
  outputRegion.push_back(regionSize[2]);

  // If we are moving to new slice
  if (sliceNumber != newSliceNumber)
  {
    if (newSliceIsEmpty)
    {
      // Copy all input seeds, as we are moving to an empty slice.
      mitk::CopyPointSets(inputSeeds, outputCopyOfInputSeeds);

      // Take all seeds on the current slice number, and propagate to new slice.
      Self::ITKPropagateSeedsToNewSlice(
          itkImage,
          &inputSeeds,
          &outputNewSeeds,
          axisNumber,
          sliceNumber,
          newSliceNumber
          );
    }
    else // new slice is not empty.
    {
      if (optimiseSeedPosition) // if this is false, we do nothing - i.e. leave existing seeds AS IS.
      {
        regionSize = region.GetSize();
        regionIndex = region.GetIndex();

        regionSize[axisNumber] = 1;
        regionIndex[axisNumber] = newSliceNumber;

        region.SetSize(regionSize);
        region.SetIndex(regionIndex);

        // We copy all seeds except those on the new slice.
        Self::ITKFilterInputPointSetToExcludeRegionOfInterest(
            itkImage,
            region,
            inputSeeds,
            outputCopyOfInputSeeds,
            outputNewSeeds
            );

        // We then re-generate a new set of seeds for the new slice.
        Self::ITKAddNewSeedsToPointSet(
            itkImage,
            region,
            newSliceNumber,
            axisNumber,
            outputNewSeeds
            );

      } // end if (optimiseSeedPosition)
    } // end if (newSliceIsEmpty)
  }
  else // We are not moving slice
  {
    if (optimiseSeedPosition)
    {
      // We copy all seeds except those on the current slice.
      Self::ITKFilterInputPointSetToExcludeRegionOfInterest(
          itkImage,
          region,
          inputSeeds,
          outputCopyOfInputSeeds,
          outputNewSeeds
          );

      // Here we calculate new seeds based on the connected component analysis - i.e. 1 seed per region.
      Self::ITKAddNewSeedsToPointSet(
          itkImage,
          region,
          sliceNumber,
          axisNumber,
          outputNewSeeds
          );
    }
  } // end if (sliceNumber != newSliceNumber)

  if (outputCopyOfInputSeeds.GetSize() == 0)
  {
    mitk::CopyPointSets(inputSeeds, outputCopyOfInputSeeds);
  }

  if (outputNewSeeds.GetSize() == 0)
  {
    mitk::CopyPointSets(inputSeeds, outputNewSeeds);
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKPreProcessingForWipe(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet &inputSeeds,
    int sliceNumber,
    int axisNumber,
    int direction,
    mitk::PointSet &outputCopyOfInputSeeds,
    mitk::PointSet &outputNewSeeds,
    std::vector<int> &outputRegion
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;

  // Work out the region of interest that will be affected.
  typename ImageType::RegionType region = itkImage->GetLargestPossibleRegion();
  typename ImageType::SizeType regionSize = region.GetSize();
  typename ImageType::IndexType regionIndex = region.GetIndex();

  if (direction == 0)
  {
    // Single slice
    regionSize[axisNumber] = 1;
    regionIndex[axisNumber] = sliceNumber;
  }
  else if (direction == 1)
  {
    // All anterior
    regionSize[axisNumber] = regionSize[axisNumber] - sliceNumber - 1;
    regionIndex[axisNumber] = sliceNumber + 1;
  }
  else if (direction == -1)
  {
    // All posterior
    regionSize[axisNumber] = sliceNumber;
    regionIndex[axisNumber] = 0;
  }
  region.SetSize(regionSize);
  region.SetIndex(regionIndex);

  outputRegion.push_back(regionIndex[0]);
  outputRegion.push_back(regionIndex[1]);
  outputRegion.push_back(regionIndex[2]);
  outputRegion.push_back(regionSize[0]);
  outputRegion.push_back(regionSize[1]);
  outputRegion.push_back(regionSize[2]);

  // We take a complete copy of the input seeds, and copy any seeds not in the current slice
  // as these seeds in the current slice will be overwritten in AddNewSeedsToPointSet.
  Self::ITKFilterInputPointSetToExcludeRegionOfInterest(
      itkImage,
      region,
      inputSeeds,
      outputCopyOfInputSeeds,
      outputNewSeeds
      );
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKDoWipe(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet* currentSeeds,
    mitk::OpWipe *op
    )
{
  // Assuming we are only called for the unsigned char, current segmentation image.
  typedef typename itk::Image<TPixel, VImageDimension> BinaryImageType;

  mitk::OpWipe::ProcessorPointer processor = op->GetProcessor();
  std::vector<int> region = op->GetRegion();
  bool redo = op->IsRedo();

  processor->SetWipeValue(0);
  processor->SetDestinationImage(itkImage);
  processor->SetDestinationRegionOfInterest(region);

  mitk::PointSet* outputSeeds = op->GetSeeds();

  if (redo)
  {
    processor->Redo();
  }
  else
  {
    processor->Undo();
  }

  processor->SetDestinationImage(NULL);

  int axis = op->GetAxisNumber();
  int slice = op->GetSliceNumber();

  // Update the current point set.
  currentSeeds->Clear();

  mitk::PointSet::PointsConstIterator outputSeedsIt = outputSeeds->Begin();
  mitk::PointSet::PointsConstIterator outputSeedsEnd = outputSeeds->End();
  for ( ; outputSeedsIt != outputSeedsEnd; ++outputSeedsIt)
  {
    mitk::PointSet::PointIdentifier outputSeedID = outputSeedsIt->Index();
    mitk::PointSet::PointType outputSeed = outputSeedsIt->Value();
    typename BinaryImageType::IndexType outputSeedIndex;
    itkImage->TransformPhysicalPointToIndex(outputSeed, outputSeedIndex);
    // If it's a do/redo then we do not copy back the seeds from the current slice. (Wipe them.)
    if (!redo || outputSeedIndex[axis] != slice)
    {
      currentSeeds->InsertPoint(outputSeedID, outputSeed);
    }
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
bool MIDASGeneralSegmentorView
::ITKImageHasNonZeroEdgePixels(
    itk::Image<TPixel, VImageDimension> *itkImage
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::SizeType SizeType;

  RegionType region = itkImage->GetLargestPossibleRegion();
  SizeType regionSize = region.GetSize();
  IndexType voxelIndex;

  for (unsigned int i = 0; i < IndexType::GetIndexDimension(); i++)
  {
    regionSize[i] -= 1;
  }

  itk::ImageRegionConstIteratorWithIndex<ImageType> iterator(itkImage, region);
  for (iterator.GoToBegin(); !iterator.IsAtEnd(); ++iterator)
  {
    voxelIndex = iterator.GetIndex();
    bool isEdge(false);
    for (unsigned int i = 0; i < IndexType::GetIndexDimension(); i++)
    {
      if ((int)voxelIndex[i] == 0 || (int)voxelIndex[i] == (int)regionSize[i])
      {
        isEdge = true;
      }
    }
    if (isEdge && itkImage->GetPixel(voxelIndex) > 0)
    {
      return true;
    }
  }
  return false;
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void MIDASGeneralSegmentorView
::ITKSliceDoesHaveUnEnclosedSeeds(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet &seeds,
    mitk::ContourModelSet &segmentationContours,
    mitk::ContourModelSet &polyToolContours,
    mitk::ContourModelSet &drawToolContours,
    mitk::Image &workingImage,
    double lowerThreshold,
    double upperThreshold,
    bool useThresholds,
    int axis,
    int slice,
    bool &sliceDoesHaveUnenclosedSeeds
    )
{
  sliceDoesHaveUnenclosedSeeds = false;

  // Note input image should be 3D grey scale.
  typedef itk::Image<TPixel, VImageDimension> GreyScaleImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> BinaryImageType;
  typedef mitk::ImageToItk< BinaryImageType > ImageToItkType;

  typename ImageToItkType::Pointer workingImageToItk = ImageToItkType::New();
  workingImageToItk->SetInput(&workingImage);
  workingImageToItk->Update();

  // Filter seeds to only use ones on current slice.
  mitk::PointSet::Pointer seedsForThisSlice = mitk::PointSet::New();
  Self::ITKFilterSeedsToCurrentSlice(itkImage, seeds, axis, slice, *(seedsForThisSlice.GetPointer()));

  GeneralSegmentorPipelineParams params;
  params.m_SliceNumber = slice;
  params.m_AxisNumber = axis;
  params.m_Seeds = seedsForThisSlice;
  params.m_SegmentationContours = &segmentationContours;
  params.m_PolyContours = &polyToolContours;
  params.m_DrawContours = &drawToolContours;
  params.m_EraseFullSlice = false;

  if (useThresholds)
  {
    params.m_LowerThreshold = lowerThreshold;
    params.m_UpperThreshold = upperThreshold;
  }
  else
  {
    params.m_LowerThreshold = std::numeric_limits<TPixel>::min();
    params.m_UpperThreshold = std::numeric_limits<TPixel>::max();
  }

  GeneralSegmentorPipeline<TPixel, VImageDimension> pipeline = GeneralSegmentorPipeline<TPixel, VImageDimension>();
  pipeline.m_UseOutput = false;  // don't export the output of this pipeline to an output image, as we are not providing one.
  pipeline.SetParam(itkImage, workingImageToItk->GetOutput(), params);
  pipeline.Update(params);

  // To make sure we release all smart pointers.
  pipeline.DisconnectPipeline();
  workingImageToItk = NULL;

  // Check the output, to see if we have seeds inside non-enclosing green contours.
  sliceDoesHaveUnenclosedSeeds = Self::ITKImageHasNonZeroEdgePixels<
      mitk::Tool::DefaultSegmentationDataType, VImageDimension>
      (pipeline.m_RegionGrowingFilter->GetOutput());
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void MIDASGeneralSegmentorView
::ITKFilterContours(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::Image &workingImage,
    mitk::PointSet &seeds,
    mitk::ContourModelSet &segmentationContours,
    mitk::ContourModelSet &drawToolContours,
    mitk::ContourModelSet &polyToolContours,
    int axis,
    int slice,
    double lowerThreshold,
    double upperThreshold,
    bool isThresholding,
    mitk::ContourModelSet &outputCopyOfInputContours,
    mitk::ContourModelSet &outputContours
    )
{
  typedef itk::Image<unsigned char, VImageDimension> ImageType;
  typedef mitk::ImageToItk< ImageType > ImageToItkType;

  typename ImageToItkType::Pointer workingImageToItk = ImageToItkType::New();
  workingImageToItk->SetInput(&workingImage);
  workingImageToItk->Update();

  // Input contour set could be empty, so nothing to filter.
  if (segmentationContours.GetSize() == 0)
  {
    return;
  }

  // Note input image should be 3D grey scale.
  typedef itk::Image<TPixel, VImageDimension> GreyScaleImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> BinaryImageType;
  typedef itk::ContinuousIndex<double, VImageDimension> ContinuousIndexType;
  typedef typename BinaryImageType::IndexType IndexType;
  typedef typename BinaryImageType::SizeType SizeType;
  typedef typename BinaryImageType::RegionType RegionType;
  typedef typename BinaryImageType::PointType PointType;

  mitk::MIDASContourTool::CopyContourSet(segmentationContours, outputCopyOfInputContours, true);

  GeneralSegmentorPipelineParams params;
  params.m_SliceNumber = slice;
  params.m_AxisNumber = axis;
  params.m_Seeds = &seeds;
  params.m_SegmentationContours = &segmentationContours;
  params.m_DrawContours = &drawToolContours;
  params.m_PolyContours = &polyToolContours;
  params.m_EraseFullSlice = true;

  if (isThresholding)
  {
    params.m_LowerThreshold = lowerThreshold;
    params.m_UpperThreshold = upperThreshold;
  }
  else
  {
    params.m_LowerThreshold = std::numeric_limits<TPixel>::min();
    params.m_UpperThreshold = std::numeric_limits<TPixel>::max();
  }

  GeneralSegmentorPipeline<TPixel, VImageDimension> localPipeline = GeneralSegmentorPipeline<TPixel, VImageDimension>();
  localPipeline.m_UseOutput = false;  // don't export the output of this pipeline to an output image, as we are not providing one.
  localPipeline.SetParam(itkImage, workingImageToItk->GetOutput(), params);
  localPipeline.Update(params);

  // To make sure we release all smart pointers.
  localPipeline.DisconnectPipeline();
  workingImageToItk = NULL;

  // Now calculate filtered contours, we want to get rid of any contours that are not near a region.
  // NOTE: Poly line contours (yellow) contours are not cleaned.

  unsigned int size = 0;
  mitk::Point3D pointInContour;
  PointType pointInMillimetres;
  ContinuousIndexType continuousVoxelIndex;
  IndexType voxelIndex;

  mitk::ContourModelSet::ContourModelSetIterator contourIt = outputCopyOfInputContours.Begin();
  mitk::ContourModel::Pointer firstContour = *contourIt;

  outputContours.Clear();
  mitk::ContourModel::Pointer outputContour = mitk::ContourModel::New();
  mitk::MIDASContourTool::InitialiseContour(*(firstContour.GetPointer()), *(outputContour.GetPointer()));

  RegionType neighbourhoodRegion;
  SizeType neighbourhoodSize;
  IndexType neighbourhoodIndex;
  neighbourhoodSize.Fill(2);
  neighbourhoodSize[axis] = 1;

  while ( contourIt != outputCopyOfInputContours.End() )
  {
    mitk::ContourModel::Pointer nextContour = *contourIt;

    size = nextContour->GetNumberOfVertices();
    for (unsigned int i = 0; i < size; i++)
    {
      pointInContour = nextContour->GetVertexAt(i)->Coordinates;
      for (unsigned int j = 0; j < SizeType::GetSizeDimension(); j++)
      {
        pointInMillimetres[j] = pointInContour[j];
      }

      itkImage->TransformPhysicalPointToContinuousIndex(pointInMillimetres, continuousVoxelIndex);

      for (unsigned int j = 0; j < SizeType::GetSizeDimension(); j++)
      {
        voxelIndex[j] = (int)(continuousVoxelIndex[j]);
      }
      voxelIndex[axis] = slice;
      neighbourhoodIndex = voxelIndex;
      neighbourhoodRegion.SetSize(neighbourhoodSize);
      neighbourhoodRegion.SetIndex(neighbourhoodIndex);

      bool isNearRegion = false;
      itk::ImageRegionConstIteratorWithIndex<BinaryImageType> regionGrowingIterator(localPipeline.m_RegionGrowingFilter->GetOutput(), neighbourhoodRegion);
      for (regionGrowingIterator.GoToBegin(); !regionGrowingIterator.IsAtEnd(); ++regionGrowingIterator)
      {
        if (regionGrowingIterator.Get() > 0)
        {
          isNearRegion = true;
          break;
        }
      }

      if (isNearRegion)
      {
        outputContour->AddVertex(pointInContour);
      }
      else if (!isNearRegion && outputContour->GetNumberOfVertices() >= 2)
      {
        outputContours.AddContourModel(outputContour);
        outputContour = mitk::ContourModel::New();
        mitk::MIDASContourTool::InitialiseContour(*(firstContour.GetPointer()), *(outputContour.GetPointer()));
      }
    }
    if (outputContour->GetNumberOfVertices() >= 2)
    {
      outputContours.AddContourModel(outputContour);
      outputContour = mitk::ContourModel::New();
      mitk::MIDASContourTool::InitialiseContour(*(firstContour.GetPointer()), *(outputContour.GetPointer()));
    }
    contourIt++;
  }
}



//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void MIDASGeneralSegmentorView
::ITKPropagateSeedsToNewSlice(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet* currentSeeds,
    mitk::PointSet* newSeeds,
    int axis,
    int oldSliceNumber,
    int newSliceNumber
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::IndexType IndexType;
  typedef typename ImageType::PointType PointType;

  bool newSliceHasSeeds = Self::ITKSliceDoesHaveSeeds(itkImage, currentSeeds, axis, newSliceNumber);

  newSeeds->Clear();

  mitk::PointSet::PointsConstIterator currentSeedsIt = currentSeeds->Begin();
  mitk::PointSet::PointsConstIterator currentSeedsEnd = currentSeeds->End();
  for ( ; currentSeedsIt != currentSeedsEnd; ++currentSeedsIt)
  {
    mitk::PointSet::PointType currentSeed = currentSeedsIt->Value();
    mitk::PointSet::PointIdentifier currentSeedID = currentSeedsIt->Index();

    newSeeds->InsertPoint(currentSeedID, currentSeed);

    // Don't overwrite any existing seeds on new slice.
    if (!newSliceHasSeeds)
    {
      PointType voxelIndexInMillimetres = currentSeed;
      IndexType voxelIndex;
      itkImage->TransformPhysicalPointToIndex(voxelIndexInMillimetres, voxelIndex);

      if (voxelIndex[axis] == oldSliceNumber)
      {
        IndexType newVoxelIndex = voxelIndex;
        newVoxelIndex[axis] = newSliceNumber;
        itkImage->TransformIndexToPhysicalPoint(newVoxelIndex, voxelIndexInMillimetres);

        newSeeds->InsertPoint(currentSeedID, voxelIndexInMillimetres);
      }
    }
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKDestroyPipeline(itk::Image<TPixel, VImageDimension>* itkImage)
{
  std::stringstream key;
  key << typeid(TPixel).name() << VImageDimension;

  std::map<std::string, GeneralSegmentorPipelineInterface*>::iterator iter;
  iter = m_TypeToPipelineMap.find(key.str());

  GeneralSegmentorPipeline<TPixel, VImageDimension> *pipeline = dynamic_cast<GeneralSegmentorPipeline<TPixel, VImageDimension>*>(iter->second);
  if (pipeline != NULL)
  {
    delete pipeline;
  };

  m_TypeToPipelineMap.clear();
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
MIDASGeneralSegmentorView
::ITKInitialiseSeedsForVolume(
    itk::Image<TPixel, VImageDimension> *itkImage,
    mitk::PointSet& seeds,
    int axis
    )
{
  typedef typename itk::Image<TPixel, VImageDimension> ImageType;
  typedef typename ImageType::RegionType RegionType;

  RegionType region = itkImage->GetLargestPossibleRegion();

  Self::ITKAddNewSeedsToPointSet(
      itkImage,
      region,
      0,
      axis,
      seeds
      );
}

/**************************************************************
 * End of ITK stuff.
 *************************************************************/