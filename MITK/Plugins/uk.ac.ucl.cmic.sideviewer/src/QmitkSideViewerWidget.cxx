/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkSideViewerWidget.h"

#include "niftkBaseView.h"

#include <berryIWorkbenchPage.h>

#include <itkCommand.h>

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkFocusManager.h>
#include <mitkBaseGeometry.h>
#include <mitkGlobalInteraction.h>
#include <mitkIRenderWindowPart.h>
#include <mitkRenderingManager.h>
#include <mitkSliceNavigationController.h>

#include <QmitkRenderWindow.h>

#include <niftkSingleViewerWidget.h>

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpacerItem>
#include <QSpinBox>
#include <QVBoxLayout>


namespace niftk
{

class EditorLifeCycleListener : public berry::IPartListener
{
  berryObjectMacro(EditorLifeCycleListener)

public:

  EditorLifeCycleListener(QmitkSideViewerWidget* sideViewerWidget)
  : m_SideViewerWidget(sideViewerWidget)
  {
  }

private:

  Events::Types GetPartEventTypes() const override
  {
    return Events::VISIBLE;
  }

  void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override
  {
    berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

    if (mitk::IRenderWindowPart* renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
    {
      mitk::BaseRenderer* focusedRenderer = mitk::GlobalInteraction::GetInstance()->GetFocus();

      bool found = false;
      /// Note:
      /// We need to look for the focused window among every window of the editor.
      /// The MITK Display has not got the concept of 'selected' window and always
      /// returns the axial window as 'active'. Therefore we cannot use GetActiveQmitkRenderWindow.
      foreach (QmitkRenderWindow* mainWindow, renderWindowPart->GetQmitkRenderWindows().values())
      {
        if (focusedRenderer == mainWindow->GetRenderer())
        {
          m_SideViewerWidget->OnMainWindowChanged(renderWindowPart, mainWindow);
          found = true;
          break;
        }
      }

      if (!found)
      {
        QmitkRenderWindow* mainWindow = renderWindowPart->GetActiveQmitkRenderWindow();
        if (mainWindow && mainWindow->isVisible())
        {
          m_SideViewerWidget->OnMainWindowChanged(renderWindowPart, mainWindow);
        }
      }
    }
  }

  QmitkSideViewerWidget* m_SideViewerWidget;

};


//-----------------------------------------------------------------------------
QmitkSideViewerWidget::QmitkSideViewerWidget(BaseView* view, QWidget* parent, mitk::RenderingManager* renderingManager)
: QWidget(parent)
, m_ContainingView(view)
, m_FocusManagerObserverTag(0)
, m_WindowLayout(WINDOW_LAYOUT_UNKNOWN)
, m_MainRenderingManager(0)
, m_MainWindow(0)
, m_MainAxialWindow(0)
, m_MainSagittalWindow(0)
, m_MainCoronalWindow(0)
, m_MainWindowSnc(0)
, m_MainAxialSnc(0)
, m_MainSagittalSnc(0)
, m_MainCoronalSnc(0)
, m_VisibilityTracker(0)
, m_Magnification(0.0)
, m_MainWindowOrientation(WINDOW_ORIENTATION_UNKNOWN)
, m_SingleWindowLayouts()
, m_ToolNodeNameFilter(0)
, m_TimeGeometry(0)
, m_EditorLifeCycleListener(new EditorLifeCycleListener(this))
, m_RenderingManager(renderingManager)
{
  this->SetupUi(parent);

  m_Viewer->SetShow3DWindowIn2x2WindowLayout(false);
  m_Viewer->SetCursorVisible(true);
  m_Viewer->SetRememberSettingsPerWindowLayout(false);
  m_Viewer->SetDisplayInteractionsEnabled(true);
  m_Viewer->SetCursorPositionBinding(false);
  m_Viewer->SetScaleFactorBinding(true);

  m_CoronalWindowRadioButton->setChecked(true);

  m_SingleWindowLayouts[WINDOW_ORIENTATION_AXIAL] = WINDOW_LAYOUT_CORONAL;
  m_SingleWindowLayouts[WINDOW_ORIENTATION_SAGITTAL] = WINDOW_LAYOUT_CORONAL;
  m_SingleWindowLayouts[WINDOW_ORIENTATION_CORONAL] = WINDOW_LAYOUT_SAGITTAL;

  m_MagnificationSpinBox->setDecimals(2);
  m_MagnificationSpinBox->setSingleStep(1.0);

  double minMagnification = std::ceil(m_Viewer->GetMinMagnification());
  double maxMagnification = std::floor(m_Viewer->GetMaxMagnification());

  m_MagnificationSpinBox->setMinimum(minMagnification);
  m_MagnificationSpinBox->setMaximum(maxMagnification);

  m_ControlsWidget->setEnabled(false);

  this->connect(m_AxialWindowRadioButton, SIGNAL(toggled(bool)), SLOT(OnAxialWindowRadioButtonToggled(bool)));
  this->connect(m_SagittalWindowRadioButton, SIGNAL(toggled(bool)), SLOT(OnSagittalWindowRadioButtonToggled(bool)));
  this->connect(m_CoronalWindowRadioButton, SIGNAL(toggled(bool)), SLOT(OnCoronalWindowRadioButtonToggled(bool)));
  this->connect(m_MultiWindowRadioButton, SIGNAL(toggled(bool)), SLOT(OnMultiWindowRadioButtonToggled(bool)));
  this->connect(m_MultiWindowComboBox, SIGNAL(currentIndexChanged(int)), SLOT(OnMultiWindowComboBoxIndexChanged()));

  this->connect(m_Viewer, SIGNAL(WindowLayoutChanged(WindowLayout)), SLOT(OnWindowLayoutChanged(WindowLayout)));

  this->connect(m_SliceSpinBox, SIGNAL(valueChanged(int)), SLOT(OnSliceSpinBoxValueChanged(int)));
  this->connect(m_Viewer, SIGNAL(SelectedPositionChanged(const mitk::Point3D&)), SLOT(OnSelectedPositionChanged(const mitk::Point3D&)));
  this->connect(m_MagnificationSpinBox, SIGNAL(valueChanged(double)), SLOT(OnMagnificationSpinBoxValueChanged(double)));
  this->connect(m_Viewer, SIGNAL(ScaleFactorChanged(WindowOrientation, double)), SLOT(OnScaleFactorChanged(WindowOrientation, double)));

  mitk::DataStorage::Pointer dataStorage = view->GetDataStorage();

  /// Note:
  /// We do not set the data storage for the viewer because that would recalculate
  /// the time geometry based on the globally visible data nodes in the data storage.
  /// This is wrong for us, since we want to see the nodes that are visible in the
  /// tracked window, and hence we want to set the time geometry of the track window
  /// to the viewer of this widget.
  /// The viewer gets the data storage from the rendering manager that is set through
  /// its constructor.

  std::vector<const mitk::BaseRenderer*> renderers;
  renderers.push_back(m_Viewer->GetAxialWindow()->GetRenderer());
  renderers.push_back(m_Viewer->GetSagittalWindow()->GetRenderer());
  renderers.push_back(m_Viewer->GetCoronalWindow()->GetRenderer());

  /// TODO Very ugly. This should be done in the other way round, from the MIDAS tools.
//    niftk::ToolWorkingDataNameFilter::Pointer filter = niftk::ToolWorkingDataNameFilter::New();

  m_ToolNodeNameFilter = DataNodeStringPropertyFilter::New();
  m_ToolNodeNameFilter->SetPropertyName("name");
  m_ToolNodeNameFilter->AddToList("One of FeedbackContourTool's feedback nodes");
  m_ToolNodeNameFilter->AddToList("MIDAS Background Contour");
  m_ToolNodeNameFilter->AddToList("MIDAS_SEEDS");
  m_ToolNodeNameFilter->AddToList("MIDAS_CURRENT_CONTOURS");
  m_ToolNodeNameFilter->AddToList("MIDAS_REGION_GROWING_IMAGE");
  m_ToolNodeNameFilter->AddToList("MIDAS_PRIOR_CONTOURS");
  m_ToolNodeNameFilter->AddToList("MIDAS_NEXT_CONTOURS");
  m_ToolNodeNameFilter->AddToList("MIDAS_DRAW_CONTOURS");
  m_ToolNodeNameFilter->AddToList("MORPH_EDITS_EROSIONS_SUBTRACTIONS");
  m_ToolNodeNameFilter->AddToList("MORPH_EDITS_EROSIONS_ADDITIONS");
  m_ToolNodeNameFilter->AddToList("MORPH_EDITS_DILATIONS_SUBTRACTIONS");
  m_ToolNodeNameFilter->AddToList("MORPH_EDITS_DILATIONS_ADDITIONS");
  m_ToolNodeNameFilter->AddToList("MORPHO_SEGMENTATION_OF_LAST_STAGE");
  m_ToolNodeNameFilter->AddToList("PolyTool anchor points");
  m_ToolNodeNameFilter->AddToList("PolyTool previous contour");
  m_ToolNodeNameFilter->AddToList("Paintbrush_Node");

  m_VisibilityTracker = DataNodeVisibilityTracker::New(dataStorage);
  m_VisibilityTracker->SetNodesToIgnore(m_Viewer->GetWidgetPlanes());
  m_VisibilityTracker->SetManagedRenderers(renderers);
  m_VisibilityTracker->AddFilter(m_ToolNodeNameFilter.GetPointer());

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();

  mitk::IRenderWindowPart* selectedEditor = this->GetSelectedEditor();
  if (selectedEditor)
  {
    bool found = false;

    mitk::BaseRenderer* focusedRenderer = focusManager->GetFocused();

    /// Note:
    /// We need to look for the focused window among every window of the editor.
    /// The MITK Display has not got the concept of 'selected' window and always
    /// returns the axial window as 'active'. Therefore we cannot use GetActiveQmitkRenderWindow.
    foreach (QmitkRenderWindow* mainWindow, selectedEditor->GetQmitkRenderWindows().values())
    {
      if (focusedRenderer == mainWindow->GetRenderer())
      {
        this->OnMainWindowChanged(selectedEditor, mainWindow);
        found = true;
        break;
      }
    }

    if (!found)
    {
      QmitkRenderWindow* mainWindow = selectedEditor->GetActiveQmitkRenderWindow();
      if (mainWindow && mainWindow->isVisible())
      {
        this->OnMainWindowChanged(selectedEditor, mainWindow);
      }
    }
  }

  // Register focus observer.
  if (focusManager)
  {
    itk::SimpleMemberCommand<QmitkSideViewerWidget>::Pointer onFocusChangedCommand =
      itk::SimpleMemberCommand<QmitkSideViewerWidget>::New();
    onFocusChangedCommand->SetCallbackFunction( this, &QmitkSideViewerWidget::OnFocusChanged );

    m_FocusManagerObserverTag = focusManager->AddObserver(mitk::FocusEvent(), onFocusChangedCommand);
  }

  m_ContainingView->GetSite()->GetPage()->AddPartListener(m_EditorLifeCycleListener.data());

  /// Note:
  /// Direct call to m_Viewer->FitToDisplay() does not work because the function
  /// computes the desired scale factor for the visible render windows. At this time,
  /// however, no renderer window is visible, since the widget is just being constructed.
  QTimer::singleShot(0, this, SLOT(FitToDisplay()));
}


//-----------------------------------------------------------------------------
QmitkSideViewerWidget::~QmitkSideViewerWidget()
{
  if (m_MainRenderingManager)
  {
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetAxialWindow()->GetRenderWindow());
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetSagittalWindow()->GetRenderWindow());
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetCoronalWindow()->GetRenderWindow());
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->Get3DWindow()->GetRenderWindow());
    m_MainRenderingManager = 0;
  }

  m_ContainingView->GetSite()->GetPage()->RemovePartListener(m_EditorLifeCycleListener.data());

  // Deregister focus observer.
  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  if (focusManager)
  {
    focusManager->RemoveObserver(m_FocusManagerObserverTag);
  }

  m_VisibilityTracker->SetTrackedRenderer(0);
  m_VisibilityTracker = 0;

  m_Viewer->SetEnabled(false);

  if (m_MainWindow)
  {
    m_MainWindowSnc->Disconnect(this);
  }
  if (m_MainAxialWindow)
  {
    m_MainAxialSnc->Disconnect(m_Viewer->GetAxialWindow()->GetSliceNavigationController());
    m_Viewer->GetAxialWindow()->GetSliceNavigationController()->Disconnect(m_MainAxialSnc);
  }
  if (m_MainSagittalWindow)
  {
    m_MainSagittalSnc->Disconnect(m_Viewer->GetSagittalWindow()->GetSliceNavigationController());
    m_Viewer->GetSagittalWindow()->GetSliceNavigationController()->Disconnect(m_MainSagittalSnc);
  }
  if (m_MainCoronalWindow)
  {
    m_MainCoronalSnc->Disconnect(m_Viewer->GetCoronalWindow()->GetSliceNavigationController());
    m_Viewer->GetCoronalWindow()->GetSliceNavigationController()->Disconnect(m_MainCoronalSnc);
  }
}


