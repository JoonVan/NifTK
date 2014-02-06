/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef niftkSingleViewerWidget_h
#define niftkSingleViewerWidget_h

#include <niftkDnDDisplayExports.h>

#include <deque>

#include <mitkDataStorage.h>
#include <mitkGeometry3D.h>
#include <mitkTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <QmitkRenderWindow.h>

#include <QColor>
#include <QTime>
#include <QWidget>

#include <mitkMIDASEnums.h>
#include <niftkDnDDisplayEnums.h>
#include "Interactions/mitkDnDDisplayStateMachineResponder.h"
#include "Interactions/mitkDnDDisplayStateMachine.h"

class QGridLayout;
class niftkMultiWindowWidget;

/**
 * \class niftkSingleViewerWidget
 * \brief A widget to wrap a single niftkMultiWindowWidget,
 * providing methods for switching the render window layout, remembering
 * the last slice, magnification and cursor position.
 *
 * IMPORTANT: This class acts as a wrapper for niftkMultiWindowWidget.
 * Do not expose niftkMultiWindowWidget, or any member variables, or any
 * dependency from niftkMultiWindowWidget to the rest of the application.
 *
 * Additionally, this widget contains its own mitk::RenderingManager which is passed to the
 * niftkMultiWindowWidget, which is itself a sub-class of QmitkStdMultiWidget.
 * This means the niftkMultiWindowWidget will update and render independently of the
 * rest of the application, and care must be taken to manage this. The reason is that
 * each of these viewers in a multi-viewer could have it's own geometry, and sometimes
 * a very different geometry from other windows, and then when the "Bind Slices" button
 * is clicked, they must all align to a specific (the currently selected window) geometry.
 * So it became necessary to manage this independent from the rest of the MITK application.
 *
 * <pre>
 * Note: The requirements specification for MIDAS style zoom basically says.
 *
 * magnification   : actual pixels per voxel.
 * on MIDAS widget :
 * 2               : 3
 * 1               : 2
 * 0               : 1 (i.e. no magnification).
 * -1              : 0.5 (i.e. 1 pixel covers 2 voxels).
 * -2              : 0.33 (i.e. 1 pixel covers 3 voxels).
 * etc.
 * </pre>
 *
 * \sa QmitkRenderWindow
 * \sa niftkMultiWindowWidget
 */
class NIFTKDNDDISPLAY_EXPORT niftkSingleViewerWidget : public QWidget, public mitk::DnDDisplayStateMachineResponder
{

  Q_OBJECT;

public:

  niftkSingleViewerWidget(QWidget* parent = 0, mitk::RenderingManager* renderingManager = 0);
  virtual ~niftkSingleViewerWidget();

  /// \brief Sets the window to be enabled, where if enabled==true, it's listening to events, and fully turned on.
  void SetEnabled(bool enabled);

  /// \brief Returns the enabled flag.
  bool IsEnabled() const;

  /// \brief If b==true, this widget is "selected" meaning it will have coloured borders,
  /// and if b==false, it is not selected, and will not have coloured borders.
  void SetSelected(bool b);

  /// \brief Returns true if this widget is selected and false otherwise.
  bool IsSelected() const;

  /// \brief Returns the selected window, that is the one with the coloured border.
  QmitkRenderWindow* GetSelectedRenderWindow() const;

  /// \brief Selects the render window and puts put a coloured border round it.
  void SetSelectedRenderWindow(QmitkRenderWindow* renderWindow);

  /// \brief Returns the specifically selected sub-pane.
  std::vector<QmitkRenderWindow*> GetVisibleRenderWindows() const;

  /// \brief Returns the list of all QmitkRenderWindow contained herein.
  const std::vector<QmitkRenderWindow*>& GetRenderWindows() const;

  /// \brief Returns the Axial Window.
  QmitkRenderWindow* GetAxialWindow() const;

