/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $LastChangedDate: 2011-12-16 09:12:58 +0000 (Fri, 16 Dec 2011) $
 Revision          : $Revision: 8039 $
 Last modified by  : $Author: mjc $

 Original author   : $Author: mjc $

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkMIDASStdMultiWidget.h"
#include "QmitkRenderWindow.h"
#include "vtkRenderWindow.h"
#include <QStackedLayout>
#include <QGridLayout>
#include <QFrame>

QmitkMIDASStdMultiWidget::QmitkMIDASStdMultiWidget(
    mitk::RenderingManager* renderingManager,
    mitk::DataStorage* dataStorage,
    QWidget* parent,
    Qt::WindowFlags f
    )
: QmitkStdMultiWidget(parent, f, renderingManager)
, m_GridLayout(NULL)
, m_StackedLayout(NULL)
, m_AxialSliceTag(0)
, m_SagittalSliceTag(0)
, m_CoronalSliceTag(0)
, m_IsSelected(false)
, m_IsEnabled(false)
, m_Display3DViewInOrthoView(false)
, m_Display2DCursorsLocally(true)
, m_Display2DCursorsGlobally(false)
, m_View(MIDAS_VIEW_ORTHO)
{
  // The spec in the header file says these must be non-null.
  assert(renderingManager);
  assert(dataStorage);

  // As soon as the base class is created, de-register the windows with the RenderingManager.
  // This stops a renderer calling m_RenderingManager->RequestUpdateAll when widget is not visible.
  m_RenderingManager = renderingManager;
  m_RenderingManager->RemoveRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
  m_RenderingManager->RemoveRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
  m_RenderingManager->RemoveRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
  m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());

  // Store this immediately, calling base class method.
  this->SetDataStorage(dataStorage);

  // See also SetEnabled(bool) to see things that are dynamically on/off
  this->HideAllWidgetToolbars();
  this->DisableStandardLevelWindow();
  this->DisableDepartmentLogo();
  this->ActivateMenuWidget(false);
  this->SetBackgroundColor(QColor(0, 0, 0));

  // 3D planes should only be visible in this specific widget, not globally, so we create them, then make them globally invisible.
  this->AddDisplayPlaneSubTree();
  this->SetDisplay2DCursorsGlobally(false);
  this->SetDisplay2DCursorsLocally(false);

  // Need each widget to react to Qt drag/drop events.
  this->mitkWidget1->setAcceptDrops(true);
  this->mitkWidget2->setAcceptDrops(true);
  this->mitkWidget3->setAcceptDrops(true);
  this->mitkWidget4->setAcceptDrops(true);

  // Turn off all interactors in base class, until we want them to be enabled.
  this->GetMouseModeSwitcher()->SetInteractionScheme(mitk::MouseModeSwitcher::OFF);

  // Set these off, as it wont matter until there is an image dropped, with a specific layout and orientation.
  this->m_CornerAnnotaions[0].cornerText->SetText(0, "");
  this->m_CornerAnnotaions[1].cornerText->SetText(0, "");
  this->m_CornerAnnotaions[2].cornerText->SetText(0, "");

  // Default to a single view. Due to Mac OpenGL bug, this MUST be ortho to get 3D window up and running.
  this->SetMIDASView(MIDAS_VIEW_ORTHO, NULL);

  // Default to unselected, so borders are off.
  this->SetSelected(false);

  // Need each widget to signal when something is dropped, so connect signals to OnNodesDropped.
  connect(this->mitkWidget1, SIGNAL(NodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)), this, SLOT(OnNodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)));
  connect(this->mitkWidget2, SIGNAL(NodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)), this, SLOT(OnNodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)));
  connect(this->mitkWidget3, SIGNAL(NodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)), this, SLOT(OnNodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)));
  connect(this->mitkWidget4, SIGNAL(NodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)), this, SLOT(OnNodesDropped(QmitkRenderWindow*, std::vector<mitk::DataNode*>)));

  // Register to listen to SliceNavigators, slice changed events.
  itk::ReceptorMemberCommand<QmitkMIDASStdMultiWidget>::Pointer onAxialSliceChangedCommand =
    itk::ReceptorMemberCommand<QmitkMIDASStdMultiWidget>::New();
  onAxialSliceChangedCommand->SetCallbackFunction( this, &QmitkMIDASStdMultiWidget::OnAxialSliceChanged );
  m_AxialSliceTag = mitkWidget1->GetSliceNavigationController()->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), onAxialSliceChangedCommand);

  itk::ReceptorMemberCommand<QmitkMIDASStdMultiWidget>::Pointer onSagittalSliceChangedCommand =
    itk::ReceptorMemberCommand<QmitkMIDASStdMultiWidget>::New();
  onSagittalSliceChangedCommand->SetCallbackFunction( this, &QmitkMIDASStdMultiWidget::OnSagittalSliceChanged );
  m_SagittalSliceTag = mitkWidget2->GetSliceNavigationController()->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), onSagittalSliceChangedCommand);

  itk::ReceptorMemberCommand<QmitkMIDASStdMultiWidget>::Pointer onCoronalSliceChangedCommand =
    itk::ReceptorMemberCommand<QmitkMIDASStdMultiWidget>::New();
  onCoronalSliceChangedCommand->SetCallbackFunction( this, &QmitkMIDASStdMultiWidget::OnCoronalSliceChanged );
  m_CoronalSliceTag = mitkWidget3->GetSliceNavigationController()->AddObserver(mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), onCoronalSliceChangedCommand);

}

