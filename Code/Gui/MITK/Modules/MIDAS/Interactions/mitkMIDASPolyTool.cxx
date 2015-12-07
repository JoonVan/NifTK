/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkMIDASPolyTool.h"
#include "mitkMIDASPolyToolEventInterface.h"
#include "mitkMIDASPolyToolOpAddToFeedbackContour.h"
#include "mitkMIDASPolyToolOpUpdateFeedbackContour.h"
#include "mitkMIDASPolyTool.xpm"
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>
#include <mitkPointUtils.h>
#include <mitkToolManager.h>
#include <mitkBaseRenderer.h>
#include <mitkContourModel.h>

#include <usModuleResource.h>
#include <usGetModuleContext.h>

const std::string mitk::MIDASPolyTool::MIDAS_POLY_TOOL_ANCHOR_POINTS("MIDAS PolyTool anchor points");
const std::string mitk::MIDASPolyTool::MIDAS_POLY_TOOL_PREVIOUS_CONTOUR("MIDAS PolyTool previous contour");
const mitk::OperationType mitk::MIDASPolyTool::MIDAS_POLY_TOOL_OP_ADD_TO_FEEDBACK_CONTOUR = 320420;
const mitk::OperationType mitk::MIDASPolyTool::MIDAS_POLY_TOOL_OP_UPDATE_FEEDBACK_CONTOUR = 320421;

namespace mitk{
  MITK_TOOL_MACRO(NIFTKMIDAS_EXPORT, MIDASPolyTool, "MIDAS Poly Tool");
}

mitk::MIDASPolyTool::MIDASPolyTool()
: MIDASContourTool()
, m_ReferencePoints(NULL)
, m_PreviousContourReferencePoints(NULL)
, m_PreviousContour(NULL)
, m_PreviousContourNode(NULL)
, m_PreviousContourVisible(false)
, m_PolyLinePointSet(NULL)
, m_PolyLinePointSetNode(NULL)
, m_PolyLinePointSetVisible(false)
, m_DraggedPointIndex(0)
{
  // These are not added to DataStorage as they are never drawn, they are just an internal data structure
  m_ReferencePoints = mitk::ContourModel::New();
  m_PreviousContourReferencePoints = mitk::ContourModel::New();

  // This point set is so we can highlight the first point (or potentially all points).
  m_PolyLinePointSet = mitk::PointSet::New();
  m_PolyLinePointSetNode = mitk::DataNode::New();
  m_PolyLinePointSetNode->SetData( m_PolyLinePointSet );
  m_PolyLinePointSetNode->SetProperty("name", mitk::StringProperty::New(MIDAS_POLY_TOOL_ANCHOR_POINTS) );
  m_PolyLinePointSetNode->SetProperty("visible", BoolProperty::New(true));
  m_PolyLinePointSetNode->SetProperty("helper object", BoolProperty::New(true));
  m_PolyLinePointSetNode->SetProperty("color", ColorProperty::New(1, 1, 0));

  // This is so we can draw the previous contour, as we stretch, and minipulate the contour with middle mouse button.
  m_PreviousContour = mitk::ContourModel::New();
  m_PreviousContourNode = mitk::DataNode::New();
  m_PreviousContourNode->SetData( m_PreviousContour );
  m_PreviousContourNode->SetProperty("name", StringProperty::New(MIDAS_POLY_TOOL_PREVIOUS_CONTOUR));
  m_PreviousContourNode->SetProperty("visible", BoolProperty::New(true));
  m_PreviousContourNode->SetProperty("helper object", BoolProperty::New(true));
  m_PreviousContourNode->SetProperty("contour.width", FloatProperty::New(m_ContourWidth));
  m_PreviousContourNode->SetProperty("color", ColorProperty::New(0, 1, 0));
  m_PreviousContourNode->SetProperty("contour.color", ColorProperty::New(0, 1, 0));

  this->Disable3dRenderingOfPreviousContour();

  m_Interface = MIDASPolyToolEventInterface::New();
  m_Interface->SetMIDASPolyTool(this);
}

mitk::MIDASPolyTool::~MIDASPolyTool()
{
}


//-----------------------------------------------------------------------------
void mitk::MIDASPolyTool::InitializeStateMachine()
{
  try
  {
    this->LoadStateMachine("MIDASPolyTool.xml", us::GetModuleContext()->GetModule());
    this->SetEventConfig("MIDASPolyToolConfig.xml", us::GetModuleContext()->GetModule());
  }
  catch( const std::exception& e )
  {
    MITK_ERROR << "Could not load statemachine pattern MIDASPolyTool.xml with exception: " << e.what();
  }
}


