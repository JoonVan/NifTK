/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkRendererFilter.h"

#include <mitkStateEvent.h>
#include <mitkInteractionEvent.h>
#include <mitkBaseRenderer.h>

namespace niftk
{

//-----------------------------------------------------------------------------
MIDASRendererFilter::MIDASRendererFilter()
{
}


//-----------------------------------------------------------------------------
MIDASRendererFilter::~MIDASRendererFilter()
{
}


//-----------------------------------------------------------------------------
bool MIDASRendererFilter::EventFilter(const mitk::StateEvent* stateEvent) const
{
  mitk::BaseRenderer* renderer = stateEvent->GetEvent()->GetSender();
  std::vector<mitk::BaseRenderer*>::const_iterator it =
      std::find(m_Renderers.begin(), m_Renderers.end(), renderer);
  return it == m_Renderers.end();
}


//-----------------------------------------------------------------------------
bool MIDASRendererFilter::EventFilter(mitk::InteractionEvent* event) const
{
  mitk::BaseRenderer* renderer = event->GetSender();
  std::vector<mitk::BaseRenderer*>::const_iterator it =
      std::find(m_Renderers.begin(), m_Renderers.end(), renderer);
  return it == m_Renderers.end();
}


//-----------------------------------------------------------------------------
void MIDASRendererFilter::AddRenderer(mitk::BaseRenderer* renderer)
{
  std::vector<mitk::BaseRenderer*>::iterator it =
      std::find(m_Renderers.begin(), m_Renderers.end(), renderer);

  if (it == m_Renderers.end())
  {
    m_Renderers.push_back(renderer);
  }
}


//-----------------------------------------------------------------------------
void MIDASRendererFilter::RemoveRenderer(mitk::BaseRenderer* renderer)
{
  std::vector<mitk::BaseRenderer*>::iterator it =
      std::find(m_Renderers.begin(), m_Renderers.end(), renderer);

  if (it != m_Renderers.end())
  {
    m_Renderers.erase(it);
  }
}

}