QmitkMIDASStdMultiWidget::~QmitkMIDASStdMultiWidget()
{
  if (mitkWidget1 != NULL && m_AxialSliceTag != 0)
  {
    mitkWidget1->GetSliceNavigationController()->RemoveObserver(m_AxialSliceTag);
  }
  if (mitkWidget2 != NULL && m_SagittalSliceTag != 0)
  {
    mitkWidget2->GetSliceNavigationController()->RemoveObserver(m_SagittalSliceTag);
  }
  if (mitkWidget3 != NULL && m_CoronalSliceTag != 0)
  {
    mitkWidget3->GetSliceNavigationController()->RemoveObserver(m_CoronalSliceTag);
  }
}

void QmitkMIDASStdMultiWidget::OnNodesDropped(QmitkRenderWindow *window, std::vector<mitk::DataNode*> nodes)
{
  emit NodesDropped(this, window, nodes);
}

void QmitkMIDASStdMultiWidget::OnAxialSliceChanged(const itk::EventObject & geometrySliceEvent)
{
  this->OnPositionChanged();
}

void QmitkMIDASStdMultiWidget::OnSagittalSliceChanged(const itk::EventObject & geometrySliceEvent)
{
  this->OnPositionChanged();
}

void QmitkMIDASStdMultiWidget::OnCoronalSliceChanged(const itk::EventObject & geometrySliceEvent)
{
  this->OnPositionChanged();
}

void QmitkMIDASStdMultiWidget::OnPositionChanged()
{
  const mitk::Geometry3D *geometry = mitkWidget1->GetSliceNavigationController()->GetInputWorldGeometry();
  if (geometry != NULL)
  {
    mitk::Point3D voxelPoint;
    mitk::Point3D millimetrePoint = this->GetCrossPosition();

    geometry->WorldToIndex(millimetrePoint, voxelPoint);

    emit PositionChanged(voxelPoint, millimetrePoint);
  }
}