void mitk::MIDASPolyTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("addLine", AddLine);
  CONNECT_FUNCTION("selectPoint", SelectPoint);
  CONNECT_FUNCTION("movePoint", MovePoint);
  CONNECT_FUNCTION("deselectPoint", DeselectPoint);
}


const char* mitk::MIDASPolyTool::GetName() const
{
  return "Poly";
}

const char** mitk::MIDASPolyTool::GetXPM() const
{
  return mitkMIDASPolyTool_xpm;
}

void mitk::MIDASPolyTool::Disable3dRenderingOfPreviousContour()
{
  this->Disable3dRenderingOfNode(m_PreviousContourNode);
}

void mitk::MIDASPolyTool::ClearData()
{
  mitk::MIDASContourTool::ClearData();

  // These are added to the DataManager, but only drawn when the middle mouse is down (and subsequently moved).
  m_PreviousContour->Initialize();
  m_PreviousContour->SetClosed(m_ContourClosed);
  // TODO Removed at from the new MITK segmentation framework.
  // Some property of the node controls this.
//  m_PreviousContour->SetWidth(m_ContourWidth);

  // These are not added to the DataManager, so will never be drawn.
  m_ReferencePoints->Initialize();
  m_ReferencePoints->SetClosed(m_ContourClosed);
  // TODO Removed at from the new MITK segmentation framework.
  // Some property of the node controls this.
//  m_ReferencePoints->SetWidth(m_ContourWidth);

  // These are not added to the DataManager, so will never be drawn.
  m_PreviousContourReferencePoints->Initialize();
  m_PreviousContourReferencePoints->SetClosed(m_ContourClosed);
  // TODO Removed at from the new MITK segmentation framework.
  // Some property of the node controls this.
//  m_PreviousContourReferencePoints->SetWidth(m_ContourWidth);
}

void mitk::MIDASPolyTool::Activated()
{
  Superclass::Activated();

  DataNode* segmentationNode = m_ToolManager->GetWorkingData(SEGMENTATION);
  if (!segmentationNode) return;

  // Store these for later (in base class), as dynamic casts are slow. HOWEVER, IT IS NOT THREAD SAFE.
  m_SegmentationImage = dynamic_cast<Image*>(segmentationNode->GetData());
  m_SegmentationImageGeometry = m_SegmentationImage->GetGeometry();

  // If these are not set, something is fundamentally wrong.
  assert(m_SegmentationImage);
  assert(m_SegmentationImageGeometry);

  // Initialize data which sets the contours to zero length, and set properties.
  this->ClearData();

  // Turn the contours on, and set to correct colour. The FeedBack contour is yellow.
  FeedbackContourTool::SetFeedbackContourVisible(true);
  FeedbackContourTool::SetFeedbackContourColor(1, 1, 0);

  MIDASContourTool::SetBackgroundContourVisible(false);
  MIDASContourTool::SetBackgroundContourColorDefault();

  // Just to be explicit
  this->Disable3dRenderingOfPreviousContour();
  this->SetPreviousContourVisible(false);
  this->SetPolyLinePointSetVisible(false);
}

void mitk::MIDASPolyTool::Deactivated()
{
  mitk::ContourModel* feedbackContour = FeedbackContourTool::GetFeedbackContour();

  if (feedbackContour != NULL && feedbackContour->GetNumberOfVertices() > 0)
  {
    if (m_ToolManager->GetWorkingData(CONTOURS))
    {
      this->AccumulateContourInWorkingData(*feedbackContour, CONTOURS);
    }
  }

  // Initialize data which sets the contours to zero length, and set properties.
  this->ClearData();

  // Set visibility.
  FeedbackContourTool::SetFeedbackContourVisible(false);
  MIDASContourTool::SetBackgroundContourVisible(false);
  this->SetPreviousContourVisible(false);
  this->SetPolyLinePointSetVisible(false);
  this->RenderAllWindows();

  Superclass::Deactivated();
}

void mitk::MIDASPolyTool::SetPreviousContourVisible(bool visible)
{
  this->Disable3dRenderingOfPreviousContour();

  if (m_PreviousContourVisible == visible)
  {
    return;
  }
  if ( DataStorage* storage = m_ToolManager->GetDataStorage() )
  {
    if (visible)
    {
      storage->Add( m_PreviousContourNode );
    }
    else
    {
      storage->Remove( m_PreviousContourNode );
    }
  }
  m_PreviousContourVisible = visible;
}

