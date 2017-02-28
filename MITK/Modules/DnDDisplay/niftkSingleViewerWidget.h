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
#include <mitkBaseGeometry.h>
#include <mitkTimeGeometry.h>
#include <mitkRenderingManager.h>
#include <QmitkRenderWindow.h>

#include <QColor>
#include <QTime>
#include <QWidget>

#include "niftkDnDDisplayEnums.h"
#include "Interactions/niftkDnDDisplayInteractor.h"

class QGridLayout;


namespace niftk
{

class MultiWindowWidget;

/**
 * \class SingleViewerWidget
 * \brief A widget to wrap a single MultiWindowWidget,
 * providing methods for switching the render window layout, remembering
 * the last slice, magnification and cursor position.
 *
 * IMPORTANT: This class acts as a wrapper for MultiWindowWidget.
 * Do not expose MultiWindowWidget, or any member variables, or any
 * dependency from MultiWindowWidget to the rest of the application.
 *
 * Additionally, this widget contains its own mitk::RenderingManager which is passed to the
 * MultiWindowWidget, which is itself a sub-class of QmitkStdMultiWidget.
 * This means the MultiWindowWidget will update and render independently of the
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
 * \sa MultiWindowWidget
 */
class NIFTKDNDDISPLAY_EXPORT SingleViewerWidget : public QWidget
{

  Q_OBJECT

public:

  /// \brief Constructs a SingleViewerWidget object.
  /// \param parent The parent object.
  /// \param renderingManager The rendering manager.
  /// \param The name of the viewer.
  ///        The name is used to construct the name of the renderers and must therefore be unique.
  SingleViewerWidget(QWidget* parent = 0, mitk::RenderingManager* renderingManager = 0, const QString& name = "DnD-Viewer");
  virtual ~SingleViewerWidget();

  /// \brief Gets the display convention of the viewer.
  int GetDisplayConvention() const;

  /// \brief Sets the display convention of the viewer.
  void SetDisplayConvention(int displayConvention);

  /// \brief Returns the enabled flag.
  bool IsEnabled() const;

  /// \brief Sets the window to be enabled, where if enabled==true, it's listening to events, and fully turned on.
  void SetEnabled(bool enabled);

  /// \brief Tells if the selected render window has the focus.
  bool IsFocused() const;

  /// \brief Sets the focus to the selected render window.
  void SetFocused();

  /// \brief Returns the selected window.
  /// If a window has the focus (and it has a coloured border) then it is
  /// returned. Otherwise, the first visible window is returned.
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

  /// \brief Returns the orientation for the selected window, returning WINDOW_ORIENTATION_UNKNOWN if not axial, sagittal or coronal.
  WindowOrientation GetOrientation() const;

  /// \brief Get the flag controlling 2D cursors on/off.
  bool IsCursorVisible() const;

  /// \brief Turn the 2D cursors on/off.
  void SetCursorVisible(bool visible);

  /// \brief Tells if the direction annotations are visible.
  bool AreDirectionAnnotationsVisible() const;

  /// \brief Sets the visibility of the direction annotations.
  void SetDirectionAnnotationsVisible(bool visible);

  /// \brief Tells if the position annotation is visible.
  bool IsPositionAnnotationVisible() const;

  /// \brief Sets the visibility of the positon annotation.
  void SetPositionAnnotationVisible(bool visible);

  /// \brief Tells if the intensity annotation is visible.
  bool IsIntensityAnnotationVisible() const;

  /// \brief Sets the visibility of the intensity annotation.
  void SetIntensityAnnotationVisible(bool visible);

  /// \brief Tells if the property annotation is visible.
  bool IsPropertyAnnotationVisible() const;

  /// \brief Sets the visibility of the property annotation.
  void SetPropertyAnnotationVisible(bool visible);

  /// \brief Gets the list of properties to display as annotation.
  QStringList GetPropertiesForAnnotation() const;

  /// \brief Sets the list of properties to display as annotation.
  void SetPropertiesForAnnotation(const QStringList& propertiesForAnnotation);

  /// \brief Sets a flag to determine if we remember the image positions (slice, time step, scale factor) when we switch the render window layout
  void SetRememberSettingsPerWindowLayout(bool remember);

  /// \brief Sets a flag to determine if we remember the image positions (slice, time step, scale factor) when we switch the render window layout
  bool GetRememberSettingsPerWindowLayout() const;

  /// \brief Sets the background colour.
  void SetBackgroundColour(QColor colour);