  /// \brief Returns the Coronal Window.
  QmitkRenderWindow* GetCoronalWindow() const;

  /// \brief Returns the Sagittal Window.
  QmitkRenderWindow* GetSagittalWindow() const;

  /// \brief Returns the 3D Window.
  QmitkRenderWindow* Get3DWindow() const;

  /// \brief Returns the orientation for the selected window, returning MIDAS_ORIENTATION_UNKNOWN if not axial, sagittal or coronal.
  MIDASOrientation GetOrientation() const;

  /// \brief Turn the 2D cursors on/off locally.
  void SetCursorVisible(bool visible);

  /// \brief Get the flag controlling 2D cursors on/off.
  bool IsCursorVisible() const;

  /// \brief Turn the 2D cursors on/off globally.
  void SetCursorGloballyVisible(bool visible);

  /// \brief Get the flag controlling 2D cursors on/off.
  bool IsCursorGloballyVisible() const;

  /// \brief Tells if the direction annotations are visible.
  bool AreDirectionAnnotationsVisible() const;

  /// \brief Sets the visibility of the direction annotations.
  void SetDirectionAnnotationsVisible(bool visible);

  /// \brief Returns the flag indicating if nodes will be visible in the 3D window in 2x2 window layout. In 3D window layout, always visible.
  bool GetShow3DWindowIn2x2WindowLayout() const;

  /// \brief If true, then nodes will be visible in the 3D window when in 2x2 window layout. In 3D window layout, always visible.
  void SetShow3DWindowIn2x2WindowLayout(bool enabled);

  /// \brief Sets a flag to determine if we remember the image positions (slice, time step, scale factor) when we switch the render window layout
  void SetRememberSettingsPerWindowLayout(bool remember);

  /// \brief Sets a flag to determine if we remember the image positions (slice, time step, scale factor) when we switch the render window layout
  bool GetRememberSettingsPerWindowLayout() const;

  /// \brief Sets the background colour.
  void SetBackgroundColor(QColor color);

  /// \brief Gets the background colour.
  QColor GetBackgroundColor() const;

  /// \brief Returns the maximum allowed slice index for a given orientation.
  unsigned int GetMaxSliceIndex(MIDASOrientation orientation) const;

  /// \brief Gets the maximum time step.
  unsigned int GetMaxTimeStep() const;

  /// \brief Returns true if the widget is fully created and contains the given render window, and false otherwise.
  bool ContainsRenderWindow(QmitkRenderWindow *renderWindow) const;

  /// \brief Sets the visible flag for all the nodes, and all the renderers in the QmitkStdMultiWidget base class.
  void SetRendererSpecificVisibility(std::vector<mitk::DataNode*> nodes, bool visible);

  /// \brief Returns the minimum allowed magnification, which is passed in as constructor arg, and held constant.
  double GetMinMagnification() const;

  /// \brief Returns the maximum allowed magnification, which is passed in as constructor arg, and held constant.
  double GetMaxMagnification() const;

  /// \brief Returns the data storage or NULL if widget is not fully created, or datastorage has not been set.
  mitk::DataStorage::Pointer GetDataStorage() const;

  /// \brief Sets the data storage on m_DataStorage, m_RenderingManager and m_MultiWidget.
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  /// \brief As each widget has its own rendering manager, we have to manually ask each widget to re-render.
  void RequestUpdate();

  /// \brief Sets the world geometry that we are sampling and sends a GeometryChanged signal.
  void SetGeometry(mitk::TimeGeometry::Pointer timeGeometry);

  /// \brief Gets the world geometry, to pass to other viewers for when slices are bound.
  mitk::TimeGeometry::Pointer GetGeometry();

  /// \brief Sets the world geometry that we are sampling when we are in bound mode.
  void SetBoundGeometry(mitk::TimeGeometry::Pointer geometry);

