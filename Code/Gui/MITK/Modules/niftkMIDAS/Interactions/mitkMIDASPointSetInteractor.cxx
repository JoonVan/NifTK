/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkMIDASPointSetInteractor.h"
#include <mitkPositionEvent.h>
#include <mitkBaseRenderer.h>
#include <mitkRenderingManager.h>
#include <mitkPointSet.h>
#include <mitkStateEvent.h>
#include <mitkAction.h>
#include <mitkInteractionConst.h>

mitk::MIDASPointSetInteractor::MIDASPointSetInteractor(const char * type, DataNode* dataNode, int n)
: mitk::PointSetInteractor(type, dataNode, n)
{
  this->SetPrecision(1);
}

mitk::MIDASPointSetInteractor::~MIDASPointSetInteractor()
{
}

float mitk::MIDASPointSetInteractor::CanHandleEvent(const mitk::StateEvent* stateEvent) const
{
  return mitk::MIDASStateMachine::CanHandleEvent(stateEvent);
}

//##Documentation
//## overwritten cause this class can handle it better!
float mitk::MIDASPointSetInteractor::CanHandle(const mitk::StateEvent* stateEvent) const
{
  float returnValue = 0.0f;

  //if it is a key event that can be handled in the current state, then return 0.5
  const mitk::DisplayPositionEvent* displayPositionEvent =
    dynamic_cast<const mitk::DisplayPositionEvent*>(stateEvent->GetEvent());

  // Key event handling:
  if (!displayPositionEvent)
  {
    // Check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId()))
    {
      return 0.5f;
    }
    else
    {
      return 0.0f;
    }
  }

  // Get the time of the sender to look for the right transition.
  mitk::BaseRenderer* renderer = stateEvent->GetEvent()->GetSender();
  if (renderer)
  {
    unsigned int timeStep = renderer->GetTimeStep(m_DataNode->GetData());

    // If the event can be understood and if there is a transition waiting for that event
    mitk::State const* state = this->GetCurrentState(timeStep);
    if (state)
    {
      if (state->GetTransition(stateEvent->GetId()))
      {
        returnValue = 0.5; //it can be understood
      }
    }

    mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
    if (pointSet)
    {
      // if we have one point or more, then check if the have been picked
      if (pointSet->GetSize(timeStep) > 0
          && pointSet->SearchPoint(displayPositionEvent->GetWorldPosition(), m_Precision, timeStep) > -1)
      {
        returnValue = 1.0;
      }
    }
  }
  return returnValue;
}

bool mitk::MIDASPointSetInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  mitk::DisplayPositionEvent const *displayPositionEvent =
      dynamic_cast<const mitk::DisplayPositionEvent*>(stateEvent->GetEvent());

  if (displayPositionEvent)
  {
    mitk::BaseRenderer* renderer = displayPositionEvent->GetSender();

    mitk::Point3D point3DInMm = displayPositionEvent->GetWorldPosition();
    const mitk::Geometry3D* worldGeometry = renderer->GetWorldGeometry();
    mitk::Point3D point3DIndex;
    worldGeometry->WorldToIndex(point3DInMm, point3DIndex);
    point3DIndex[0] = std::floor(point3DIndex[0]) + 0.5;
    point3DIndex[1] = std::floor(point3DIndex[1]) + 0.5;
    worldGeometry->IndexToWorld(point3DIndex, point3DInMm);

    mitk::Point2D point2DInMm;
    mitk::Point2D point2DInPx;

    mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
    displayGeometry->Map(point3DInMm, point2DInMm);
    displayGeometry->WorldToDisplay(point2DInMm, point2DInPx);

    const_cast<mitk::DisplayPositionEvent*>(displayPositionEvent)->SetDisplayPosition(point2DInPx);
  }

  return Superclass::ExecuteAction(action, stateEvent);
}