void QmitkMIDASStdMultiWidget::SetBackgroundColor(QColor colour)
{
  m_BackgroundColor = colour;
  m_GradientBackground1->SetGradientColors(colour.redF(),colour.greenF(),colour.blueF(),colour.redF(),colour.greenF(),colour.blueF());
  m_GradientBackground1->Enable();
  m_GradientBackground2->SetGradientColors(colour.redF(),colour.greenF(),colour.blueF(),colour.redF(),colour.greenF(),colour.blueF());
  m_GradientBackground2->Enable();
  m_GradientBackground3->SetGradientColors(colour.redF(),colour.greenF(),colour.blueF(),colour.redF(),colour.greenF(),colour.blueF());
  m_GradientBackground3->Enable();
  m_GradientBackground4->SetGradientColors(colour.redF(),colour.greenF(),colour.blueF(),colour.redF(),colour.greenF(),colour.blueF());
  m_GradientBackground4->Enable();

  QPalette palette = this->palette();
  palette.setColor( backgroundRole(), colour );
  this->setAutoFillBackground( true );
  this->setPalette( palette );
}

QColor QmitkMIDASStdMultiWidget::GetBackgroundColor() const
{
  return m_BackgroundColor;
}

void QmitkMIDASStdMultiWidget::SetSelected(bool b)
{
  m_IsSelected = b;
  if (b)
  {
    this->EnableColoredRectangles();
  }
  else
  {
    this->DisableColoredRectangles();
  }
}

bool QmitkMIDASStdMultiWidget::IsSelected() const
{
  return m_IsSelected;
}

void QmitkMIDASStdMultiWidget::SetSelectedWindow(vtkRenderWindow* window)
{
  // When we "Select", the selection is at the level of the QmitkMIDASStdMultiWidget
  // so the whole of this widget is selected. However, we may have clicked in
  // a specific view, so it still helps to highlight the most recently clicked on view.
  // Also, if you are displaying orthoview then you actually have 4 windows present,
  // then highlighting them all starts to look a bit confusing, so we just highlight the
  // most recently focussed window, (eg. axial, sagittal, coronal or 3D).

  if (this->ContainsVtkRenderWindow(window))
  {
    m_IsSelected = true;

    if (window == this->GetRenderWindow1()->GetVtkRenderWindow())
    {
      m_RectangleRendering1->Enable(1.0, 0.0, 0.0);
      m_RectangleRendering2->Disable();
      m_RectangleRendering3->Disable();
      m_RectangleRendering4->Disable();
    }
    else if (window == this->GetRenderWindow2()->GetVtkRenderWindow())
    {
      m_RectangleRendering1->Disable();
      m_RectangleRendering2->Enable(0.0, 1.0, 0.0);
      m_RectangleRendering3->Disable();
      m_RectangleRendering4->Disable();
    }
    else if (window == this->GetRenderWindow3()->GetVtkRenderWindow())
    {
      m_RectangleRendering1->Disable();
      m_RectangleRendering2->Disable();
      m_RectangleRendering3->Enable(0.0, 0.0, 1.0);
      m_RectangleRendering4->Disable();
    }
    else if (window == this->GetRenderWindow4()->GetVtkRenderWindow())
    {
      m_RectangleRendering1->Disable();
      m_RectangleRendering2->Disable();
      m_RectangleRendering3->Disable();
      m_RectangleRendering4->Enable(1.0, 1.0, 0.0);
    }
  }
  else
  {
    this->SetSelected(false);
  }
}

std::vector<QmitkRenderWindow*> QmitkMIDASStdMultiWidget::GetSelectedWindows() const
{

  std::vector<QmitkRenderWindow*> result;

  if (m_RectangleRendering1->IsEnabled())
  {
    result.push_back(this->GetRenderWindow1());
  }
  if (m_RectangleRendering2->IsEnabled())
  {
    result.push_back(this->GetRenderWindow2());
  }
  if (m_RectangleRendering3->IsEnabled())
  {
    result.push_back(this->GetRenderWindow3());
  }
  if (m_RectangleRendering4->IsEnabled())
  {
    result.push_back(this->GetRenderWindow4());
  }
  return result;
}