void mitk::MIDASPolyTool::SetPolyLinePointSetVisible(bool visible)
{
  if (m_PolyLinePointSetVisible == visible)
  {
    return;
  }
  if ( DataStorage* storage = m_ToolManager->GetDataStorage() )
  {
    if (visible)
    {
      storage->Add( m_PolyLinePointSetNode );
    }
    else
    {
      storage->Remove( m_PolyLinePointSetNode );
    }
  }
  m_PolyLinePointSetVisible = visible;
}

void mitk::MIDASPolyTool::DrawWholeContour(
    const mitk::ContourModel& contourReferencePointsInput,
    const PlaneGeometry* planeGeometry,
    mitk::ContourModel& feedbackContour,
    mitk::ContourModel& backgroundContour
    )
{
  // If these are not set, something is fundamentally wrong.
  assert(m_SegmentationImage);
  assert(m_SegmentationImageGeometry);

  // Reset the contours, as we are redrawing the whole thing.
  feedbackContour.Initialize();
  backgroundContour.Initialize();

  // Only bother drawing if we have at least two points
  if (contourReferencePointsInput.GetNumberOfVertices() > 1)
  {
    const mitk::ContourModel::VertexType* v1 = contourReferencePointsInput.GetVertexAt(0);

    for (unsigned long i = 1; i < contourReferencePointsInput.GetNumberOfVertices(); i++)
    {
      const mitk::ContourModel::VertexType* v2 = contourReferencePointsInput.GetVertexAt(i);

      this->DrawLineAroundVoxelEdges(
        m_SegmentationImage,
        m_SegmentationImageGeometry,
        planeGeometry,
        v2->Coordinates,
        v1->Coordinates,
        feedbackContour,
        backgroundContour
      );

      v1 = v2;
    }
  }
}

void mitk::MIDASPolyTool::UpdateFeedbackContour(
    bool registerNewPoint,
    const mitk::Point3D& closestCornerPoint,
    const PlaneGeometry* planeGeometry,
    mitk::ContourModel& contourReferencePointsInput,
    mitk::ContourModel& feedbackContour,
    mitk::ContourModel& backgroundContour,
    bool provideUndo
    )
{
  // Find closest point in reference points
  float distance = std::numeric_limits<float>::max();
  float closestDistance = std::numeric_limits<float>::max();
  mitk::Point3D closestPoint;
  mitk::Point3D p1;

  if (registerNewPoint)
  {
    m_DraggedPointIndex = 0;
    for (unsigned long i = 0; i < contourReferencePointsInput.GetNumberOfVertices(); i++)
    {
      p1 = contourReferencePointsInput.GetVertexAt(i)->Coordinates;
      distance = mitk::GetSquaredDistanceBetweenPoints(p1, closestCornerPoint);
      if (distance < closestDistance)
      {
        closestDistance = distance;
        closestPoint = p1;
        m_DraggedPointIndex = i;
      }
    }
  }

  if (provideUndo)
  {
    mitk::MIDASPolyToolOpUpdateFeedbackContour *doOp = new mitk::MIDASPolyToolOpUpdateFeedbackContour(
        MIDAS_POLY_TOOL_OP_UPDATE_FEEDBACK_CONTOUR,
        m_DraggedPointIndex,
        closestCornerPoint,
        &contourReferencePointsInput,
        planeGeometry
        );


    mitk::MIDASPolyToolOpUpdateFeedbackContour *undoOp = new mitk::MIDASPolyToolOpUpdateFeedbackContour(
        MIDAS_POLY_TOOL_OP_UPDATE_FEEDBACK_CONTOUR,
        m_DraggedPointIndex,
        m_PreviousContourReferencePoints->GetVertexAt(m_DraggedPointIndex)->Coordinates,
        &contourReferencePointsInput,
        planeGeometry
        );

    mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, "Update PolyLine");
    mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
    this->ExecuteOperation(doOp);
  }
  else
  {
    mitk::ContourModel::VertexType* v = const_cast<mitk::ContourModel::VertexType*>(contourReferencePointsInput.GetVertexAt(m_DraggedPointIndex));
    v->Coordinates = closestCornerPoint;
    this->DrawWholeContour(contourReferencePointsInput, planeGeometry, feedbackContour, backgroundContour);
  }
}

