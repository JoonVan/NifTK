/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkMIDASPointSetInteractor.h"
#include "mitkPositionEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkPointSet.h"
#include "mitkStateEvent.h"
#include "mitkAction.h"
#include "mitkInteractionConst.h"

mitk::MIDASPointSetInteractor
::MIDASPointSetInteractor(const char * type, DataNode* dataNode, int n)
:PointSetInteractor(type, dataNode, n)
{
}

mitk::MIDASPointSetInteractor::~MIDASPointSetInteractor()
{
}

//##Documentation
//## overwritten cause this class can handle it better!
float mitk::MIDASPointSetInteractor::CanHandleEvent(StateEvent const* stateEvent) const
{
  float returnValue = 0.0;

  //if it is a key event that can be handled in the current state, then return 0.5
  mitk::DisplayPositionEvent const  *disPosEvent =
    dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (disPosEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0;
    }
  }

  //get the time of the sender to look for the right transition.
  mitk::BaseRenderer* sender = stateEvent->GetEvent()->GetSender();
  if (sender != NULL)
  {
    unsigned int timeStep = sender->GetTimeStep(m_DataNode->GetData());

    //if the event can be understood and if there is a transition waiting for that event
    mitk::State const* state = this->GetCurrentState(timeStep);
    if (state!= NULL)
    {
      if (state->GetTransition(stateEvent->GetId())!=NULL)
      {
        returnValue = 0.5;//it can be understood
      }
    }


    mitk::PointSet *pointSet = dynamic_cast<mitk::PointSet*>(m_DataNode->GetData());
    if ( pointSet != NULL )
    {
      //if we have one point or more, then check if the have been picked
      if ( (pointSet->GetSize( timeStep ) > 0)
        && (pointSet->SearchPoint(disPosEvent->GetWorldPosition(), m_Precision, timeStep) > -1) )
      {
        returnValue = 1.0;
      }
    }
  }
  return returnValue;
}