  /// \brief Sets the geometry binding 'on' or 'off'. If 'on' then the geometry of
  /// this viewer will be bound to other viewers in the same multi viewer widget.
  void SetBoundGeometryActive(bool isBound);

  /// \brief Returns true if the geometry of the viewer is bound to other viewers, otherwise false.
  bool IsBoundGeometryActive();

  /// \brief Get the current slice index for a given orientation.
  unsigned int GetSliceIndex(MIDASOrientation orientation) const;

  /// \brief Set the current slice index for a given orientation.
  void SetSliceIndex(MIDASOrientation orientation, unsigned int sliceIndex);

  /// \brief Get the current time step.
  unsigned int GetTimeStep() const;

  /// \brief Set the current time step.
  void SetTimeStep(unsigned int timeStep);

  /// \brief Gets the render window layout.
  WindowLayout GetWindowLayout() const;

  /// \brief Sets the render window layout to either axial, sagittal or coronal, 3D or ortho (2x2) etc, effectively causing a view reset.
  void SetWindowLayout(WindowLayout windowLayout, bool dontSetSelectedPosition = false, bool dontSetCursorPositions = false, bool dontSetScaleFactors = false);

  /// \brief Get the currently selected position in world coordinates (mm)
  const mitk::Point3D& GetSelectedPosition() const;

  /// \brief Set the currently selected position in world coordinates (mm)
  void SetSelectedPosition(const mitk::Point3D& selectedPosition);

  /// \brief Get the current cursor position of the render window in pixels, normalised with the size of the render windows.
  mitk::Vector2D GetCursorPosition(MIDASOrientation orientation) const;

