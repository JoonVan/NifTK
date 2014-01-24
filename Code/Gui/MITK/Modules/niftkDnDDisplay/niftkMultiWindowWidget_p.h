/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkMultiWindowWidget_h
#define niftkMultiWindowWidget_h

//#include <itkConversionUtils.h>

#include <QColor>

#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkGeometry3D.h>
#include <mitkVector.h>
#include <QmitkStdMultiWidget.h>

#include "Interactions/mitkDnDDisplayInteractor.h"

#include <mitkMIDASEnums.h>
#include <niftkDnDDisplayEnums.h>

class QGridLayout;
class QStackedLayout;
class DisplayGeometryModificationCommand;

class vtkRenderer;
class vtkSideAnnotation;

namespace mitk
{
class SliceNavigationController;
}

/**
 * \class niftkMultiWindowWidget
 * \brief Subclass of QmitkStdMultiWidget to provide MIDAS specific functionality
 * by having convenient methods to control geometry, background, cursors on/off etc.
 * via calling methods in the base class QmitkStdMultiWidget.
 *
 * In MIDAS terms, the widget will nearly always be in Axial, Coronal or Sagittal mode, but we
 * subclass QmitkStdMultiWidget so that we can optionally have 3D views, ortho-views etc.
 *
 * Please do NOT expose this class to the rest of the NiftyView code-base, or else
 * dependency management becomes a bit of an issue.  The class niftkSingleViewerWidget
 * wraps this one, and the rest of our application should only deal with
 * niftkSingleViewerWidget.
 *
 * Note: The requirements specification for MIDAS style zoom basically says:
 * <pre>
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
 * In contrast with the original MIDAS, in NiftyMIDAS the zooming is continuous, so
 * the rule above is extended to real numbers as well. If the image has anisotropic pixels then
 * the size of the longest side of the voxels is used for the calculation.
 *
 * \sa QmitkStdMultiWidget
 * \sa niftkSingleViewerWidget
 * \sa niftkMultiViewerWidget
 */
class niftkMultiWindowWidget : public QmitkStdMultiWidget
{

  Q_OBJECT

public:

  /// \brief Constructor.
  niftkMultiWindowWidget(QWidget* parent = 0,
                           Qt::WindowFlags f = 0,
                           mitk::RenderingManager* renderingManager = 0,
                           mitk::DataStorage* dataStorage = 0);

  /// \brief Destructor.
  virtual ~niftkMultiWindowWidget();

  /// \brief There are several things we turn off/on depending on whether the widget is
  /// visible or considered active, so we group them all under this Enabled(true/false) flag.
  void SetEnabled(bool b);

  /// \brief Return whether this widget is considered 'enabled'.
  bool IsEnabled() const;

  /// \brief Turn the 2D cursors visible/invisible for this viewer (renderer specific properties).
  void SetCursorVisible(bool visible);

  /// \brief Tells if the direction annotations are visible.
  bool AreDirectionAnnotationsVisible() const;

  /// \brief Sets the visibility of the direction annotations.
  void SetDirectionAnnotationsVisible(bool visible);

  /// \brief Get the flag controlling the 2D cursors visibility (renderer specific properties).
  bool IsCursorVisible() const;

  /// \brief Turn the 2D cursors visible/invisible globally, controlled by a user preference.
  void SetCursorGloballyVisible(bool visible);

  /// \brief Get the flag controlling 2D cursors global visibility.
  bool IsCursorGloballyVisible() const;

  /// \brief If true, then nodes will be visible in 3D window when in 2x2 window layout. In 3D window layout, always visible.
  void SetShow3DWindowIn2x2WindowLayout(bool visible);

  /// \brief Returns the flag indicating if nodes will be visible in 3D window when in 2x2 window layout. In 3D window layout, always visible.
  bool GetShow3DWindowIn2x2WindowLayout() const;

  /// \brief Initialises the geometry in the QmitkStdMultiWidget base class.
  /// This has been a difficult method to get to work properly. Developers should look at the code comments.
  void SetGeometry(mitk::TimeGeometry* geometry);

  /// \brief Switches the window layout, i.e. the set and the arrangement of the render windows.
  void SetWindowLayout(WindowLayout windowLayout);