void QmitkSideViewerWidget::SetFocused()
{
  m_Viewer->setFocus();
}


void QmitkSideViewerWidget::SetupUi(QWidget *parent)
{
    QVBoxLayout* verticalLayout = new QVBoxLayout(parent);
    verticalLayout->setSpacing(3);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    m_Viewer = new niftkSingleViewerWidget(parent, m_RenderingManager, "side viewer");

    verticalLayout->addWidget(m_Viewer);

    m_ControlsWidget = new QWidget(parent);
    QHBoxLayout* horizontalLayout_2 = new QHBoxLayout(m_ControlsWidget);
    horizontalLayout_2->setSpacing(0);
    horizontalLayout_2->setContentsMargins(3, 0, 0, 3);
    m_LayoutWidget = new QWidget(m_ControlsWidget);
    QHBoxLayout* horizontalLayout = new QHBoxLayout(m_LayoutWidget);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    m_AxialWindowRadioButton = new QRadioButton(m_LayoutWidget);
    m_AxialWindowRadioButton->setText("ax");

    horizontalLayout->addWidget(m_AxialWindowRadioButton);

    m_SagittalWindowRadioButton = new QRadioButton(m_LayoutWidget);
    m_SagittalWindowRadioButton->setText("sag");

    horizontalLayout->addWidget(m_SagittalWindowRadioButton);
    m_CoronalWindowRadioButton = new QRadioButton(m_LayoutWidget);
    m_CoronalWindowRadioButton->setText("cor");

    horizontalLayout->addWidget(m_CoronalWindowRadioButton);

    m_MultiWindowRadioButton = new QRadioButton(m_LayoutWidget);

    horizontalLayout->addWidget(m_MultiWindowRadioButton);

    m_MultiWindowComboBox = new QComboBox(m_LayoutWidget);
    m_MultiWindowComboBox->addItem("2H");
    m_MultiWindowComboBox->addItem("2V");
    m_MultiWindowComboBox->setMaximumSize(QSize(52, 16777215));

    horizontalLayout->addWidget(m_MultiWindowComboBox);

    horizontalLayout_2->addWidget(m_LayoutWidget);

    m_SliceLabel = new QLabel(m_ControlsWidget);
    m_SliceLabel->setText("slice:");

    horizontalLayout_2->addWidget(m_SliceLabel);

    m_SliceSpinBox = new QSpinBox(m_ControlsWidget);
    m_SliceSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_SliceSpinBox->setMaximum(999);

    horizontalLayout_2->addWidget(m_SliceSpinBox);

    m_MagnificationLabel = new QLabel(m_ControlsWidget);
    m_MagnificationLabel->setText("magn.:");

    horizontalLayout_2->addWidget(m_MagnificationLabel);

    m_MagnificationSpinBox = new QDoubleSpinBox(m_ControlsWidget);
    m_MagnificationSpinBox->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    horizontalLayout_2->addWidget(m_MagnificationSpinBox);

    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout_2->addItem(horizontalSpacer);

    verticalLayout->addWidget(m_ControlsWidget);

    verticalLayout->setStretch(0, 1);
}