  /// \brief Set the current cursor position of the render window in pixels, normalised with the size of the render windows.
  void SetCursorPosition(MIDASOrientation orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Gets the current cursor position of each render window in pixels, normalised with the size of the render windows.
  const std::vector<mitk::Vector2D>& GetCursorPositions() const;

  /// \brief Sets the current cursor position of each render window in pixels, normalised with the size of the render windows.
  void SetCursorPositions(const std::vector<mitk::Vector2D>& cursorPositions);

  /// \brief Get the current scale factor.
  double GetScaleFactor(MIDASOrientation orientation) const;

  /// \brief Set the current scale factor.
  void SetScaleFactor(MIDASOrientation orientation, double scaleFactor);

  /// \brief Gets the current scale factor of each render window.
  const std::vector<double>& GetScaleFactors() const;

  /// \brief Sets the current scale factor for each render window.
  void SetScaleFactors(const std::vector<double>& scaleFactors);

  /// \brief Get the current magnification.
  double GetMagnification(MIDASOrientation orientation) const;

  /// \brief Set the current magnification.
  void SetMagnification(MIDASOrientation orientation, double magnification);

  /// \brief Sets the flag that controls whether we are listening to the navigation controller events.
  void SetNavigationControllerEventListening(bool enabled);

  /// \brief Gets the flag that controls whether we are listening to the navigation controller events.
  bool GetNavigationControllerEventListening() const;

  /// \brief Sets the flag that controls whether the display interactions are enabled for the render windows.
  void SetDisplayInteractionsEnabled(bool enabled);

  /// \brief Gets the flag that controls whether the display interactions are enabled for the render windows.
  bool AreDisplayInteractionsEnabled() const;

  /// \brief Gets the flag that controls whether the cursor position is bound across the render windows.
  bool GetCursorPositionBinding() const;

  /// \brief Sets the flag that controls whether the cursor position is bound across the render windows.
  void SetCursorPositionBinding(bool bound);

  /// \brief Gets the flag that controls whether the scale factors are bound across the render windows.
  bool GetScaleFactorBinding() const;

  /// \brief Sets the flag that controls whether the scale factors are bound across the render windows.
  void SetScaleFactorBinding(bool bound);

  /// \brief Only to be used for Thumbnail mode, makes the displayed 2D geometry fit the display window.
  void FitToDisplay();

  /// \brief Returns pointers to the widget planes.
  std::vector<mitk::DataNode*> GetWidgetPlanes();

  /// \brief According to the currently set geometry will return +1, or -1 for the direction to increment the slice index to move "up".
  ///
  /// \see mitkMIDASOrientationUtils.
  int GetSliceUpDirection(MIDASOrientation orientation) const;

  /// \brief Sets the default single window layout (axial, coronal etc.), which only takes effect when a node is next dropped into a given window.
  void SetDefaultSingleWindowLayout(WindowLayout windowLayout);

  /// \brief Sets the default multiple window layout (2x2, 3H, 3V etc.), which only takes effect when a node is next dropped into a given window.
  void SetDefaultMultiWindowLayout(WindowLayout windowLayout);

  /// \brief Move anterior a slice.
  bool MoveAnterior();

  /// \brief Move posterior a slice.
  bool MovePosterior();

  /// \brief Switch to Axial.
  bool SwitchToAxial();

  /// \brief Switch to Sagittal.
  bool SwitchToSagittal();

  /// \brief Switch to Coronal.
  bool SwitchToCoronal();

  /// \brief Switch to 3D.
  bool SwitchTo3D();

  /// \brief Switch the from single window to multiple windows or back
  bool ToggleMultiWindowLayout();

  /// \brief Shows or hides the cursor.
  bool ToggleCursorVisibility();

  /**
   * \brief Sets the focus to the currently selected window, or to this viewer
   * itself if no window is selected.
   */
  virtual void SetFocus();

signals:

  /// \brief Emitted when nodes are dropped on the SingleViewer widget.
  void NodesDropped(niftkSingleViewerWidget* thisViewer, QmitkRenderWindow *renderWindow, std::vector<mitk::DataNode*> nodes);

  /// \brief Emitted when the selected slice has changed in a render window of this viewer.
  void SelectedPositionChanged(niftkSingleViewerWidget* thisViewer, const mitk::Point3D& selectedPosition);

  /// \brief Emitted when the selected time step has changed in this viewer.
  void SelectedTimeStepChanged(niftkSingleViewerWidget* thisViewer, int timeStep);

  /// \brief Emitted when the cursor position has changed in this viewer.
  void CursorPositionChanged(niftkSingleViewerWidget* thisViewer, MIDASOrientation orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Emitted when the scale factor has changed in this viewer.
  void ScaleFactorChanged(niftkSingleViewerWidget* thisViewer, MIDASOrientation orientation, double scaleFactor);

  /// \brief Emitted when the cursor position binding has changed in this viewer.
  void CursorPositionBindingChanged(niftkSingleViewerWidget* thisViewer, bool bound);

  /// \brief Emitted when the scale factor binding has changed in this viewer.
  void ScaleFactorBindingChanged(niftkSingleViewerWidget* thisViewer, bool bound);

  /// \brief Emitted when the window layout has changed in this viewer.
  void WindowLayoutChanged(niftkSingleViewerWidget* thisViewer, WindowLayout windowLayout);

  /// \brief Emitted when the geometry of this viewer has changed.
  void GeometryChanged(niftkSingleViewerWidget* thisViewer, mitk::TimeGeometry* geometry);

  /// \brief Emitted when the visibility of the cursor (aka. crosshair) has changed.
  void CursorVisibilityChanged(niftkSingleViewerWidget* thisViewer, bool visible);

protected:

  /// \brief Re-renders the visible render windows on a paint event, e.g. when the widget is resized.
  virtual void paintEvent(QPaintEvent* event);

protected slots:

  /// \brief Called when nodes are dropped on the contained render windows.
  virtual void OnNodesDropped(QmitkRenderWindow *renderWindow, std::vector<mitk::DataNode*> nodes);

  /// \brief Called when the selected position has changed.
  virtual void OnSelectedPositionChanged(const mitk::Point3D& selectedPosition);

  /// \brief Called when the cursor position has changed.
  virtual void OnCursorPositionChanged(MIDASOrientation orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Called when the scale factor has changed.
  virtual void OnScaleFactorChanged(MIDASOrientation orientation, double scaleFactor);

private:

  inline int Index(int index) const
  {
    return (index << 1) + m_IsBoundGeometryActive;
  }

  /// \brief Used to move either anterior/posterior by a certain number of slices.
  bool MoveAnteriorPosterior(int slices);

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::RenderingManager::Pointer m_RenderingManager;

  QGridLayout* m_GridLayout;
  niftkMultiWindowWidget* m_MultiWidget;

  bool m_IsBoundGeometryActive;
  mitk::TimeGeometry::Pointer m_Geometry;       // This comes from which ever image is dropped, so not visible outside this class.
  mitk::TimeGeometry::Pointer m_BoundGeometry;  // Passed in, when we do "bind", so shared amongst multiple windows.

  double m_MinimumMagnification;         // Passed in as constructor arguments, so this class unaware of where it came from.
  double m_MaximumMagnification;         // Passed in as constructor arguments, so this class unaware of where it came from.

  WindowLayout m_WindowLayout;
  MIDASOrientation m_Orientation;

  /// \brief Stores the selected point per window layout. Two for each window layout. Unbound, then bound, alternatingly.
  mitk::Point3D m_SelectedPositions[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the selected time step. One for unbound, one for bound.
  int m_TimeSteps[2];                                             // Two, one for unbound, one for bound.

  /// \brief Stores the cursor positions for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  /// The vectors store the cursor positions for the render windows of the layout.
  std::vector<mitk::Vector2D> m_CursorPositions[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the cursor positions for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  /// The vectors store the scale factors of the render windows of the layout.
  std::vector<double> m_ScaleFactors[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the selected render window for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  QmitkRenderWindow* m_SelectedRenderWindow[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the cursor position binding property for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  bool m_CursorPositionBinding[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the scale factor binding property for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  bool m_ScaleFactorBinding[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores whether the layout has been initialised. Two for each window layout. Unbound, then bound, alternatingly.
  bool m_WindowLayoutInitialised[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the last three selected positions.
  ///
  /// The aim with storing these positions is that if the window layout is switched
  /// between single and multi by double clicking, we can can discard the position changes
  /// because of the double clicking itself, and remember the previously selected position,
  /// so that we can restore it next time when the user returns to the window layout.
  std::deque<mitk::Point3D> m_LastSelectedPositions;

  /// \brief Stores the time of the last position selection events in milliseconds.
  ///
  /// This is used to distinguish between simple position selection events by a single click
  /// and single/multiple window layout switch by double click. If latter happens, we have to
  /// save the position from before the double clicking.
  std::deque<QTime> m_LastSelectedPositionTimes;

  /// \brief Stores the position of the cursor in the 2D render windows a the last seven times.
  ///
  /// The aim with storing these positions is that if the window layout is switched
  /// between single and multi by double clicking, we can can discard the position changes
  /// because of the double clicking itself, and remember the previously selected position,
  /// so that we can restore it next time when the user returns to the window layout.
  std::deque<std::vector<mitk::Vector2D> > m_LastCursorPositions;

  /// \brief Stores the time of the last events in milliseconds when the position of the cursor has changed in the 2D windows.
  ///
  /// This is used to distinguish between simple position selection events by a single click
  /// and single/multiple window layout switch by double click. If latter happens, we have to
  /// save the position from before the double clicking.
  std::deque<QTime> m_LastCursorPositionTimes;

  bool m_NavigationControllerEventListening;
  bool m_RememberSettingsPerWindowLayout;

  WindowLayout m_SingleWindowLayout;
  WindowLayout m_MultiWindowLayout;

  mitk::DnDDisplayStateMachine::Pointer m_DnDDisplayStateMachine;
};

#endif
