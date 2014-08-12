/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkDnDDisplayStateMachineResponder_h
#define mitkDnDDisplayStateMachineResponder_h

namespace mitk {

/**
 * \class DnDDisplayStateMachineResponder
 * \brief Pure Virtual Interface to be implemented by classes that want to
 * respond to DnD Display key events relevant to how the image is viewed where events
 * are passed through from the DnDDisplayStateMachine which is a subclass of
 * StateMachine, and hence registered with the interaction loop.
 *
 * \sa DnDDisplayStateMachine
 * \sa StateMachine
 */
class DnDDisplayStateMachineResponder
{
public:

  DnDDisplayStateMachineResponder() {}
  virtual ~DnDDisplayStateMachineResponder() {}

  /// \brief Move anterior a slice.
  virtual bool MoveAnterior() = 0;

  /// \brief Move posterior a slice.
  virtual bool MovePosterior() = 0;

  /// \brief Moves back a time step.
  virtual bool SelectPreviousTimeStep() = 0;

  /// \brief Moves forward a time step.
  virtual bool SelectNextTimeStep() = 0;

  /// \brief Switch to Axial.
  virtual bool SwitchToAxial() = 0;

  /// \brief Switch to Sagittal.
  virtual bool SwitchToSagittal() = 0;

  /// \brief Switch to Coronal.
  virtual bool SwitchToCoronal() = 0;

  /// \brief Switch window layout.
  virtual bool ToggleMultiWindowLayout() = 0;

  /// \brief Toggles the visibility of the cursor, aka. crosshair.
  virtual bool ToggleCursorVisibility() = 0;

};

} // end namespace

#endif