  /// \brief Gets the window layout, i.e. the set and the arrangement of the render windows.
  /// The MIDAS functionality is only interested in those orientations given by this enum,
  /// currently ax, sag, cor, ortho, 3D, 3H, 3V.
  WindowLayout GetWindowLayout() const;

  /// \brief Works out the orientation of the current window layout.
  MIDASOrientation GetOrientation();

  /// \brief Set the background color, applied to 2D and 3D windows, and currently we don't do gradients.
  void SetBackgroundColor(QColor color);

  /// \brief Get the background color, applied to 2D and 3D windows, and currently we don't do gradients.
  QColor GetBackgroundColor() const;

  /// \brief If selected, this widget is "selected" meaning its selected render window (if any)
  /// will have coloured border, otherwise it is not selected, and will not have coloured borders
  /// even if one of its render window was selected.
  void SetSelected(bool selected);

  /// \brief Returns true if this widget is selected and false otherwise.
  bool IsSelected() const;

  /// \brief Returns the selected window, that is the one with the coloured border.
  QmitkRenderWindow* GetSelectedRenderWindow() const;

  /// \brief Selects the render window and puts put a coloured border round it.
  void SetSelectedRenderWindow(QmitkRenderWindow* renderWindow);

  /// \brief Returns the specifically selected render window, which may be 1 if the viewer is
  /// showing a single axial, coronal or sagittal plane, or may be up to 4 if the viewer
  /// is displaying the 2x2 window layout.
  std::vector<QmitkRenderWindow*> GetVisibleRenderWindows() const;

  /// \brief Returns the list of all QmitkRenderWindow contained herein.
  std::vector<QmitkRenderWindow*> GetRenderWindows() const;

  /// \brief Gets the render window corresponding to the given orientation, or NULL if it can't be found.
  QmitkRenderWindow* GetRenderWindow(const MIDASOrientation& orientation) const;

  /// \brief Gets the orientation corresponding to the given render window.
  /// Returns MIDAS_ORIENTATION_UNKNOWN for the 3D window.
  MIDASOrientation GetOrientation(const QmitkRenderWindow* renderWindow) const;

  /// \brief Returns true if this widget contains the provided window and false otherwise.
  bool ContainsRenderWindow(QmitkRenderWindow* renderWindow) const;

  /// \brief Returns the render window that has the given VTK render window, or NULL if there is not any.
  QmitkRenderWindow* GetRenderWindow(vtkRenderWindow* aVtkRenderWindow) const;

  /// \brief Returns the maximum allowed slice index for a given orientation.
  unsigned int GetMaxSliceIndex(MIDASOrientation orientation) const;

  /// \brief Returns the maximum allowed time step.
  unsigned int GetMaxTimeStep() const;

  /// \brief Get the current slice index.
  unsigned int GetSliceIndex(MIDASOrientation orientation) const;

  /// \brief Set the current slice index.
  void SetSliceIndex(MIDASOrientation orientation, unsigned int sliceIndex);

  /// \brief Get the current time step.
  unsigned int GetTimeStep() const;

  /// \brief Set the current time step.
  void SetTimeStep(unsigned int timeStep);

  /// \brief Gets the selected position in the world coordinate system (mm).
  const mitk::Point3D GetSelectedPosition() const;

  /// \brief Sets the selected position in the world coordinate system (mm).
  void SetSelectedPosition(const mitk::Point3D& selectedPosition);

//  /// \brief Gets the cursor position normalised with the render window size.
//  /// The values are in the [0.0, 1.0] range and represent the position inside the render window:
//  ///
//  ///    pixel coordinate / render window size
//  ///
//  mitk::Vector3D GetCursorPosition(QmitkRenderWindow* renderWindow) const;

//  /// \brief Sets the cursor position normalised with the render window size.
//  /// The values are in the [0.0, 1.0] range and represent the position inside the render window:
//  ///
//  ///    pixel coordinate / render window size
//  ///
//  void SetCursorPosition(QmitkRenderWindow* renderWindow, const mitk::Vector3D& cursorPosition);

  /// \brief Gets the cursor position normalised with the render window size.
  /// The values are in the [0.0, 1.0] range and represent the position inside the render window:
  ///
  ///    pixel coordinate / render window size
  ///
  const mitk::Vector2D GetCursorPosition2D(QmitkRenderWindow* renderWindow) const;