void QmitkMIDASStdMultiWidget::RequestUpdate()
{
  // The point of all this is to minimise the number of Updates.
  // So, ONLY call RequestUpdate on the specific window that is shown.

  if (this->isVisible() && this->IsEnabled())
  {
    switch(this->m_View)
    {
    case MIDAS_VIEW_AXIAL:
      m_RenderingManager->RequestUpdate(mitkWidget1->GetRenderWindow());
      break;
    case MIDAS_VIEW_SAGITTAL:
      m_RenderingManager->RequestUpdate(mitkWidget2->GetRenderWindow());
      break;
    case MIDAS_VIEW_CORONAL:
      m_RenderingManager->RequestUpdate(mitkWidget3->GetRenderWindow());
      break;
    case MIDAS_VIEW_ORTHO:
      m_RenderingManager->RequestUpdate(mitkWidget1->GetRenderWindow());
      m_RenderingManager->RequestUpdate(mitkWidget2->GetRenderWindow());
      m_RenderingManager->RequestUpdate(mitkWidget3->GetRenderWindow());
      if (this->m_Display3DViewInOrthoView)
      {
        m_RenderingManager->RequestUpdate(mitkWidget4->GetRenderWindow());
      }
      break;
    case MIDAS_VIEW_3D:
      m_RenderingManager->RequestUpdate(mitkWidget4->GetRenderWindow());
      break;
    default:
      // die, this should never happen
      assert(m_View >= 0 && m_View <= 4);
      break;
    }
  }
}

void QmitkMIDASStdMultiWidget::SetEnabled(bool b)
{
  // See also constructor for things that are ALWAYS on/off.

  m_IsEnabled = b;

  bool locked = !m_IsEnabled;
  this->SetWidgetPlanesLocked(locked);
  this->SetWidgetPlanesRotationLocked(locked);

  if (b)
  {
    this->EnableNavigationControllerEventListening();
    this->AddPlanesToDataStorage();
  }
  else
  {
    this->DisableNavigationControllerEventListening();
    this->RemovePlanesFromDataStorage();
  }
}

bool QmitkMIDASStdMultiWidget::IsEnabled() const
{
  return m_IsEnabled;
}

void QmitkMIDASStdMultiWidget::SetDisplay3DViewInOrthoView(bool visible)
{
  m_Display3DViewInOrthoView = visible;
  if (this->m_View == MIDAS_VIEW_ORTHO)
  {
    this->SetMIDASView(MIDAS_VIEW_ORTHO, const_cast<mitk::Geometry3D*>(mitkWidget1->GetSliceNavigationController()->GetInputWorldGeometry()));
  }
  else
  {
    // If we are not in ortho view, then view will get recalculated when user selectes ortho view.
  }
}

bool QmitkMIDASStdMultiWidget::GetDisplay3DViewInOrthoView() const
{
  return m_Display3DViewInOrthoView;
}

void QmitkMIDASStdMultiWidget::SetDisplay2DCursorsLocally(bool visible)
{
  // Here, "locally" means, for this widget, so we are setting Renderer Specific properties.
  m_Display2DCursorsLocally = visible;
  this->SetVisibility(mitkWidget1, m_PlaneNode1, visible);
  this->SetVisibility(mitkWidget1, m_PlaneNode2, visible);
  this->SetVisibility(mitkWidget1, m_PlaneNode3, visible);
  this->SetVisibility(mitkWidget2, m_PlaneNode1, visible);
  this->SetVisibility(mitkWidget2, m_PlaneNode2, visible);
  this->SetVisibility(mitkWidget2, m_PlaneNode3, visible);
  this->SetVisibility(mitkWidget3, m_PlaneNode1, visible);
  this->SetVisibility(mitkWidget3, m_PlaneNode2, visible);
  this->SetVisibility(mitkWidget3, m_PlaneNode3, visible);

  this->SetVisibility(mitkWidget4, m_PlaneNode1, true);
  this->SetVisibility(mitkWidget4, m_PlaneNode2, true);
  this->SetVisibility(mitkWidget4, m_PlaneNode3, true);
}

bool QmitkMIDASStdMultiWidget::GetDisplay2DCursorsLocally() const
{
  return m_Display2DCursorsLocally;
}

