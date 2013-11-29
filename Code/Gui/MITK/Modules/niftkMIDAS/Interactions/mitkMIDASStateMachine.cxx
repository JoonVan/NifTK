/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkMIDASStateMachine.h"

#include <mitkToolManager.h>
#include <mitkGlobalInteraction.h>
#include <itkCommand.h>

// MicroServices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>

#include <Interactions/mitkDnDDisplayInteractor.h>

#include "mitkMIDASEventFilter.h"

//-----------------------------------------------------------------------------
mitk::MIDASStateMachine::MIDASStateMachine()
{
}


//-----------------------------------------------------------------------------
mitk::MIDASStateMachine::~MIDASStateMachine()
{
}


//-----------------------------------------------------------------------------
float mitk::MIDASStateMachine::CanHandleEvent(const mitk::StateEvent* stateEvent) const
{
  if (this->IsFiltered(stateEvent))
  {
    return 0.0f;
  }

  return this->CanHandle(stateEvent);
}


//-----------------------------------------------------------------------------
void mitk::MIDASStateMachine::InstallEventFilter(mitk::MIDASEventFilter* eventFilter)
{
  std::vector<MIDASEventFilter*>::iterator it =
      std::find(m_EventFilters.begin(), m_EventFilters.end(), eventFilter);

  if (it == m_EventFilters.end())
  {
    m_EventFilters.push_back(eventFilter);
  }
}


//-----------------------------------------------------------------------------
void mitk::MIDASStateMachine::RemoveEventFilter(mitk::MIDASEventFilter* eventFilter)
{
  std::vector<MIDASEventFilter*>::iterator it =
      std::find(m_EventFilters.begin(), m_EventFilters.end(), eventFilter);

  if (it != m_EventFilters.end())
  {
    m_EventFilters.erase(it);
  }
}


//-----------------------------------------------------------------------------
std::vector<mitk::MIDASEventFilter*> mitk::MIDASStateMachine::GetEventFilters() const
{
  return m_EventFilters;
}


//-----------------------------------------------------------------------------
bool mitk::MIDASStateMachine::IsFiltered(const mitk::StateEvent* event) const
{
  /// Sanity check.
  if (!event || !event->GetEvent() || !event->GetEvent()->GetSender())
  {
    return false;
  }

  std::vector<MIDASEventFilter*>::const_iterator it = m_EventFilters.begin();
  std::vector<MIDASEventFilter*>::const_iterator itEnd = m_EventFilters.end();

  for ( ; it != itEnd; ++it)
  {
    if ((*it)->EventFilter(event))
    {
      return true;
    }
  }

  return false;
}