  /// \brief Sets the cursor position normalised with the render window size.
  /// The values are in the [0.0, 1.0] range and represent the position inside the render window:
  ///
  ///    pixel coordinate / render window size
  ///
  void SetCursorPosition2D(QmitkRenderWindow* renderWindow, const mitk::Vector2D& cursorPosition);

  /// \brief Gets the cursor position normalised with the render window size.
  /// The values are in the [0.0, 1.0] range and represent the position inside the render window:
  ///
  ///    pixel coordinate / render window size
  ///
  mitk::Vector2D GetCursorPosition(MIDASOrientation orientation) const;

  /// \brief Sets the cursor position normalised with the render window size.
  /// The values are in the [0.0, 1.0] range and represent the position inside the render window:
  ///
  ///    pixel coordinate / render window size
  ///
  void SetCursorPosition(MIDASOrientation, const mitk::Vector2D& cursorPosition);

//  /// \brief Gets the cursor position normalised with the size of the render windows.
//  /// The values are in the [0.0, 1.0] range and represent the position inside the render windows:
//  ///
//  ///    pixel coordinate / render window size
//  ///
//  /// The first two coordinates correspond to the coordinates in the axial render window. The third
//  /// coordinate corresponds to the second coordinate in the sagittal render window.
//  /// The correspondence of the orientation axes is the following:
//  ///
//  ///     axial[0] <-> coronal[0]
//  ///     axial[1] <-> 1.0 - sagittal[0]
//  ///     sagittal[1] <-> coronal[1]
//  ///
//  const mitk::Vector3D& GetCursorPosition() const;

//  /// \brief Moves the image (world) in the render windows so that the selected position gets to the
//  /// specified position in the render windows. The function does not change the selected position.
//  /// The values are in the [0.0, 1.0] range and represent the position inside the render windows:
//  ///
//  ///    pixel coordinate / render window size
//  ///
//  /// The first two coordinates correspond to the coordinates in the axial render window. The third
//  /// coordinate corresponds to the second coordinate in the sagittal render window.
//  /// The correspondence of the orientation axes is the following:
//  ///
//  ///     axial[0] <-> coronal[0]
//  ///     axial[1] <-> 1.0 - sagittal[0]
//  ///     sagittal[1] <-> coronal[1]
//  ///
//  void SetCursorPosition(const mitk::Vector3D& cursorPosition);

  /// \brief Gets the scale factor of the given render window. (mm/px)
  double GetScaleFactor(QmitkRenderWindow* renderWindow) const;

  /// \brief Sets the scale factor of the render window to the given value (mm/px)
  /// and moves the image so that the position of the focus remains the same.
  void SetScaleFactor(QmitkRenderWindow* renderWindow, double scaleFactor);

  /// \brief Gets the scale factor of the selected render window or 0.0 if no
  /// window is selected.
  double GetScaleFactor() const;

  /// \brief Sets the scale factor of the selected window to the given value.
  /// If the zooming is bound across the windows then this will set the scaling
  /// of the other windows as well.
  void SetScaleFactor(double scaleFactor);

  /// \brief Gets the voxel size (mm/vx).
  const mitk::Vector3D& GetVoxelSize() const;

  /// \brief Gets the "Magnification Factor", which is a MIDAS term describing how many screen pixels per image voxel (px/vx).
  double GetMagnification() const;

  /// \brief Sets the "Magnification Factor", which is a MIDAS term describing how many screen pixels per image voxel (px/vx).
  void SetMagnification(double magnification);

  /// \brief Computes the magnification of a render window.
  double GetMagnification(QmitkRenderWindow* renderWindow) const;

  /// \brief Sets the magnification of a render window to the given value.
  void SetMagnification(QmitkRenderWindow* renderWindow, double magnification);

  /// \brief Only to be used for Thumbnail mode, makes the displayed 2D geometry fit the display window.
  void FitToDisplay();

  /// \brief Sets the visible flag for all the nodes, and all the renderers in the QmitkStdMultiWidget base class.
  void SetRendererSpecificVisibility(std::vector<mitk::DataNode*> nodes, bool visible);

  /// \brief Only request an update for screens that are visible and enabled.
  void RequestUpdate();

  /// \brief According to the currently set geometry will return +1, or -1 for the direction to increment the slice number to move "up".
  ///
  /// \see mitkMIDASOrientationUtils.
  int GetSliceUpDirection(MIDASOrientation orientation) const;

