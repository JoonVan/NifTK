/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkDnDDisplayInteractor_h
#define mitkDnDDisplayInteractor_h

#include <niftkDnDDisplayExports.h>

#include <mitkDisplayInteractor.h>
#include <niftkDnDDisplayEnums.h>

#include <vector>
#include <QObject>

class niftkSingleViewerWidget;
class QmitkRenderWindow;
class QTimer;

namespace mitk
{

class FocusManager;

/**
 *\class DnDDisplayInteractor
 *@brief Observer that manages the interaction with the display.
 *
 * mitk::ToolManager reloads the configuration of the registered mitk::DisplayInteractor
 * objects when a tool is switched on. This configuration conflicts with most of the tools.
 *
 * @ingroup Interaction
 **/
/**
 * Inherits from mitk::InteractionEventObserver since it doesn't alter any data (only their representation),
 * and its actions cannot be associated with a DataNode. Also inherits from EventStateMachine
 */
class NIFTKDNDDISPLAY_EXPORT DnDDisplayInteractor: public QObject, public mitk::DisplayInteractor
{
  Q_OBJECT

public:
  mitkClassMacro(DnDDisplayInteractor, DisplayInteractor)
  mitkNewMacro1Param(Self, niftkSingleViewerWidget*);

  /**
   * By this function the Observer gets notifier about new events.
   * Here it is adapted to pass the events to the state machine in order to use
   * its infrastructure.
   * It also checks if event is to be accepted when i already has been processed by a DataInteractor.
   */
  virtual void Notify(InteractionEvent* interactionEvent, bool isHandled);

protected:
  DnDDisplayInteractor(niftkSingleViewerWidget* viewer);
  virtual ~DnDDisplayInteractor();

  virtual void ConnectActionsAndFunctions();

  virtual bool SelectPosition(StateMachineAction*, InteractionEvent*);

  /// \brief Like Superclass::Init, but blocks the update and selects the focused window.
  virtual bool InitMove(StateMachineAction*, InteractionEvent*);

  /// \brief Like Superclass::Init, but blocks the update and selects the focused window.
  /// It also changes the selected position to the middle of the focused voxel.
  virtual bool InitZoom(StateMachineAction*, InteractionEvent*);

  /// \brief Switches to axial window layout.
  virtual bool SetWindowLayoutToAxial(StateMachineAction*, InteractionEvent*);

  /// \brief Switches to sagittal window layout.
  virtual bool SetWindowLayoutToSagittal(StateMachineAction*, InteractionEvent*);

  /// \brief Switches to coronal window layout.
  virtual bool SetWindowLayoutToCoronal(StateMachineAction*, InteractionEvent*);

  /// \brief Switches to 3D window layout.
  virtual bool SetWindowLayoutTo3D(StateMachineAction*, InteractionEvent*);

  /// \brief Switches to multi window layout.
  virtual bool SetWindowLayoutToMulti(StateMachineAction*, InteractionEvent*);

  /// \brief Toggles between single and multi window layout.
  virtual bool ToggleMultiWindowLayout(StateMachineAction*, InteractionEvent*);

  /// \brief Toggles the visibility of the cursor.
  virtual bool ToggleCursorVisibility(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Toggles displaying the direction annotations on/off.
  virtual bool ToggleDirectionAnnotations(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Toggles displaying the intensity annotation on/off.
  virtual bool ToggleIntensityAnnotation(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Selects the previous slice.
  /// The slices are ordered in the following way:
  ///   <li>axial: inferior to superior
  ///   <li>sagittal: right to left
  ///   <li>coronal: anterior to posterior
  virtual bool SelectPreviousSlice(StateMachineAction*, InteractionEvent*);

  /// \brief Selects the next slice.
  /// The slices are ordered in the following way:
  ///   <li>axial: inferior to superior
  ///   <li>sagittal: right to left
  ///   <li>coronal: anterior to posterior
  virtual bool SelectNextSlice(StateMachineAction*, InteractionEvent*);

  /// \brief Selects the previous time step.
  virtual bool SelectPreviousTimeStep(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Selects the next time step.
  virtual bool SelectNextTimeStep(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Starts scrolling through slices in a loop backwards.
  virtual bool StartScrollingThroughSlicesBackwards(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Starts scrolling through slices in a loop in posterior direction.
  virtual bool StartScrollingThroughSlicesForwards(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Starts scrolling through time steps in a loop, backwards.
  virtual bool StartScrollingThroughTimeStepsBackwards(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Starts scrolling through time steps in a loop, forwards.
  virtual bool StartScrollingThroughTimeStepsForwards(StateMachineAction* action, InteractionEvent* interactionEvent);

  /// \brief Stops scrolling through slices.
  virtual bool StopScrolling(StateMachineAction* action, InteractionEvent* interactionEvent);

private slots:

  /// \brief Selects the previous slice or the last slice if the first slice is currently selected.
  /// This slot connected to a timer when the user starts auto-scrolling slices backwards.
  void SelectPreviousSlice();

  /// \brief Selects the next slice or the first slice if the last slice is currently selected.
  /// This slot connected to a timer when the user starts auto-scrolling slices forwards.
  void SelectNextSlice();

  /// \brief Selects the previous time step or the last time step if the first time step is currently selected.
  /// This slot connected to a timer when the user starts auto-scrolling time steps backwards.
  void SelectPreviousTimeStep();

  /// \brief Selects the next time step or the first time step if the last time step is currently selected.
  /// This slot connected to a timer when the user starts auto-scrolling time steps forwards.
  void SelectNextTimeStep();

private:

  QmitkRenderWindow* GetRenderWindow(mitk::BaseRenderer* renderer);

  niftkSingleViewerWidget* m_Viewer;

  std::vector<mitk::BaseRenderer*> m_Renderers;

  mitk::FocusManager* m_FocusManager;

  QTimer* m_AutoScrollTimer;

};

}

#endif