void mitk::MIDASPolyTool::UpdateContours(mitk::StateMachineAction* action, mitk::InteractionPositionEvent* positionEvent, bool provideUndo, bool registerNewPoint)
{
  if (m_ReferencePoints->GetNumberOfVertices() > 1)
  {
    // Don't forget to call baseclass method.
    MIDASContourTool::OnMousePressed(action, positionEvent);

    // If these are not set, something is fundamentally wrong.
    assert(m_SegmentationImage);
    assert(m_SegmentationImageGeometry);

    // Make sure we have valid contours, otherwise no point continuing.
    mitk::ContourModel* feedbackContour = mitk::FeedbackContourTool::GetFeedbackContour();
    assert(feedbackContour);
    mitk::ContourModel* backgroundContour = MIDASContourTool::GetBackgroundContour();
    assert(backgroundContour);

    // Make sure we have a valid geometry, otherwise no point continuing.
    const PlaneGeometry* planeGeometry = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry();
    if (!planeGeometry)
    {
      return;
    }

    // Convert mouse click to closest corner point, as in effect, we always draw from corner to corner.
    mitk::Point3D closestCornerPoint;
    this->ConvertPointToNearestVoxelCentreInMm(positionEvent->GetPositionInWorld(), closestCornerPoint);

    // Redraw the "previous" contour line in green.
    this->DrawWholeContour(*(m_PreviousContourReferencePoints.GetPointer()), planeGeometry, *(m_PreviousContour.GetPointer()), *backgroundContour);

    // Redraw the "current" contour line in yellow.
    this->UpdateFeedbackContour(registerNewPoint, closestCornerPoint, planeGeometry, *(m_ReferencePoints.GetPointer()), *feedbackContour, *backgroundContour, provideUndo);

    // Make sure all views everywhere get updated.
    this->RenderAllWindows();
  }
}

/**
 * Poly lines are created by responding only to left mouse down.
 * When the tool is activated, the next mouse click starts the line.
 * We then keep adding points and lines until the tool is deactivated.
 */
bool mitk::MIDASPolyTool::AddLine(mitk::StateMachineAction* action, mitk::InteractionEvent* event)
{
  // Don't forget to call baseclass method.
  MIDASContourTool::OnMousePressed(action, event);

  // If these are not set, something is fundamentally wrong.
  assert(m_SegmentationImage);
  assert(m_SegmentationImageGeometry);

  // Make sure we have a valid position event, otherwise no point continuing.
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
  if (!positionEvent)
  {
    return false;
  }

  // Similarly, we can't do plane calculations if no geometry set.
  const PlaneGeometry* planeGeometry = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry();
  if (!planeGeometry) return false;

  // Convert mouse click to closest corner point, as in effect, we always draw from corner to corner.
  mitk::Point3D closestCornerPoint;
  this->ConvertPointToNearestVoxelCentreInMm(positionEvent->GetPositionInWorld(), closestCornerPoint);

  mitk::Point3D previousPoint;
  if (m_ReferencePoints->GetNumberOfVertices() > 0)
  {
    previousPoint = m_ReferencePoints->GetVertexAt(m_ReferencePoints->GetNumberOfVertices() - 1)->Coordinates;
  }

  mitk::ContourModel::Pointer currentPoints = mitk::ContourModel::New();
  this->CopyContour(*(m_ReferencePoints.GetPointer()), *(currentPoints.GetPointer()));

  mitk::ContourModel::Pointer nextPoints = mitk::ContourModel::New();
  this->CopyContour(*(m_ReferencePoints.GetPointer()), *(nextPoints.GetPointer()));
  nextPoints->AddVertex(closestCornerPoint);

  mitk::MIDASPolyToolOpAddToFeedbackContour *doOp = new mitk::MIDASPolyToolOpAddToFeedbackContour(
      MIDAS_POLY_TOOL_OP_ADD_TO_FEEDBACK_CONTOUR,
      closestCornerPoint,
      nextPoints,
      planeGeometry
      );


  mitk::MIDASPolyToolOpAddToFeedbackContour *undoOp = new mitk::MIDASPolyToolOpAddToFeedbackContour(
      MIDAS_POLY_TOOL_OP_ADD_TO_FEEDBACK_CONTOUR,
      previousPoint,
      currentPoints,
      planeGeometry
      );

  mitk::OperationEvent* operationEvent = new mitk::OperationEvent( m_Interface, doOp, undoOp, "Add to PolyLine");
  mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );
  this->ExecuteOperation(doOp);

  // Set this flag to indicate that we have stopped editing, which will trigger an update of the region growing.
  this->UpdateWorkingDataNodeBoolProperty(SEGMENTATION, mitk::MIDASContourTool::EDITING_PROPERTY_NAME, false);
  return true;
}