//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnAMainWindowDestroyed(QObject* mainWindow)
{
  if (m_MainRenderingManager)
  {
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetAxialWindow()->GetRenderWindow());
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetSagittalWindow()->GetRenderWindow());
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetCoronalWindow()->GetRenderWindow());
    m_MainRenderingManager->RemoveRenderWindow(m_Viewer->Get3DWindow()->GetRenderWindow());
    m_MainRenderingManager = 0;
  }

  if (mainWindow == m_MainWindow)
  {
    m_VisibilityTracker->SetTrackedRenderer(0);
    m_Viewer->SetEnabled(false);
    m_MainWindow = 0;
    m_MainWindowSnc = 0;
  }

  if (mainWindow == m_MainAxialWindow)
  {
    m_Viewer->GetAxialWindow()->GetSliceNavigationController()->Disconnect(m_MainAxialSnc);
    m_MainAxialWindow = 0;
    m_MainAxialSnc = 0;
  }
  else if (mainWindow == m_MainSagittalWindow)
  {
    m_Viewer->GetSagittalWindow()->GetSliceNavigationController()->Disconnect(m_MainSagittalSnc);
    m_MainSagittalWindow = 0;
    m_MainSagittalSnc = 0;
  }
  else if (mainWindow == m_MainCoronalWindow)
  {
    m_Viewer->GetCoronalWindow()->GetSliceNavigationController()->Disconnect(m_MainCoronalSnc);
    m_MainCoronalWindow = 0;
    m_MainCoronalSnc = 0;
  }
  else
  {
    /// This should not happen.
    assert(false);
  }

  m_Viewer->RequestUpdate();
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::FitToDisplay()
{
  m_Viewer->FitToDisplay();
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnAxialWindowRadioButtonToggled(bool checked)
{
  if (checked)
  {
    m_SingleWindowLayouts[m_MainWindowOrientation] = WINDOW_LAYOUT_AXIAL;
    m_Viewer->SetWindowLayout(WINDOW_LAYOUT_AXIAL);
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnSagittalWindowRadioButtonToggled(bool checked)
{
  if (checked)
  {
    m_SingleWindowLayouts[m_MainWindowOrientation] = WINDOW_LAYOUT_SAGITTAL;
    m_Viewer->SetWindowLayout(WINDOW_LAYOUT_SAGITTAL);
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnCoronalWindowRadioButtonToggled(bool checked)
{
  if (checked)
  {
    m_SingleWindowLayouts[m_MainWindowOrientation] = WINDOW_LAYOUT_CORONAL;
    m_Viewer->SetWindowLayout(WINDOW_LAYOUT_CORONAL);
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnMultiWindowRadioButtonToggled(bool checked)
{
  if (checked)
  {
    WindowLayout windowLayout = this->GetMultiWindowLayoutForOrientation(m_MainWindowOrientation);
    m_Viewer->SetWindowLayout(windowLayout);
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnMultiWindowComboBoxIndexChanged()
{
  if (!m_MultiWindowRadioButton->isChecked())
  {
    bool wasBlocked = m_MultiWindowRadioButton->blockSignals(true);
    m_MultiWindowRadioButton->setChecked(true);
    m_MultiWindowRadioButton->blockSignals(wasBlocked);
  }

  WindowLayout windowLayout = this->GetMultiWindowLayoutForOrientation(m_MainWindowOrientation);
  m_Viewer->SetWindowLayout(windowLayout);
}


//-----------------------------------------------------------------------------
WindowLayout QmitkSideViewerWidget::GetMultiWindowLayoutForOrientation(WindowOrientation mainWindowOrientation)
{
  WindowLayout windowLayout = WINDOW_LAYOUT_UNKNOWN;

  // 2H
  if (m_MultiWindowComboBox->currentIndex() == 0)
  {
    if (mainWindowOrientation == WINDOW_ORIENTATION_AXIAL)
    {
      windowLayout = WINDOW_LAYOUT_COR_SAG_H;
    }
    else if (mainWindowOrientation == WINDOW_ORIENTATION_SAGITTAL)
    {
      windowLayout = WINDOW_LAYOUT_COR_AX_H;
    }
    else if (mainWindowOrientation == WINDOW_ORIENTATION_CORONAL)
    {
      windowLayout = WINDOW_LAYOUT_SAG_AX_H;
    }
  }
  // 2V
  else if (m_MultiWindowComboBox->currentIndex() == 1)
  {
    if (mainWindowOrientation == WINDOW_ORIENTATION_AXIAL)
    {
      windowLayout = WINDOW_LAYOUT_COR_SAG_V;
    }
    else if (mainWindowOrientation == WINDOW_ORIENTATION_SAGITTAL)
    {
      windowLayout = WINDOW_LAYOUT_COR_AX_V;
    }
    else if (mainWindowOrientation == WINDOW_ORIENTATION_CORONAL)
    {
      windowLayout = WINDOW_LAYOUT_SAG_AX_V;
    }
  }

  return windowLayout;
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnMainWindowOrientationChanged(WindowOrientation mainWindowOrientation)
{
  WindowLayout windowLayout = WINDOW_LAYOUT_UNKNOWN;

  bool wasBlocked = m_LayoutWidget->blockSignals(true);

  WindowLayout defaultMultiWindowLayout = this->GetMultiWindowLayoutForOrientation(mainWindowOrientation);

  if (!m_MultiWindowRadioButton->isChecked())
  {
    windowLayout = m_SingleWindowLayouts[mainWindowOrientation];

    if (defaultMultiWindowLayout != WINDOW_LAYOUT_UNKNOWN)
    {
      m_Viewer->SetDefaultMultiWindowLayout(defaultMultiWindowLayout);
    }

    m_ControlsWidget->setEnabled(true);
    m_AxialWindowRadioButton->setEnabled(m_MainWindowOrientation != WINDOW_ORIENTATION_AXIAL);
    m_SagittalWindowRadioButton->setEnabled(m_MainWindowOrientation != WINDOW_ORIENTATION_SAGITTAL);
    m_CoronalWindowRadioButton->setEnabled(m_MainWindowOrientation != WINDOW_ORIENTATION_CORONAL);
  }
  else
  {
    windowLayout = defaultMultiWindowLayout;
  }

  m_LayoutWidget->blockSignals(wasBlocked);

  m_Viewer->SetWindowLayout(windowLayout);
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnFocusChanged()
{
  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  mitk::BaseRenderer* focusedRenderer = focusManager->GetFocused();

  const std::vector<QmitkRenderWindow*>& viewerRenderWindows = m_Viewer->GetRenderWindows();
  for (int i = 0; i < viewerRenderWindows.size(); ++i)
  {
    // If the newly focused window is in this widget, nothing to update. Stop early.
    if (focusedRenderer == viewerRenderWindows[i]->GetRenderer())
    {
      this->OnViewerWindowChanged();
      return;
    }
  }

  mitk::IRenderWindowPart* selectedEditor = this->GetSelectedEditor();
  if (selectedEditor)
  {
    /// Note:
    /// We need to look for the focused window among every window of the editor.
    /// The MITK Display has not got the concept of 'selected' window and always
    /// returns the axial window as 'active'. Therefore we cannot use GetActiveQmitkRenderWindow.
    foreach (QmitkRenderWindow* mainWindow, selectedEditor->GetQmitkRenderWindows().values())
    {
      if (focusedRenderer == mainWindow->GetRenderer())
      {
        this->OnMainWindowChanged(selectedEditor, mainWindow);
        break;
      }
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnViewerWindowChanged()
{
  WindowOrientation orientation = m_Viewer->GetOrientation();

  if (orientation != WINDOW_ORIENTATION_UNKNOWN)
  {
    int selectedSlice = m_Viewer->GetSelectedSlice(m_Viewer->GetOrientation());
    int maxSlice = m_Viewer->GetMaxSlice(m_Viewer->GetOrientation());

    bool wasBlocked = m_SliceSpinBox->blockSignals(true);
    m_SliceSpinBox->setMaximum(maxSlice);
    m_SliceSpinBox->setValue(selectedSlice);
    m_SliceSpinBox->setEnabled(true);
    m_SliceSpinBox->blockSignals(wasBlocked);

    double magnification = m_Viewer->GetMagnification(m_Viewer->GetOrientation());
    m_Magnification = magnification;

    wasBlocked = m_MagnificationSpinBox->blockSignals(true);
    m_MagnificationSpinBox->setValue(magnification);
    m_MagnificationSpinBox->setEnabled(true);
    m_MagnificationSpinBox->blockSignals(wasBlocked);
  }
  else
  {
    bool wasBlocked = m_SliceSpinBox->blockSignals(true);
    m_SliceSpinBox->setValue(0);
    m_SliceSpinBox->setEnabled(false);
    m_SliceSpinBox->blockSignals(wasBlocked);

    m_Magnification = 0;

    wasBlocked = m_MagnificationSpinBox->blockSignals(true);
    m_MagnificationSpinBox->setValue(0.0);
    m_MagnificationSpinBox->setEnabled(false);
    m_MagnificationSpinBox->blockSignals(wasBlocked);
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnMainWindowChanged(mitk::IRenderWindowPart* renderWindowPart, QmitkRenderWindow* mainWindow)
{
  mitk::RenderingManager* mainRenderingManager = mainWindow->GetRenderer()->GetRenderingManager();
  if (mainRenderingManager != m_MainRenderingManager)
  {
    if (m_MainRenderingManager)
    {
      m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetAxialWindow()->GetRenderWindow());
      m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetSagittalWindow()->GetRenderWindow());
      m_MainRenderingManager->RemoveRenderWindow(m_Viewer->GetCoronalWindow()->GetRenderWindow());
      m_MainRenderingManager->RemoveRenderWindow(m_Viewer->Get3DWindow()->GetRenderWindow());
    }

    m_MainRenderingManager = mainRenderingManager;

    if (m_MainRenderingManager)
    {
      m_MainRenderingManager->AddRenderWindow(m_Viewer->GetAxialWindow()->GetRenderWindow());
      m_MainRenderingManager->AddRenderWindow(m_Viewer->GetSagittalWindow()->GetRenderWindow());
      m_MainRenderingManager->AddRenderWindow(m_Viewer->GetCoronalWindow()->GetRenderWindow());
      m_MainRenderingManager->AddRenderWindow(m_Viewer->Get3DWindow()->GetRenderWindow());
    }
  }

  // Get hold of main windows, using QmitkAbstractView lookup mitkIRenderWindowPart.
  QmitkRenderWindow* mainAxialWindow = renderWindowPart->GetQmitkRenderWindow("axial");
  QmitkRenderWindow* mainSagittalWindow = renderWindowPart->GetQmitkRenderWindow("sagittal");
  QmitkRenderWindow* mainCoronalWindow = renderWindowPart->GetQmitkRenderWindow("coronal");

  if (mainWindow != mainAxialWindow
      && mainWindow != mainSagittalWindow
      && mainWindow != mainCoronalWindow)
  {
    return;
  }

  QmitkRenderWindow* axialWindow = m_Viewer->GetAxialWindow();
  QmitkRenderWindow* sagittalWindow = m_Viewer->GetSagittalWindow();
  QmitkRenderWindow* coronalWindow = m_Viewer->GetCoronalWindow();

  mitk::SliceNavigationController* axialSnc = axialWindow->GetSliceNavigationController();
  mitk::SliceNavigationController* sagittalSnc = sagittalWindow->GetSliceNavigationController();
  mitk::SliceNavigationController* coronalSnc = coronalWindow->GetSliceNavigationController();

  if (m_MainWindow)
  {
    m_MainWindow->GetSliceNavigationController()->Disconnect(this);
  }
  if (m_MainAxialWindow)
  {
    m_MainAxialWindow->disconnect(SIGNAL(destroyed(QObject*)), this, SLOT(OnAMainWindowDestroyed(QObject*)));
    m_MainAxialSnc->Disconnect(axialSnc);
    axialSnc->Disconnect(m_MainAxialSnc);
  }
  if (m_MainSagittalWindow)
  {
    m_MainSagittalWindow->disconnect(SIGNAL(destroyed(QObject*)), this, SLOT(OnAMainWindowDestroyed(QObject*)));
    m_MainSagittalSnc->Disconnect(sagittalSnc);
    sagittalSnc->Disconnect(m_MainSagittalSnc);
  }
  if (m_MainCoronalWindow)
  {
    m_MainCoronalWindow->disconnect(SIGNAL(destroyed(QObject*)), this, SLOT(OnAMainWindowDestroyed(QObject*)));
    m_MainCoronalSnc->Disconnect(coronalSnc);
    coronalSnc->Disconnect(m_MainCoronalSnc);
  }

  if (!mainWindow)
  {
    m_VisibilityTracker->SetTrackedRenderer(0);
    m_Viewer->SetEnabled(false);

    m_TimeGeometry = 0;

    m_MainWindowSnc = 0;

    m_MainAxialWindow = 0;
    m_MainSagittalWindow = 0;
    m_MainCoronalWindow = 0;

    m_MainAxialSnc = 0;
    m_MainSagittalSnc = 0;
    m_MainCoronalSnc = 0;

    m_MainWindowOrientation = WINDOW_ORIENTATION_UNKNOWN;

    m_Viewer->RequestUpdate();

    return;
  }

  m_MainWindow = mainWindow;
  m_MainWindowSnc = mainWindow->GetSliceNavigationController();

  m_MainAxialWindow = mainAxialWindow;
  m_MainSagittalWindow = mainSagittalWindow;
  m_MainCoronalWindow = mainCoronalWindow;

  m_MainAxialSnc = mainAxialWindow->GetSliceNavigationController();
  m_MainSagittalSnc = mainSagittalWindow->GetSliceNavigationController();
  m_MainCoronalSnc = mainCoronalWindow->GetSliceNavigationController();

  WindowOrientation mainWindowOrientation;
  if (mainWindow == mainAxialWindow)
  {
    mainWindowOrientation = WINDOW_ORIENTATION_AXIAL;
  }
  else if (mainWindow == mainSagittalWindow)
  {
    mainWindowOrientation = WINDOW_ORIENTATION_SAGITTAL;
  }
  else if (mainWindow == mainCoronalWindow)
  {
    mainWindowOrientation = WINDOW_ORIENTATION_CORONAL;
  }
  else
  {
    mainWindowOrientation = WINDOW_ORIENTATION_UNKNOWN;
  }

  /// Note:
  /// The SetWindowLayout function does not change the layout if the viewer has not got
  /// a valid geometry. Therefore, if the viewer is initialised with a geometry for the
  /// first time, we need to set the window layout again, according to the main window
  /// orientation.
  bool geometryFirstInitialised = false;

  mitk::TimeGeometry* timeGeometry = const_cast<mitk::TimeGeometry*>(mainWindow->GetRenderer()->GetWorldTimeGeometry());
  if (timeGeometry != m_TimeGeometry)
  {
    if (!m_TimeGeometry)
    {
      geometryFirstInitialised = true;
      m_Viewer->SetEnabled(true);
    }

    if (timeGeometry)
    {
      m_Viewer->SetTimeGeometry(timeGeometry);
      m_VisibilityTracker->SetTrackedRenderer(mainWindow->GetRenderer());
    }
    else
    {
      m_Viewer->SetEnabled(false);
    }

    m_TimeGeometry = timeGeometry;
  }

  if (mainWindowOrientation != m_MainWindowOrientation || geometryFirstInitialised)
  {
    m_MainWindowOrientation = mainWindowOrientation;
    this->OnMainWindowOrientationChanged(mainWindowOrientation);
  }

  if (m_MainWindow)
  {
    m_MainWindow->GetSliceNavigationController()->ConnectGeometrySendEvent(this);
  }

  /// Note that changing the window layout resets the geometry, what sets the selected position in the centre.
  /// Therefore, we need to set the slice indexes to that of the main windows.

  if (mainAxialWindow)
  {
    m_MainAxialSnc->ConnectGeometryEvents(axialSnc);
    axialSnc->ConnectGeometryEvents(m_MainAxialSnc);
    this->connect(m_MainAxialWindow, SIGNAL(destroyed(QObject*)), SLOT(OnAMainWindowDestroyed(QObject*)));
    axialSnc->GetSlice()->SetPos(m_MainAxialSnc->GetSlice()->GetPos());
  }
  if (mainSagittalWindow)
  {
    m_MainSagittalSnc->ConnectGeometryEvents(sagittalSnc);
    sagittalSnc->ConnectGeometryEvents(m_MainSagittalSnc);
    this->connect(m_MainSagittalWindow, SIGNAL(destroyed(QObject*)), SLOT(OnAMainWindowDestroyed(QObject*)));
    sagittalSnc->GetSlice()->SetPos(m_MainSagittalSnc->GetSlice()->GetPos());
  }
  if (mainCoronalWindow)
  {
    m_MainCoronalSnc->ConnectGeometryEvents(coronalSnc);
    coronalSnc->ConnectGeometryEvents(m_MainCoronalSnc);
    this->connect(m_MainCoronalWindow, SIGNAL(destroyed(QObject*)), SLOT(OnAMainWindowDestroyed(QObject*)));
    coronalSnc->GetSlice()->SetPos(m_MainCoronalSnc->GetSlice()->GetPos());
  }

  m_Viewer->RequestUpdate();
}


//-----------------------------------------------------------------------------
mitk::IRenderWindowPart* QmitkSideViewerWidget::GetSelectedEditor()
{
  berry::IWorkbenchPage::Pointer page = m_ContainingView->GetSite()->GetPage();

  // Returns the active editor if it implements mitk::IRenderWindowPart
  mitk::IRenderWindowPart* renderWindowPart =
      dynamic_cast<mitk::IRenderWindowPart*>(page->GetActiveEditor().GetPointer());

  if (!renderWindowPart)
  {
    // No suitable active editor found, check visible editors
    foreach (berry::IEditorReference::Pointer editor, page->GetEditorReferences())
    {
      berry::IWorkbenchPart::Pointer part = editor->GetPart(false);
      if (page->IsPartVisible(part))
      {
        renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
        break;
      }
    }
  }

  return renderWindowPart;
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnSelectedPositionChanged(const mitk::Point3D& selectedPosition)
{
  niftkSingleViewerWidget* viewer = qobject_cast<niftkSingleViewerWidget*>(this->sender());

  WindowOrientation orientation = m_Viewer->GetOrientation();
  if (orientation != WINDOW_ORIENTATION_UNKNOWN)
  {
    bool wasBlocked = m_SliceSpinBox->blockSignals(true);
    m_SliceSpinBox->setValue(m_Viewer->GetSelectedSlice(orientation));
    m_SliceSpinBox->blockSignals(wasBlocked);
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnScaleFactorChanged(WindowOrientation orientation, double scaleFactor)
{
  double magnification = m_Viewer->GetMagnification(m_Viewer->GetOrientation());

  bool wasBlocked = m_MagnificationSpinBox->blockSignals(true);
  m_MagnificationSpinBox->setValue(magnification);
  m_MagnificationSpinBox->blockSignals(wasBlocked);

  m_Magnification = magnification;
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnWindowLayoutChanged(WindowLayout windowLayout)
{
  bool axialWindowRadioButtonWasBlocked = m_AxialWindowRadioButton->blockSignals(true);
  bool sagittalWindowRadioButtonWasBlocked = m_SagittalWindowRadioButton->blockSignals(true);
  bool coronalWindowRadioButtonWasBlocked = m_CoronalWindowRadioButton->blockSignals(true);
  bool multiWindowRadioButtonWasBlocked = m_MultiWindowRadioButton->blockSignals(true);
  bool multiWindowComboBoxWasBlocked = m_MultiWindowComboBox->blockSignals(true);

  if (windowLayout == WINDOW_LAYOUT_AXIAL)
  {
    m_AxialWindowRadioButton->setChecked(true);
  }
  else if (windowLayout == WINDOW_LAYOUT_SAGITTAL)
  {
    m_SagittalWindowRadioButton->setChecked(true);
  }
  else if (windowLayout == WINDOW_LAYOUT_CORONAL)
  {
    m_CoronalWindowRadioButton->setChecked(true);
  }
  else if (niftk::IsMultiWindowLayout(windowLayout))
  {
    m_MultiWindowRadioButton->setChecked(true);
    if (windowLayout == WINDOW_LAYOUT_COR_AX_H
        || windowLayout == WINDOW_LAYOUT_COR_SAG_H
        || windowLayout == WINDOW_LAYOUT_SAG_AX_H)
    {
      m_MultiWindowComboBox->setCurrentIndex(0);
    }
    else if (windowLayout == WINDOW_LAYOUT_COR_AX_V
        || windowLayout == WINDOW_LAYOUT_COR_SAG_V
        || windowLayout == WINDOW_LAYOUT_SAG_AX_V)
    {
      m_MultiWindowComboBox->setCurrentIndex(1);
    }
    else if (windowLayout == WINDOW_LAYOUT_ORTHO)
    {
      m_MultiWindowComboBox->setCurrentIndex(2);
    }
  }
  else
  {
    m_AxialWindowRadioButton->setChecked(false);
    m_SagittalWindowRadioButton->setChecked(false);
    m_CoronalWindowRadioButton->setChecked(false);
    m_MultiWindowRadioButton->setChecked(false);
  }

  m_AxialWindowRadioButton->blockSignals(axialWindowRadioButtonWasBlocked);
  m_SagittalWindowRadioButton->blockSignals(sagittalWindowRadioButtonWasBlocked);
  m_CoronalWindowRadioButton->blockSignals(coronalWindowRadioButtonWasBlocked);
  m_MultiWindowRadioButton->blockSignals(multiWindowRadioButtonWasBlocked);
  m_MultiWindowComboBox->blockSignals(multiWindowComboBoxWasBlocked);

  m_WindowLayout = windowLayout;

  /// Note:
  /// The selected window has not necessarily been changed, but it is not costly
  /// to refresh the GUI buttons every time.
//  this->OnViewerWindowChanged();
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnSliceSpinBoxValueChanged(int slice)
{
  bool wasBlocked = m_Viewer->blockSignals(true);
  m_Viewer->SetSelectedSlice(m_Viewer->GetOrientation(), slice);
  m_Viewer->blockSignals(wasBlocked);
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::OnMagnificationSpinBoxValueChanged(double magnification)
{
  double roundedMagnification = std::floor(magnification);

  // If we are between two integers, we raise a new event:
  if (magnification != roundedMagnification)
  {
    double newMagnification = roundedMagnification;
    // If the value has decreased, we have to increase the rounded value.
    if (magnification < m_Magnification)
    {
      newMagnification += 1.0;
    }

    m_MagnificationSpinBox->setValue(newMagnification);
  }
  else
  {
    bool wasBlocked = m_Viewer->blockSignals(true);
    m_Viewer->SetMagnification(m_Viewer->GetOrientation(), magnification);
    m_Viewer->blockSignals(wasBlocked);
    m_Magnification = magnification;
  }
}


//-----------------------------------------------------------------------------
void QmitkSideViewerWidget::SetGeometry(const itk::EventObject& geometrySendEvent)
{
  const mitk::SliceNavigationController::GeometrySendEvent* sendEvent =
      dynamic_cast<const mitk::SliceNavigationController::GeometrySendEvent *>(&geometrySendEvent);

  assert(sendEvent);

  const mitk::TimeGeometry* timeGeometry = sendEvent->GetTimeGeometry();

  assert(timeGeometry);

  if (timeGeometry != m_TimeGeometry)
  {
    m_Viewer->SetTimeGeometry(timeGeometry);

    if (!m_TimeGeometry)
    {
      m_Viewer->SetEnabled(true);

      /// Note: SetWindowLayout does not work when the viewer has no geometry.
      /// Therefore, we need to set it at the first time when the viewer receives
      /// a geometry.
      this->OnMainWindowOrientationChanged(m_MainWindowOrientation);
    }

    m_TimeGeometry = timeGeometry;
  }
}

}