  /// \brief Gets the background colour.
  QColor GetBackgroundColour() const;

  /// \brief Returns the maximum allowed slice index for a given orientation.
  int GetMaxSlice(WindowOrientation orientation) const;

  /// \brief Gets the maximum time step.
  int GetMaxTimeStep() const;

  /// \brief Returns true if the widget is fully created and contains the given render window, and false otherwise.
  bool ContainsRenderWindow(QmitkRenderWindow *renderWindow) const;

  /// \brief Sets the visible flag for all the nodes, and all the renderers.
  void SetVisibility(std::vector<mitk::DataNode*> nodes, bool visible);

  /// \brief Returns the minimum allowed magnification.
  double GetMinMagnification() const;

  /// \brief Returns the maximum allowed magnification.
  double GetMaxMagnification() const;

  /// \brief As each widget has its own rendering manager, we have to manually ask each widget to re-render.
  void RequestUpdate();

  /// \brief Gets the world geometry.
  const mitk::TimeGeometry* GetTimeGeometry() const;

  /// \brief Sets the world geometry that we are sampling and sends a TimeGeometryChanged signal.
  void SetTimeGeometry(const mitk::TimeGeometry* timeGeometry);

  /// \brief Sets the world geometry that we are sampling when we are in bound mode.
  void SetBoundTimeGeometry(const mitk::TimeGeometry* timeGeometry);

  /// \brief Sets the geometry binding 'on' or 'off'. If 'on' then the geometry of
  /// this viewer will be bound to other viewers in the same multi viewer widget.
  void SetBoundTimeGeometryActive(bool isBound);

  /// \brief Returns true if the geometry of the viewer is bound to other viewers, otherwise false.
  bool IsBoundTimeGeometryActive();

  /// \brief Gets the index of the selected slice for a given orientation.
  int GetSelectedSlice(WindowOrientation orientation) const;

  /// \brief Sets the index of the selected slice for a given orientation.
  void SetSelectedSlice(WindowOrientation orientation, int slice);

  /// \brief Get the current time step.
  int GetTimeStep() const;

  /// \brief Set the current time step.
  void SetTimeStep(int timeStep);

  /// \brief Gets the render window layout.
  WindowLayout GetWindowLayout() const;

  /// \brief Sets the render window layout to either axial, sagittal or coronal, 3D or ortho (2x2) etc, effectively causing a view reset.
  void SetWindowLayout(WindowLayout windowLayout);

  /// \brief Get the currently selected position in world coordinates (mm)
  const mitk::Point3D& GetSelectedPosition() const;

  /// \brief Set the currently selected position in world coordinates (mm)
  void SetSelectedPosition(const mitk::Point3D& selectedPosition);

  /// \brief Get the current cursor position of the render window in pixels, normalised with the size of the render windows.
  mitk::Vector2D GetCursorPosition(WindowOrientation orientation) const;

