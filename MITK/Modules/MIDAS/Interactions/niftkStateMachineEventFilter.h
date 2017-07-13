/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkStateMachineEventFilter_h
#define niftkStateMachineEventFilter_h

#include "niftkMIDASExports.h"

#include <mitkCommon.h>

namespace mitk
{
class StateEvent;
class InteractionEvent;
}

namespace niftk
{

/**
 * \class StateMachineEventFilter
 *
 * \brief StateMachineEventFilter represents a condition that has to be fulfilled
 * so that an event is processed by a tool or interactor that derives from
 * niftk::FilteringStateMachine.
 *
 * This can be used e.g. to restrict the scope of a tool or interactor to specific
 * render windows.
 */
class NIFTKMIDAS_EXPORT StateMachineEventFilter
{

public:

  mitkClassMacroNoParent(StateMachineEventFilter)

  StateMachineEventFilter();
  virtual ~StateMachineEventFilter();

  /// \brief Returns true if the event should be filtered, i.e. not processed,
  /// otherwise false.
  virtual bool EventFilter(const mitk::StateEvent* stateEvent) const
  {
    return false;
  }

  virtual bool EventFilter(mitk::InteractionEvent* event) const
  {
    return false;
  }

};

}

#endif
