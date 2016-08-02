/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkDnDDisplayInteractor.h"

#include <string.h>

#include <QTimer>

#include <mitkBaseRenderer.h>
#include <mitkGlobalInteraction.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkLine.h>
#include <mitkSliceNavigationController.h>

#include <niftkInteractionEventObserverMutex.h>

#include "niftkSingleViewerWidget.h"


namespace niftk
{

//-----------------------------------------------------------------------------
DnDDisplayInteractor::DnDDisplayInteractor(SingleViewerWidget* viewer)
: mitk::DisplayInteractor()
, m_Viewer(viewer)
, m_Renderers(4)
, m_FocusManager(mitk::GlobalInteraction::GetInstance()->GetFocusManager())
, m_AutoScrollTimer(new QTimer(this))
{
  const std::vector<QmitkRenderWindow*>& renderWindows = m_Viewer->GetRenderWindows();
  m_Renderers[0] = renderWindows[0]->GetRenderer();
  m_Renderers[1] = renderWindows[1]->GetRenderer();
  m_Renderers[2] = renderWindows[2]->GetRenderer();
  m_Renderers[3] = renderWindows[3]->GetRenderer();

  m_AutoScrollTimer->setInterval(200);
}


//-----------------------------------------------------------------------------
DnDDisplayInteractor::~DnDDisplayInteractor()
{
}


//-----------------------------------------------------------------------------
void DnDDisplayInteractor::Notify(mitk::InteractionEvent* interactionEvent, bool isHandled)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (std::find(m_Renderers.begin(), m_Renderers.end(), renderer) != m_Renderers.end())
  {
    Superclass::Notify(interactionEvent, isHandled);
  }
}


//-----------------------------------------------------------------------------
void DnDDisplayInteractor::ConnectActionsAndFunctions()
{
  /// Note:
  /// We do not call the overridden function here. It assign handlers to actions
  /// that are not defined for this state machine.

  CONNECT_FUNCTION("startSelectingPosition", StartSelectingPosition);
  CONNECT_FUNCTION("selectPosition", SelectPosition);
  CONNECT_FUNCTION("stopSelectingPosition", StopSelectingPosition);
  CONNECT_FUNCTION("startPanning", StartPanning);
  CONNECT_FUNCTION("pan", Pan);
  CONNECT_FUNCTION("stopPanning", StopPanning);
  CONNECT_FUNCTION("startZooming", StartZooming);
  CONNECT_FUNCTION("zoom", Zoom);
  CONNECT_FUNCTION("stopZooming", StopZooming);
  CONNECT_FUNCTION("setWindowLayoutToAxial", SetWindowLayoutToAxial);
  CONNECT_FUNCTION("setWindowLayoutToSagittal", SetWindowLayoutToSagittal);
  CONNECT_FUNCTION("setWindowLayoutToCoronal", SetWindowLayoutToCoronal);
  CONNECT_FUNCTION("setWindowLayoutTo3D", SetWindowLayoutTo3D);
  CONNECT_FUNCTION("setWindowLayoutToMulti", SetWindowLayoutToMulti);
  CONNECT_FUNCTION("toggleMultiWindowLayout", ToggleMultiWindowLayout);
  CONNECT_FUNCTION("toggleCursorVisibility", ToggleCursorVisibility);
  CONNECT_FUNCTION("toggleDirectionAnnotations", ToggleDirectionAnnotations);
  CONNECT_FUNCTION("toggleIntensityAnnotation", ToggleIntensityAnnotation);
  CONNECT_FUNCTION("selectPreviousSlice", SelectPreviousSlice);
  CONNECT_FUNCTION("selectNextSlice", SelectNextSlice);
  CONNECT_FUNCTION("selectPreviousTimeStep", SelectPreviousTimeStep);
  CONNECT_FUNCTION("selectNextTimeStep", SelectNextTimeStep);

  CONNECT_FUNCTION("startScrollingThroughSlicesBackwards", StartScrollingThroughSlicesBackwards);
  CONNECT_FUNCTION("startScrollingThroughSlicesForwards", StartScrollingThroughSlicesForwards);
  CONNECT_FUNCTION("startScrollingThroughTimeStepsBackwards", StartScrollingThroughTimeStepsBackwards);
  CONNECT_FUNCTION("startScrollingThroughTimeStepsForwards", StartScrollingThroughTimeStepsForwards);
  CONNECT_FUNCTION("stopScrolling", StopScrolling);
}