void QmitkMIDASStdMultiWidget::SetDisplay2DCursorsGlobally(bool visible)
{
  // Here, "globally" means the plane nodes created within this widget will be available in ALL other render windows.
  m_Display2DCursorsGlobally = visible;
  m_PlaneNode1->SetVisibility(visible);
  m_PlaneNode1->Modified();
  m_PlaneNode2->SetVisibility(visible);
  m_PlaneNode2->Modified();
  m_PlaneNode3->SetVisibility(visible);
  m_PlaneNode3->Modified();
}

bool QmitkMIDASStdMultiWidget::GetDisplay2DCursorsGlobally() const
{
  return m_Display2DCursorsGlobally;
}

void QmitkMIDASStdMultiWidget::SetVisibility(QmitkRenderWindow *window, mitk::DataNode *node, bool visible)
{
  if (window != NULL && node != NULL)
  {
    vtkRenderWindow *vtkRenderWindow = window->GetVtkRenderWindow();
    if (vtkRenderWindow != NULL)
    {
      mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(vtkRenderWindow);
      if (renderer != NULL)
      {
        bool currentVisibility = false;
        node->GetVisibility(currentVisibility, renderer);

        if (visible != currentVisibility)
        {
          node->SetVisibility(visible, renderer);
        }
      }
    }
  }
}

void QmitkMIDASStdMultiWidget::SetRendererSpecificVisibility(std::vector<mitk::DataNode*> nodes, bool visible)
{
  for (unsigned int i = 0; i < nodes.size(); i++)
  {
    this->SetVisibility(mitkWidget1, nodes[i], visible);
    this->SetVisibility(mitkWidget2, nodes[i], visible);
    this->SetVisibility(mitkWidget3, nodes[i], visible);
    this->SetVisibility(mitkWidget4, nodes[i], visible);
  }
}

bool QmitkMIDASStdMultiWidget::ContainsWindow(QmitkRenderWindow *window) const
{
  bool result = false;
  if (   mitkWidget1 == window
      || mitkWidget2 == window
      || mitkWidget3 == window
      || mitkWidget4 == window
      )
  {
    result = true;
  }
  return result;
}

bool QmitkMIDASStdMultiWidget::ContainsVtkRenderWindow(vtkRenderWindow *window) const
{
  bool result = false;
  if (   mitkWidget1->GetVtkRenderWindow() == window
      || mitkWidget2->GetVtkRenderWindow() == window
      || mitkWidget3->GetVtkRenderWindow() == window
      || mitkWidget4->GetVtkRenderWindow() == window
      )
  {
    result = true;
  }
  return result;
}

std::vector<QmitkRenderWindow*> QmitkMIDASStdMultiWidget::GetAllWindows() const
{
  std::vector<QmitkRenderWindow*> result;
  result.push_back(this->GetRenderWindow1());
  result.push_back(this->GetRenderWindow2());
  result.push_back(this->GetRenderWindow3());
  result.push_back(this->GetRenderWindow4());
  return result;
}

std::vector<vtkRenderWindow*> QmitkMIDASStdMultiWidget::GetAllVtkWindows() const
{
  std::vector<vtkRenderWindow*> result;
  result.push_back(this->GetRenderWindow1()->GetVtkRenderWindow());
  result.push_back(this->GetRenderWindow2()->GetVtkRenderWindow());
  result.push_back(this->GetRenderWindow3()->GetVtkRenderWindow());
  result.push_back(this->GetRenderWindow4()->GetVtkRenderWindow());
  return result;
}