  /// \brief Sets the flag that controls whether the display interactions are enabled for the render windows.
  void SetDisplayInteractionsEnabled(bool enabled);

  /// \brief Gets the flag that controls whether the display interactions are enabled for the render windows.
  bool AreDisplayInteractionsEnabled() const;

  /// \brief Gets the flag that controls whether the cursor position is bound between the 2D render windows.
  bool AreCursorPositionsBound() const;

  /// \brief Sets the flag that controls whether the cursor position is bound between the 2D render windows.
  void SetCursorPositionsBound(bool bound);

  /// \brief Gets the flag controls whether the scale factors are bound across the 2D render windows.
  bool AreScaleFactorsBound() const;

  /// \brief Sets the flag that controls whether the scale factors are bound across the 2D render windows.
  void SetScaleFactorsBound(bool bound);

signals:

  /// \brief Emits a signal to say that this widget/window has had the following nodes dropped on it.
  void NodesDropped(QmitkRenderWindow* renderWindow, std::vector<mitk::DataNode*> nodes);

  /// \brief Emitted when the selected slice has changed in a render window.
//  void SelectedPositionChanged(QmitkRenderWindow* renderWindow, int sliceIndex);
  void SelectedPositionChanged(const mitk::Point3D& selectedPosition);

//  /// \brief Emitted when the cursor position has changed.
//  void CursorPositionChanged(const mitk::Vector3D& cursorPosition);

  /// \brief Emitted when the cursor position has changed.
  void CursorPositionChanged(MIDASOrientation orientation, const mitk::Vector2D& cursorPosition);

  /// \brief Emitted when the scale factor has changed.
  void ScaleFactorChanged(double scaleFactor);

protected slots:

  /// \brief The 4 individual render windows get connected to this slot, and then all emit NodesDropped.
  void OnNodesDropped(QmitkRenderWindow* renderWindow, std::vector<mitk::DataNode*> nodes);

private:

  /// \brief Callback from internal Axial SliceNavigatorController
  void OnAxialSliceChanged(const itk::EventObject& geometrySliceEvent);

  /// \brief Callback from internal Sagittal SliceNavigatorController
  void OnSagittalSliceChanged(const itk::EventObject& geometrySliceEvent);

  /// \brief Callback from internal Coronal SliceNavigatorController
  void OnCoronalSliceChanged(const itk::EventObject& geometrySliceEvent);

  /// \brief Callback, called from OnAxialSliceChanged, OnSagittalSliceChanged, OnCoronalSliceChanged to emit SelectedPositionChanged
  void OnSelectedPositionChanged(MIDASOrientation orientation);

  /// \brief Method to update the visibility property of all nodes in 3D window.
  void Update3DWindowVisibility();

  /// \brief Returns the current slice navigation controller, and calling it is only valid if the widget is displaying one render window (i.e. either axial, coronal, sagittal).
  mitk::SliceNavigationController* GetSliceNavigationController(MIDASOrientation orientation) const;

  /// \brief For the given window and the list of nodes, will set the renderer specific visibility property, for all the contained renderers.
  void SetVisibility(QmitkRenderWindow* renderWindow, mitk::DataNode* node, bool visible);

  // \brief Sets the origin of the display geometry of the render window
  void SetOrigin(QmitkRenderWindow* renderWindow, const mitk::Vector2D& originInMm);

  /// \brief Scales a specific render window about the cursor. The zoom factor is the ratio of the required
  /// and the current scale factor.
  /// \deprecated
  /// {
  ///   This function is deprecated because it requires the 'relative' scale factor.
  ///   Use the SetScaleFactor functions instead.
  /// }
  void ZoomDisplayAboutCursor(QmitkRenderWindow* renderWindow, double zoomFactor);

  /// \brief Returns a scale factor describing how many pixels on screen correspond to a single voxel or millimetre.
  /// \deprecated
  /// {
  ///   This should be calculated from the world geometry dimensions, display geometry dimensions
  ///   and the scale factor of the display geometry.
  /// }
  void GetScaleFactors(QmitkRenderWindow* renderWindow, mitk::Vector2D& scaleFactorPxPerVx, mitk::Vector2D& scaleFactorPxPerMm);