  /// \brief Set the current cursor position of the render window in pixels, normalised with the size of the render windows.
  void SetCursorPosition(WindowOrientation orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Gets the current cursor position of each render window in pixels, normalised with the size of the render windows.
  const std::vector<mitk::Vector2D>& GetCursorPositions() const;

  /// \brief Sets the current cursor position of each render window in pixels, normalised with the size of the render windows.
  void SetCursorPositions(const std::vector<mitk::Vector2D>& cursorPositions);

  /// \brief Get the current scale factor.
  double GetScaleFactor(WindowOrientation orientation) const;

  /// \brief Set the current scale factor.
  void SetScaleFactor(WindowOrientation orientation, double scaleFactor);

  /// \brief Gets the current scale factor of each render window.
  const std::vector<double>& GetScaleFactors() const;

  /// \brief Sets the current scale factor for each render window.
  void SetScaleFactors(const std::vector<double>& scaleFactors);

  /// \brief Get the current magnification.
  double GetMagnification(WindowOrientation orientation) const;

  /// \brief Set the current magnification.
  void SetMagnification(WindowOrientation orientation, double magnification);

  /// \brief Gets the flag that controls whether we are listening to the navigation controller events.
  bool IsLinkedNavigationEnabled() const;

  /// \brief Sets the flag that controls whether we are listening to the navigation controller events.
  void SetLinkedNavigationEnabled(bool linkedNavigationEnabled);

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

  /// \brief Moves the displayed regions to the centre of the 2D render windows and scales them, optionally.
  /// If no scale factor is given or the specified value is 0.0 then the maximal zooming is
  /// applied, using which each region fits into their window, also considering whether the scale
  /// factors are bound across the windows.
  /// If a positive scale factor is given then the scale factor of each render window is set
  /// to the specified value.
  /// If the specified scale factor is -1.0 then no scaling is applied.
  /// The regions are moved to the middle of the render windows in each cases.
  void FitToDisplay(double scaleFactor = 0.0);

  /// \brief Returns pointers to the widget planes.
  std::vector<mitk::DataNode*> GetWidgetPlanes();

  /// \brief According to the currently set geometry will return +1, or -1 for the direction to increment the slice index to move "up".
  int GetSliceUpDirection(WindowOrientation orientation) const;

  /// \brief Sets the default single window layout (axial, coronal etc.), which only takes effect when a node is next dropped into a given window.
  void SetDefaultSingleWindowLayout(WindowLayout windowLayout);

  /// \brief Sets the default multiple window layout (2x2, 3H, 3V etc.), which only takes effect when a node is next dropped into a given window.
  void SetDefaultMultiWindowLayout(WindowLayout windowLayout);

  /// \brief Selects the nth slice before or after the currently selected slice.
  /// Slices are ordered: coronal: anterior to posterior, sagittal: right to left, axial: inferior to superior
  void MoveSlice(WindowOrientation orientation, int delta, bool restart = false);

  /// \brief Switch the from single window to multiple windows or back
  void ToggleMultiWindowLayout();

  /// \brief Shows or hides the cursor.
  void ToggleCursorVisibility();

  /// \brief Shows or hides the direction annotations.
  void ToggleDirectionAnnotations();

  /// \brief Shows or hides the position annotation.
  void TogglePositionAnnotation();

  /// \brief Shows or hides the intensity annotation.
  void ToggleIntensityAnnotation();

  /// \brief Shows or hides the property annotation.
  void TogglePropertyAnnotation();

  /// \brief Blocks the update of the viewer.
  ///
  /// Returns true if the update was already blocked, otherwise false.
  /// While the update is blocked, the state changes are recorded but the render windows are
  /// not updated and no signals are sent out. The render windows are updated and the "pending"
  /// signals are sent out when the update is unblocked.
  /// The purpose of this function is to avoid unnecessary updates and signals when a serious of
  /// operations needs to be performed on the viewer as a single atomic unit, e.g. changing
  /// layout and setting positions.
  /// After the required state of the viewer is set, the previous blocking state should be restored.
  ///
  /// Pattern of usage:
  ///
  ///     bool updateWasBlocked = viewer->BlockUpdate(true);
  ///     ... set the required state ...
  ///     viewer->BlockUpdate(updateWasBlocked);
  ///
  bool BlockUpdate(bool blocked);

signals:

  /// \brief Emitted when nodes are dropped on the SingleViewer widget.
  void NodesDropped(std::vector<mitk::DataNode*> nodes);

  /// \brief Emitted when the selected slice has changed in a render window of this viewer.
  void SelectedPositionChanged(const mitk::Point3D& selectedPosition);

  /// \brief Emitted when the selected time step has changed in this viewer.
  void TimeStepChanged(int timeStep);

  /// \brief Emitted when the cursor position has changed in this viewer.
  void CursorPositionChanged(WindowOrientation orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Emitted when the scale factor has changed in this viewer.
  void ScaleFactorChanged(WindowOrientation orientation, double scaleFactor);

  /// \brief Emitted when the cursor position binding has changed in this viewer.
  void CursorPositionBindingChanged(bool bound);

  /// \brief Emitted when the scale factor binding has changed in this viewer.
  void ScaleFactorBindingChanged(bool bound);

  /// \brief Emitted when the window layout has changed in this viewer.
  void WindowLayoutChanged(WindowLayout windowLayout);

  /// \brief Emitted when the geometry of this viewer has changed.
  void TimeGeometryChanged(const mitk::TimeGeometry* timeGeometry);

  /// \brief Emitted when the visibility of the cursor (aka. crosshair) has changed.
  void CursorVisibilityChanged(bool visible);

  /// \brief Emitted when the visibility of the direction annotations has changed.
  void DirectionAnnotationsVisibilityChanged(bool visible);

  /// \brief Emitted when the visibility of the position annotation has changed.
  void PositionAnnotationVisibilityChanged(bool visible);

  /// \brief Emitted when the visibility of the intensity annotation has changed.
  void IntensityAnnotationVisibilityChanged(bool visible);

  /// \brief Emitted when the visibility of the property annotation has changed.
  void PropertyAnnotationVisibilityChanged(bool visible);

public slots:

  /// \brief Called when nodes are dropped on the contained render windows.
  virtual void OnNodesDropped(QmitkRenderWindow *renderWindow, std::vector<mitk::DataNode*> nodes);

protected:

  /// \brief Re-renders the visible render windows on a paint event, e.g. when the widget is resized.
  virtual void paintEvent(QPaintEvent* event) override;

protected slots:

  /// \brief Called when the window layout has changed.
  virtual void OnWindowLayoutChanged(WindowLayout windowLayout);

  /// \brief Called when the selected position has changed.
  virtual void OnSelectedPositionChanged(const mitk::Point3D& selectedPosition);

  /// \brief Called when the cursor position has changed.
  virtual void OnCursorPositionChanged(int orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Called when the scale factor has changed.
  virtual void OnScaleFactorChanged(int orientation, double scaleFactor);

  /// \brief Called when the cursor position binding has changed.
  virtual void OnCursorPositionBindingChanged();

  /// \brief Called when the scale factor binding has changed.
  virtual void OnScaleFactorBindingChanged();

private:

  inline int Index(int index) const
  {
    return (index << 1) + m_IsBoundTimeGeometryActive;
  }

  /// \brief Resets the last few remembered selected and cursor positions.
  /// These positions are remembered so that if you double click to toggle between single and
  /// multiple window layout, the position changing side-effect of the double clicking can be
  /// un-done, and the positions can be restored from the time before the double clicking.
  /// This function clears the previous remembered positions and remembers the actual positions.
  void ResetLastPositions();

  /// \brief Gets the position of the centre of the displayed region, relative to the render window.
  mitk::Vector2D GetCentrePosition(int windowIndex);

  /// \brief Gets the position of the centre of the displayed regions, relative to their render windows.
  std::vector<mitk::Vector2D> GetCentrePositions();

  /// \brief Gets the cursor position in the given render window, assuming that the centre of the displayed region is at the given display position.
  mitk::Vector2D GetCursorPositionFromCentre(int windowIndex, const mitk::Vector2D& centrePosition);

  /// \brief Gets the cursor positions in the render windows, assuming that the centre of the displayed regions is at the given display positions.
  std::vector<mitk::Vector2D> GetCursorPositionsFromCentres(const std::vector<mitk::Vector2D>& centrePositions);

  mitk::RenderingManager::Pointer m_RenderingManager;

  int m_DisplayConvention;

  QGridLayout* m_GridLayout;
  MultiWindowWidget* m_MultiWidget;

  bool m_IsBoundTimeGeometryActive;
  mitk::TimeGeometry::ConstPointer m_TimeGeometry;       // This comes from which ever image is dropped, so not visible outside this class.
  mitk::TimeGeometry::ConstPointer m_BoundTimeGeometry;  // Passed in, when we do "bind", so shared amongst multiple windows.

  double m_MinimumMagnification;         // Passed in as constructor arguments, so this class unaware of where it came from.
  double m_MaximumMagnification;         // Passed in as constructor arguments, so this class unaware of where it came from.

  WindowLayout m_WindowLayout;

  /// \brief Stores the selected position for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  mitk::Point3D m_SelectedPositions[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the centre positions for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  /// The vectors store the centre positions for the render windows of the layout.
  std::vector<mitk::Vector2D> m_CentrePositions[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the cursor positions for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  /// The vectors store the scale factors of the render windows of the layout.
  std::vector<double> m_ScaleFactors[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the cursor position binding property for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  bool m_CursorPositionBinding[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores the scale factor binding property for each window layout. Two for each window layout. Unbound, then bound, alternatingly.
  bool m_ScaleFactorBinding[WINDOW_LAYOUT_NUMBER * 2];

  /// \brief Stores whether the geometry has been initialised.
  bool m_GeometryInitialised;

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

  bool m_RememberSettingsPerWindowLayout;

  WindowLayout m_SingleWindowLayout;
  WindowLayout m_MultiWindowLayout;

  DnDDisplayInteractor::Pointer m_DisplayInteractor;

  /**
   * Reference to the service registration of the display interactor.
   * It is needed to unregister the observer on unload.
   */
  us::ServiceRegistrationU m_DisplayInteractorService;
};

}

#endif