MIDASOrientation QmitkMIDASStdMultiWidget::GetOrientation()
{
  MIDASOrientation result = MIDAS_ORIENTATION_UNKNOWN;
  if (m_RectangleRendering1->IsEnabled()
      && !m_RectangleRendering2->IsEnabled()
      && !m_RectangleRendering3->IsEnabled()
      && !m_RectangleRendering4->IsEnabled()
      )
  {
    result = MIDAS_ORIENTATION_AXIAL;
  }
  else if (!m_RectangleRendering1->IsEnabled()
      && m_RectangleRendering2->IsEnabled()
      && !m_RectangleRendering3->IsEnabled()
      && !m_RectangleRendering4->IsEnabled()
      )
  {
    result = MIDAS_ORIENTATION_SAGITTAL;
  }
  else if (!m_RectangleRendering1->IsEnabled()
      && !m_RectangleRendering2->IsEnabled()
      && m_RectangleRendering3->IsEnabled()
      && !m_RectangleRendering4->IsEnabled()
      )
  {
    result = MIDAS_ORIENTATION_CORONAL;
  }
  return result;
}

void QmitkMIDASStdMultiWidget::SetMIDASView(MIDASView view, mitk::Geometry3D *geometry)
{
  // Add these the first time we have a real geometry.
  if (geometry != NULL)
  {
    this->m_CornerAnnotaions[0].cornerText->SetText(0, "Axial");
    this->m_CornerAnnotaions[1].cornerText->SetText(0, "Sagittal");
    this->m_CornerAnnotaions[2].cornerText->SetText(0, "Coronal");
  }

  // If this is not the first time we are in SetMIDASView, we need to remove vtkRenderWindows from RenderingManagers.
  std::vector< vtkRenderWindow* > tmpWindows;
  std::vector< vtkRenderWindow* > registeredWindows = m_RenderingManager->GetAllRegisteredRenderWindows();

  if (registeredWindows.size() > 0 && this->m_GridLayout != NULL && this->m_StackedLayout != NULL)
  {
    switch(this->m_View)
    {
    case MIDAS_VIEW_AXIAL:
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
      break;
    case MIDAS_VIEW_SAGITTAL:
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
      break;
    case MIDAS_VIEW_CORONAL:
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
      break;
    case MIDAS_VIEW_ORTHO:
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
      tmpWindows = m_RenderingManager->GetAllRegisteredRenderWindows();
      if (tmpWindows.size() > 0)
      {
        m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
      }
      break;
    case MIDAS_VIEW_3D:
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
      break;
    default:
      // die, this should never happen
      assert(m_View >= 0 && m_View <= 4);
      break;
    }
  }

  // Now destroy layouts (in reverse order obviously).
  if (m_StackedLayout != NULL)
  {
    delete m_StackedLayout;
  }
  if (m_GridLayout != NULL)
  {
    delete m_GridLayout;
  }
  if (QmitkStdMultiWidgetLayout != NULL)
  {
    delete QmitkStdMultiWidgetLayout;
  }

  // These in base class are not needed.
  m_MainSplit->setVisible(false);
  m_LayoutSplit->setVisible(false);
  m_SubSplit1->setVisible(false);
  m_SubSplit2->setVisible(false);

  // Rebuild layout, explicitly setting all the borders to zero.
  m_StackedLayout = new QStackedLayout();
  m_StackedLayout->setContentsMargins(0, 0, 0, 0);
  m_StackedLayout->setSpacing(0);
  m_StackedLayout->setStackingMode(QStackedLayout::StackAll);

  m_GridLayout = new QGridLayout();
  m_GridLayout->setContentsMargins(0, 0, 0, 0);
  m_GridLayout->setSpacing(0);

  QmitkStdMultiWidgetLayout =  new QHBoxLayout( this );
  QmitkStdMultiWidgetLayout->setContentsMargins(0, 0, 0, 0);
  QmitkStdMultiWidgetLayout->setSpacing(0);

  m_GridLayout->addLayout(m_StackedLayout, 0, 0);
  QmitkStdMultiWidgetLayout->addLayout(m_GridLayout);

  this->mitkWidget1Container->show();
  this->mitkWidget2Container->show();
  this->mitkWidget3Container->show();
  this->mitkWidget4Container->show();

  switch (view)
  {
  case MIDAS_VIEW_AXIAL:

    m_StackedLayout->addWidget(this->mitkWidget4Container);
    m_StackedLayout->addWidget(this->mitkWidget1Container);
    m_GridLayout->addWidget(this->mitkWidget2Container, 0, 1);
    m_GridLayout->addWidget(this->mitkWidget3Container, 1, 0);
    break;

  case MIDAS_VIEW_SAGITTAL:

    m_StackedLayout->addWidget(this->mitkWidget4Container);
    m_StackedLayout->addWidget(this->mitkWidget2Container);
    m_GridLayout->addWidget(this->mitkWidget1Container, 0, 1);
    m_GridLayout->addWidget(this->mitkWidget3Container, 1, 0);
    break;

  case MIDAS_VIEW_CORONAL:

    m_StackedLayout->addWidget(this->mitkWidget4Container);
    m_StackedLayout->addWidget(this->mitkWidget3Container);
    m_GridLayout->addWidget(this->mitkWidget1Container, 0, 1);
    m_GridLayout->addWidget(this->mitkWidget2Container, 1, 0);
    break;

  case MIDAS_VIEW_ORTHO:

    if (m_Display3DViewInOrthoView)
    {
      m_GridLayout->addWidget(mitkWidget4Container, 1, 1);
    }
    else
    {
      m_StackedLayout->addWidget(this->mitkWidget4Container);
    }
    m_StackedLayout->addWidget(this->mitkWidget1Container);
    m_GridLayout->addWidget(mitkWidget2Container, 0, 1);
    m_GridLayout->addWidget(mitkWidget3Container, 1, 0);
    break;

  case MIDAS_VIEW_3D:

    m_StackedLayout->addWidget(this->mitkWidget4Container);
    m_GridLayout->addWidget(this->mitkWidget1Container, 0, 1);
    m_GridLayout->addWidget(this->mitkWidget2Container, 1, 0);
    m_GridLayout->addWidget(this->mitkWidget3Container, 1, 1);
    break;

  default:
    // die, this should never happen
    assert(view >= 0 && view <= 4);
    break;
  }

  // Now add all 4 windows to the RenderingManager, so we can initialize geometry.
  // In addition, we need the size of each window to be "correct" so that the geometry calculations work.
  // This means the bit where we add the 4 windows to the RenderingManager, must come after we have all 4 windows added to a Layout.

  if (geometry != NULL)
  {
    m_RenderingManager->AddRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
    m_RenderingManager->AddRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
    m_RenderingManager->AddRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
    m_RenderingManager->AddRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
    m_RenderingManager->InitializeViews(geometry);
  }

  switch (view)
  {
  case MIDAS_VIEW_AXIAL:
    if (geometry != NULL)
    {
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
      this->mitkWidget2Container->hide();
      this->mitkWidget3Container->hide();
      this->mitkWidget4Container->hide();
    }
    else
    {
      m_RenderingManager->AddRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
    }
    break;
  case MIDAS_VIEW_SAGITTAL:
    if (geometry != NULL)
    {
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
      this->mitkWidget1Container->hide();
      this->mitkWidget3Container->hide();
      this->mitkWidget4Container->hide();
    }
    else
    {
      m_RenderingManager->AddRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
    }
    break;
  case MIDAS_VIEW_CORONAL:
    if (geometry != NULL)
    {
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
      this->mitkWidget1Container->hide();
      this->mitkWidget2Container->hide();
      this->mitkWidget4Container->hide();
    }
    else
    {
      m_RenderingManager->AddRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
    }
    break;
  case MIDAS_VIEW_ORTHO:
    if (m_Display3DViewInOrthoView)
    {
      if (geometry == NULL)
      {
        m_RenderingManager->AddRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
        m_RenderingManager->AddRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
        m_RenderingManager->AddRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
        m_RenderingManager->AddRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
      }
    }
    else
    {
      if (geometry != NULL)
      {
        m_RenderingManager->RemoveRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
        this->mitkWidget4Container->hide();
      }
      else
      {
        m_RenderingManager->AddRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
        m_RenderingManager->AddRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
        m_RenderingManager->AddRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
      }
    }
    break;
  case MIDAS_VIEW_3D:
    if (geometry != NULL)
    {
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget1->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget2->GetVtkRenderWindow());
      m_RenderingManager->RemoveRenderWindow(this->mitkWidget3->GetVtkRenderWindow());
      this->mitkWidget1Container->hide();
      this->mitkWidget2Container->hide();
      this->mitkWidget3Container->hide();
    }
    else
    {
      m_RenderingManager->AddRenderWindow(this->mitkWidget4->GetVtkRenderWindow());
    }
    break;
  default:
    // die, this should never happen
    assert(view >= 0 && view <= 4);
    break;
  }
  m_View = view;
}