//-----------------------------------------------------------------------------
QmitkRenderWindow* DnDDisplayInteractor::GetRenderWindow(mitk::BaseRenderer* renderer)
{
  QmitkRenderWindow* renderWindow = 0;

  std::size_t i = std::find(m_Renderers.begin(), m_Renderers.end(), renderer) - m_Renderers.begin();

  if (i < 4)
  {
    renderWindow = m_Viewer->GetRenderWindows()[i];
  }

  return renderWindow;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartSelectingPosition(mitk::StateMachineAction* /*action*/, mitk::InteractionEvent* /*interactionEvent*/)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SelectPosition(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();

  /// Note:
  /// We do not re-implement position selection for the 3D window.
  if (renderer == m_Renderers[3])
  {
    return false;
  }

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  assert(positionEvent);

  // First, check if the slice navigation controllers have a valid geometry,
  // i.e. an image is loaded.
  if (!m_Renderers[0]->GetSliceNavigationController()->GetCreatedWorldGeometry())
  {
    return false;
  }

  bool updateWasBlocked = m_Viewer->BlockUpdate(true);

  if (renderer != m_FocusManager->GetFocused())
  {
    QmitkRenderWindow* renderWindow = this->GetRenderWindow(renderer);
    m_Viewer->SetSelectedRenderWindow(renderWindow);
    m_Viewer->SetFocused();
  }

  // Selects the point under the mouse pointer in the slice navigation controllers.
  // In the MultiWindowWidget this puts the crosshair to the mouse position, and
  // selects the slice in the two other render window.
  const mitk::Point3D& positionInWorld = positionEvent->GetPositionInWorld();
  m_Viewer->SetSelectedPosition(positionInWorld);

  m_Viewer->BlockUpdate(updateWasBlocked);

  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StopSelectingPosition(mitk::StateMachineAction* /*action*/, mitk::InteractionEvent* /*interactionEvent*/)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Unlock(this);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartPanning(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  assert(positionEvent);

  // First, check if the slice navigation controllers have a valid geometry,
  // i.e. an image is loaded.
  if (!m_Renderers[0]->GetSliceNavigationController()->GetCreatedWorldGeometry())
  {
    return false;
  }

  bool updateWasBlocked = m_Viewer->BlockUpdate(true);

  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (renderer != m_FocusManager->GetFocused())
  {
    QmitkRenderWindow* renderWindow = this->GetRenderWindow(renderer);
    m_Viewer->SetSelectedRenderWindow(renderWindow);
    m_Viewer->SetFocused();
  }

  bool result = this->Init(action, interactionEvent);

  m_Viewer->BlockUpdate(updateWasBlocked);

  return result;
}


////-----------------------------------------------------------------------------
bool DnDDisplayInteractor::Pan(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  return Superclass::Move(action, interactionEvent);
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StopPanning(mitk::StateMachineAction* /*action*/, mitk::InteractionEvent* /*interactionEvent*/)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Unlock(this);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartZooming(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);

  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);
  assert(positionEvent);

  // First, check if the slice navigation controllers have a valid geometry,
  // i.e. an image is loaded.
  if (!m_Renderers[0]->GetSliceNavigationController()->GetCreatedWorldGeometry())
  {
    return false;
  }

  bool updateWasBlocked = m_Viewer->BlockUpdate(true);

  mitk::BaseRenderer* renderer = interactionEvent->GetSender();
  if (renderer != m_FocusManager->GetFocused())
  {
    QmitkRenderWindow* renderWindow = this->GetRenderWindow(renderer);
    m_Viewer->SetSelectedRenderWindow(renderWindow);
    m_Viewer->SetFocused();
  }

  /// Note that the zoom focus must always be the selected position,
  /// i.e. the position at the cursor (crosshair).
  mitk::Point3D focusPoint3DInMm = m_Viewer->GetSelectedPosition();

  mitk::Point2D focusPoint2DInMm;
  mitk::Point2D focusPoint2DInPx;
  mitk::Point2D focusPoint2DInPxUL;

  mitk::DisplayGeometry* displayGeometry = renderer->GetDisplayGeometry();
  displayGeometry->Map(focusPoint3DInMm, focusPoint2DInMm);
  displayGeometry->WorldToDisplay(focusPoint2DInMm, focusPoint2DInPx);
  displayGeometry->DisplayToULDisplay(focusPoint2DInPx, focusPoint2DInPxUL);

  // Create a new position event with the selected position.
  mitk::InteractionPositionEvent::Pointer positionEvent2 = mitk::InteractionPositionEvent::New(renderer, focusPoint2DInPxUL, focusPoint3DInMm);

  bool result = this->Init(action, positionEvent2);

  m_Viewer->BlockUpdate(updateWasBlocked);

  return result;
}


////-----------------------------------------------------------------------------
bool DnDDisplayInteractor::Zoom(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  return Superclass::Zoom(action, interactionEvent);
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StopZooming(mitk::StateMachineAction* /*action*/, mitk::InteractionEvent* /*interactionEvent*/)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Unlock(this);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SetWindowLayoutToAxial(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->SetWindowLayout(WINDOW_LAYOUT_AXIAL);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SetWindowLayoutToSagittal(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->SetWindowLayout(WINDOW_LAYOUT_SAGITTAL);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SetWindowLayoutToCoronal(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->SetWindowLayout(WINDOW_LAYOUT_CORONAL);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SetWindowLayoutTo3D(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->SetWindowLayout(WINDOW_LAYOUT_3D);
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SetWindowLayoutToMulti(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->ToggleMultiWindowLayout();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::ToggleMultiWindowLayout(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->ToggleMultiWindowLayout();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::ToggleCursorVisibility(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->ToggleCursorVisibility();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::ToggleDirectionAnnotations(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->ToggleDirectionAnnotations();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::ToggleIntensityAnnotation(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  m_Viewer->ToggleIntensityAnnotation();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SelectPreviousSlice(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();

  /// Note:
  /// We do not implement scrolling for the 3D window.
  if (renderer == m_Renderers[3])
  {
    return true;
  }

  bool updateWasBlocked = m_Viewer->BlockUpdate(true);

  if (renderer != m_FocusManager->GetFocused())
  {
    QmitkRenderWindow* renderWindow = this->GetRenderWindow(renderer);
    m_Viewer->SetSelectedRenderWindow(renderWindow);
    m_Viewer->SetFocused();
  }

  /// Note:
  /// This does not work if the slice are locked.
  /// See:
  ///   SingleViewerWidget::SetNavigationControllerEventListening(bool)
  /// and
  ///   QmitkMultiWindowWidget::SetWidgetPlanesLocked(bool)

//  bool result = Superclass::ScrollOneUp(action, interactionEvent);

  WindowOrientation orientation = m_Viewer->GetOrientation();
  m_Viewer->MoveSlice(orientation, -1);

  m_Viewer->BlockUpdate(updateWasBlocked);

//  return result;
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SelectNextSlice(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  mitk::BaseRenderer* renderer = interactionEvent->GetSender();

  /// Note:
  /// We do not implement scrolling for the 3D window.
  if (renderer == m_Renderers[3])
  {
    return true;
  }

  bool updateWasBlocked = m_Viewer->BlockUpdate(true);

  if (renderer != m_FocusManager->GetFocused())
  {
    QmitkRenderWindow* renderWindow = this->GetRenderWindow(renderer);
    m_Viewer->SetSelectedRenderWindow(renderWindow);
    m_Viewer->SetFocused();
  }

  /// Note:
  /// This does not work if the slice are locked.
  /// See:
  ///   SingleViewerWidget::SetNavigationControllerEventListening(bool)
  /// and
  ///   QmitkMultiWindowWidget::SetWidgetPlanesLocked(bool)

//  bool result = Superclass::ScrollOneDown(action, interactionEvent);

  WindowOrientation orientation = m_Viewer->GetOrientation();
  m_Viewer->MoveSlice(orientation, +1);

  m_Viewer->BlockUpdate(updateWasBlocked);

//  return result;
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SelectPreviousTimeStep(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  int timeStep = m_Viewer->GetTimeStep() - 1;

  if (timeStep >= 0)
  {
    m_Viewer->SetTimeStep(timeStep);
  }

  return timeStep == m_Viewer->GetTimeStep();
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::SelectNextTimeStep(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  int timeStep = m_Viewer->GetTimeStep() + 1;

  if (timeStep <= m_Viewer->GetMaxTimeStep())
  {
    m_Viewer->SetTimeStep(timeStep);
  }

  return timeStep == m_Viewer->GetTimeStep();
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartScrollingThroughSlicesBackwards(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);

  this->connect(m_AutoScrollTimer, SIGNAL(timeout()), SLOT(SelectPreviousSlice()));
  m_AutoScrollTimer->start();

  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartScrollingThroughSlicesForwards(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);

  this->connect(m_AutoScrollTimer, SIGNAL(timeout()), SLOT(SelectNextSlice()));
  m_AutoScrollTimer->start();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartScrollingThroughTimeStepsBackwards(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);

  this->connect(m_AutoScrollTimer, SIGNAL(timeout()), SLOT(SelectPreviousTimeStep()));
  m_AutoScrollTimer->start();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StartScrollingThroughTimeStepsForwards(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Lock(this);

  this->connect(m_AutoScrollTimer, SIGNAL(timeout()), SLOT(SelectNextTimeStep()));
  m_AutoScrollTimer->start();
  return true;
}


//-----------------------------------------------------------------------------
bool DnDDisplayInteractor::StopScrolling(mitk::StateMachineAction* action, mitk::InteractionEvent* interactionEvent)
{
  niftk::InteractionEventObserverMutex::GetInstance()->Unlock(this);

  m_AutoScrollTimer->stop();
  m_AutoScrollTimer->disconnect(this);
  return true;
}


//-----------------------------------------------------------------------------
void DnDDisplayInteractor::SelectPreviousSlice()
{
  WindowOrientation orientation = m_Viewer->GetOrientation();
  m_Viewer->MoveSlice(orientation, -1, true);
}


//-----------------------------------------------------------------------------
void DnDDisplayInteractor::SelectNextSlice()
{
  WindowOrientation orientation = m_Viewer->GetOrientation();
  m_Viewer->MoveSlice(orientation, +1, true);
}


//-----------------------------------------------------------------------------
void DnDDisplayInteractor::SelectPreviousTimeStep()
{
  int timeStep = m_Viewer->GetTimeStep() - 1;
  if (timeStep < 0)
  {
    timeStep = m_Viewer->GetMaxTimeStep();
  }
  m_Viewer->SetTimeStep(timeStep);
}


//-----------------------------------------------------------------------------
void DnDDisplayInteractor::SelectNextTimeStep()
{
  int timeStep = m_Viewer->GetTimeStep() + 1;
  if (timeStep > m_Viewer->GetMaxTimeStep())
  {
    timeStep = 0;
  }
  m_Viewer->SetTimeStep(timeStep);
}

}