bool mitk::MIDASPolyTool::SelectPoint(mitk::StateMachineAction* action, mitk::InteractionEvent* event)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
  if (!positionEvent)
  {
    return false;
  }

  this->CopyContour(*(m_ReferencePoints), *(m_PreviousContourReferencePoints));
  this->UpdateContours(action, positionEvent, false, true);
  this->UpdateWorkingDataNodeBoolProperty(SEGMENTATION, mitk::MIDASContourTool::EDITING_PROPERTY_NAME, true);
  return true;
}

bool mitk::MIDASPolyTool::MovePoint(mitk::StateMachineAction* action, mitk::InteractionEvent* event)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
  if (!positionEvent)
  {
    return false;
  }

  this->SetPreviousContourVisible(true);
  this->UpdateContours(action, positionEvent, false, false);
  this->UpdateWorkingDataNodeBoolProperty(SEGMENTATION, mitk::MIDASContourTool::EDITING_PROPERTY_NAME, true);
  return true;
}

bool mitk::MIDASPolyTool::DeselectPoint(mitk::StateMachineAction* action, mitk::InteractionEvent* event)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(event);
  if (!positionEvent)
  {
    return false;
  }
  this->SetPreviousContourVisible(false);
  this->UpdateContours(action, positionEvent, true, false);
  this->UpdateWorkingDataNodeBoolProperty(SEGMENTATION, mitk::MIDASContourTool::EDITING_PROPERTY_NAME, false);
  return true;
}

void mitk::MIDASPolyTool::ExecuteOperation(Operation* operation)
{
  if (!operation) return;

  mitk::MIDASContourTool::ExecuteOperation(operation);

  mitk::ContourModel* feedbackContour = mitk::FeedbackContourTool::GetFeedbackContour();
  assert(feedbackContour);

  // Retrieve the background contour, used to plot the closest straight line.
  mitk::ContourModel* backgroundContour = MIDASContourTool::GetBackgroundContour();
  assert(backgroundContour);

  switch (operation->GetOperationType())
  {
  case MIDAS_POLY_TOOL_OP_ADD_TO_FEEDBACK_CONTOUR:
    {
      MIDASPolyToolOpAddToFeedbackContour *op = static_cast<MIDASPolyToolOpAddToFeedbackContour*>(operation);
      if (op != NULL)
      {
        mitk::Point3D point = op->GetPoint();
        mitk::ContourModel* contour = op->GetContour();
        const mitk::PlaneGeometry* planeGeometry = op->GetPlaneGeometry();

        if (contour->GetNumberOfVertices() == 1)
        {
          m_PolyLinePointSet->InsertPoint(0, point);
          this->SetPolyLinePointSetVisible(true);
        }
        else
        {
          this->SetPolyLinePointSetVisible(false);
        }
        this->CopyContour(*contour, *(m_ReferencePoints.GetPointer()));
        m_MostRecentPointInMillimetres = point;
        this->DrawWholeContour(*(m_ReferencePoints.GetPointer()), planeGeometry, *feedbackContour, *backgroundContour);
      }
    }
    break;
  case MIDAS_POLY_TOOL_OP_UPDATE_FEEDBACK_CONTOUR:
    {
      MIDASPolyToolOpUpdateFeedbackContour *op = static_cast<MIDASPolyToolOpUpdateFeedbackContour*>(operation);
      if (op != NULL)
      {
        unsigned int pointId = op->GetPointId();
        mitk::Point3D point = op->GetPoint();
        mitk::ContourModel* contour = op->GetContour();
        const mitk::PlaneGeometry* planeGeometry = op->GetPlaneGeometry();

        if (pointId < contour->GetNumberOfVertices())
        {
          mitk::ContourModel::VertexType* v = const_cast<mitk::ContourModel::VertexType*>(contour->GetVertexAt(pointId));
          v->Coordinates = point;
          this->DrawWholeContour(*contour, planeGeometry, *feedbackContour, *backgroundContour);
        }
        else
        {
          MITK_ERROR << "Received invalid pointId=" << pointId << std::endl;
        }
      }
    }
    break;
  default:
    ;
  }

  // Signal that something has happened, and that it may be worth updating.
  ContoursHaveChanged.Send();

  // Make sure all views everywhere get updated.
  this->RenderAllWindows();
}