MIDASView QmitkMIDASStdMultiWidget::GetMIDASView() const
{
  return m_View;
}

bool QmitkMIDASStdMultiWidget::IsSingle2DView() const
{
  bool result = false;
  if (m_View == MIDAS_VIEW_AXIAL || m_View == MIDAS_VIEW_SAGITTAL || m_View == MIDAS_VIEW_CORONAL)
  {
    result = true;
  }
  return result;
}

mitk::SliceNavigationController::Pointer QmitkMIDASStdMultiWidget::GetSliceNavigationController(MIDASOrientation orientation) const
{
  mitk::SliceNavigationController::Pointer result = NULL;
  if (orientation == MIDAS_ORIENTATION_AXIAL)
  {
    result = mitkWidget1->GetSliceNavigationController();
  }
  else if (orientation == MIDAS_ORIENTATION_SAGITTAL)
  {
    result = mitkWidget2->GetSliceNavigationController();
  }
  else if (orientation == MIDAS_ORIENTATION_CORONAL)
  {
    result = mitkWidget3->GetSliceNavigationController();
  }
  return result;
}

unsigned int QmitkMIDASStdMultiWidget::GetMinSlice(MIDASOrientation orientation) const
{
  return 0;
}

unsigned int QmitkMIDASStdMultiWidget::GetMaxSlice(MIDASOrientation orientation) const
{
  unsigned int result = 0;

  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController(orientation);
  assert(snc);

  if (snc->GetSlice() != NULL)
  {
    if (snc->GetSlice()->GetSteps() >= 1)
    {
      result = snc->GetSlice()->GetSteps() -1;
    }
  }
  return result;
}