  /// \brief Adds a display geometry observer to the render window. Used to synchronise panning and zooming.
  void AddDisplayGeometryModificationObserver(QmitkRenderWindow* renderWindow);

  /// \brief Removes a display geometry observer from the render window. Used to synchronise panning and zooming.
  void RemoveDisplayGeometryModificationObserver(QmitkRenderWindow* renderWindow);

  /// \brief Called when the origin of the display geometry of the render window has changed.
  void OnOriginChanged(QmitkRenderWindow* renderWindow, bool beingPanned);

  /// \brief Called when the scale factor of the display geometry of the render window has changed.
  void OnFocusChanged(QmitkRenderWindow* renderWindow, const mitk::Vector2D& focusPoint);

  /// \brief Called when the scale factor of the display geometry of the render window has changed.
  void OnScaleFactorChanged(QmitkRenderWindow* renderWindow, double scaleFactor);

//  /// \brief Computes the origin for a render window from the cursor position.
//  mitk::Vector2D ComputeOriginFromCursorPosition(QmitkRenderWindow* renderWindow, const mitk::Vector3D& cursorPosition);

  /// \brief Computes the origin for a render window from the cursor position.
  mitk::Vector2D ComputeOriginFromCursorPosition(QmitkRenderWindow* renderWindow, const mitk::Vector2D& cursorPosition);

  /// \brief Computes the zoom factor for a render window from a magnification factor.
  /// The zoom factor is the ratio of the required and the current scale factor.
  /// \deprecated
  /// {
  ///   This function is deprecated because it needs to know the current scaling in the render window.
  ///   The function was used to compute the zoom factor for the ZoomDisplayAboutCursor function that
  ///   has been deprecated as well. Use the ComputeScaleFactors function to calculate the absolute
  ///   scale factors from the magnification and the SetScaleFactor function to set the required
  ///   scale factor for a render window.
  /// }
  double ComputeZoomFactor(QmitkRenderWindow* renderWindow, double magnification);

  /// \brief Computes the scale factors from the magnification for each axes in mm/px.
  /// Since the magnification is in linear relation with the px/vx ratio but not the
  /// voxel size, the three scale factors can differ if the image has anisotropic voxels.
  /// The voxel sizes are calculated when the geometry is set.
  mitk::Vector3D ComputeScaleFactors(double magnification);

  /// \brief The magnification is calculated with the longer voxel side of an orientation.
  /// This function returns the index of this axis.
  int GetDominantAxis(MIDASOrientation orientation) const;

  QmitkRenderWindow* m_RenderWindows[4];
  QColor m_BackgroundColor;
  QGridLayout* m_GridLayout;
  unsigned m_AxialSliceTag;
  unsigned m_SagittalSliceTag;
  unsigned m_CoronalSliceTag;
  bool m_IsSelected;
  bool m_IsEnabled;
  QmitkRenderWindow* m_SelectedRenderWindow;
  bool m_CursorVisibility;
  bool m_CursorGlobalVisibility;
  bool m_Show3DWindowIn2x2WindowLayout;
  WindowLayout m_WindowLayout;
  mitk::Point3D m_SelectedPosition;
  std::vector<mitk::Vector2D> m_CursorPositions;
//  mitk::Vector3D m_CursorPosition;
  double m_Magnification;
  double m_ScaleFactor;
  mutable std::map<MIDASOrientation, int> m_OrientationToAxisMap;
  mitk::Geometry3D* m_Geometry;
  mitk::TimeGeometry* m_TimeGeometry;

  /// \brief Voxel size in millimetres.
  mitk::Vector3D m_MmPerVx;

  vtkSideAnnotation* m_DirectionAnnotations[3];
  vtkRenderer* m_DirectionAnnotationRenderers[3];

  std::map<QmitkRenderWindow*, unsigned long> m_DisplayGeometryModificationObservers;
  bool m_BlockDisplayGeometryEvents;

  bool m_CursorPositionsAreBound;
  bool m_ScaleFactorsAreBound;

  friend class DisplayGeometryModificationCommand;

  mitk::DnDDisplayInteractor::Pointer m_DisplayInteractor;

  /**
   * Reference to the service registration of the display interactor.
   * It is needed to unregister the observer on unload.
   */
  us::ServiceRegistrationU m_DisplayInteractorService;
};

#endif