unsigned int QmitkMIDASStdMultiWidget::GetMinTime() const
{
  return 0;
}

unsigned int QmitkMIDASStdMultiWidget::GetMaxTime() const
{
  unsigned int result = 0;

  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController(MIDAS_ORIENTATION_AXIAL);
  assert(snc);

  if (snc->GetTime() != NULL)
  {
    if (snc->GetTime()->GetSteps() >= 1)
    {
      result = snc->GetTime()->GetSteps() -1;
    }
  }
  return result;
}

void QmitkMIDASStdMultiWidget::SetSliceNumber(MIDASOrientation orientation, unsigned int sliceNumber)
{
  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController(orientation);
  assert(snc);

  snc->GetSlice()->SetPos(sliceNumber);
}

unsigned int QmitkMIDASStdMultiWidget::GetSliceNumber(MIDASOrientation orientation) const
{
  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController(orientation);
  assert(snc);

  return snc->GetSlice()->GetPos();
}

void QmitkMIDASStdMultiWidget::SetTime(unsigned int timeSlice)
{
  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController(MIDAS_ORIENTATION_AXIAL);
  assert(snc);

  snc->GetTime()->SetPos(timeSlice);
}

unsigned int QmitkMIDASStdMultiWidget::GetTime() const
{
  mitk::SliceNavigationController::Pointer snc = this->GetSliceNavigationController(MIDAS_ORIENTATION_AXIAL);
  assert(snc);

  return snc->GetTime()->GetPos();
}
