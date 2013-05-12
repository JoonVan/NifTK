/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef QmitkMIDASSingleViewWidget_h
#define QmitkMIDASSingleViewWidget_h

#include <niftkMIDASGuiExports.h>
#include "mitkMIDASEnums.h"
#include "QmitkMIDASStdMultiWidget.h"
#include <QWidget>
#include <QColor>
#include <mitkDataStorage.h>
#include <mitkGeometry3D.h>
#include <mitkRenderingManager.h>
#include <QmitkRenderWindow.h>

class QGridLayout;

/**
 * \class QmitkMIDASSingleViewWidget
 * \brief A widget to wrap a single QmitkMIDASStdMultiWidget view,
 * providing methods for switching the view mode, remembering the last slice,
 * magnification and in the future camera position.
 *
 * IMPORTANT: This class acts as a wrapper for QmitkMIDASStdMultiWidget.
 * Do not expose QmitkMIDASStdMultiWidget, or any member variables, or any
 * dependency from QmitkMIDASStdMultiWidget to the rest of the application.
 *
 * Additionally, this widget contains its own mitk::RenderingManager which is passed to the
 * QmitkMIDASStdMultiWidget, which is itself a sub-class of QmitkStdMultiWidget.
 * This means the QmitkMIDASStdMultiWidget will update and render independently of the
 * rest of the application, and care must be taken to manage this. The reason is that
 * each of these windows in a MIDAS layout could have it's own geometry, and sometimes
 * a very different geometry from other windows, and then when the "Bind Slices" button
 * is clicked, they must all align to a specific (the currently selected Window) geometry.
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
 * \sa QmitkMIDASStdMultiWidget
 */
class NIFTKMIDASGUI_EXPORT QmitkMIDASSingleViewWidget : public QWidget {

  /// \brief Defining Q_OBJECT macro, so we can register signals and slots if needed.
  Q_OBJECT;

public:

  friend class QmitkMIDASSegmentationViewWidget;

  QmitkMIDASSingleViewWidget(QWidget *parent);

  QmitkMIDASSingleViewWidget(QString windowName,
                             double minimumMagnification,
                             double maximumMagnification,
                             QWidget *parent = 0,
                             mitk::RenderingManager* renderingManager = 0,
                             mitk::DataStorage* dataStorage = 0
                             );
  ~QmitkMIDASSingleViewWidget();

  /// \brief Returns true if the view is axial, coronal or sagittal and false otherwise (e.g. if ortho-view or 3D view).
  bool IsSingle2DView() const;

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
  std::vector<QmitkRenderWindow*> GetRenderWindows() const;

  /// \brief Returns the Axial Window.
  QmitkRenderWindow* GetAxialWindow() const;

  /// \brief Returns the Coronal Window.
  QmitkRenderWindow* GetCoronalWindow() const;

  /// \brief Returns the Sagittal Window.
  QmitkRenderWindow* GetSagittalWindow() const;

  /// \brief Returns the 3D Window.
  QmitkRenderWindow* Get3DWindow() const;

  /// \brief Returns the orientation for the selected window, returning MIDAS_ORIENTATION_UNKNOWN if not axial, sagittal or coronal.
  MIDASOrientation GetOrientation();

  /// \brief Turn the 2D cursors on/off locally.
  void SetDisplay2DCursorsLocally(bool visible);

  /// \brief Get the flag controlling 2D cursors on/off.
  bool GetDisplay2DCursorsLocally() const;

  /// \brief Turn the 2D cursors on/off globally.
  void SetDisplay2DCursorsGlobally(bool visible);

  /// \brief Get the flag controlling 2D cursors on/off.
  bool GetDisplay2DCursorsGlobally() const;

  /// \brief Returns the flag indicating if nodes will be visible in 3D window when in ortho (2x2) view. In 3D view, always visible.
  bool GetShow3DWindowInOrthoView() const;

  /// \brief If true, then nodes will be visible in 3D window when in ortho (2x2) view. In 3D view, always visible.
  void SetShow3DWindowInOrthoView(bool enabled);

  /// \brief Sets a flag to determin if we remember the view settings such as slice, cursor position, magnification, time step when we switch between views axial, coronal, sagittal.
  void SetRememberViewSettingsPerOrientation(bool remember);

  /// \brief Get the flag to determin if we remember the view settings such as slice, cursor position, magnification, time step when we switch between views axial, coronal, sagittal.
  bool GetRememberViewSettingsPerOrientation() const;

  /// \brief Sets the background colour.
  void SetBackgroundColor(QColor color);

  /// \brief Gets the background colour.
  QColor GetBackgroundColor() const;

  /// \brief Returns the minimum allowed slice number for a given orientation.
  unsigned int GetMinSlice(MIDASOrientation orientation) const;

  /// \brief Returns the maximum allowed slice number for a given orientation.
  unsigned int GetMaxSlice(MIDASOrientation orientation) const;

  /// \brief Gets the minimum time step, or -1 if the widget is currently showing multiple views.
  unsigned int GetMinTime() const;

  /// \brief Gets the maximum time step, or -1 if the widget is currently showing multiple views.
  unsigned int GetMaxTime() const;

  /// \brief Returns true if the widget is fully created and contains the given render window, and false otherwise.
  bool ContainsRenderWindow(QmitkRenderWindow *renderWindow) const;

  /// \brief Returns the render window that has the given VTK render window, or NULL if there is not any.
  QmitkRenderWindow* GetRenderWindow(vtkRenderWindow *aVtkRenderWindow) const;

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

  /// \brief Sets the world geometry that we are sampling.
  void SetGeometry(mitk::Geometry3D::Pointer geometry);

  /// \brief Gets the world geometry, to pass to other viewers for when slices are bound.
  mitk::Geometry3D::Pointer GetGeometry();

  /// \brief Sets the world geometry that we are sampling when we are in bound mode.
  void SetBoundGeometry(mitk::Geometry3D::Pointer geometry);

  /// \brief If we tell the widget to be in bound mode, it uses the bound geometries.
  void SetBoundGeometryActive(bool isBound);

  /// \brief Returns the bound flag.
  bool GetBoundGeometryActive();

  /// \brief Get the current slice number for a given orientation.
  unsigned int GetSliceNumber(MIDASOrientation orientation) const;

  /// \brief Set the current slice number for a given orientation.
  void SetSliceNumber(MIDASOrientation orientation, unsigned int sliceNumber);

  /// \brief Get the current time step number.
  unsigned int GetTime() const;

  /// \brief Set the current time step number.
  void SetTime(unsigned int timeSlice);

  /// \brief Get the view ID.
  MIDASView GetView() const;

  /// \brief Sets the view to either axial, sagittal or coronal, 3D or ortho etc, effectively causing a view reset.
  void SetView(MIDASView view, bool fitToDisplay);

  /// \brief In contrast to SetView this method does as little as possible, to be analagous to just switching the orientation.
  void SwitchView(MIDASView view);

  /// \brief Get the currently selected position in world coordinates (mm)
  mitk::Point3D GetSelectedPosition() const;

  /// \brief Set the currently selected position in world coordinates (mm)
  void SetSelectedPosition(const mitk::Point3D& selectedPosition);

  /// \brief Get the current cursor position on the render window in pixels, normalised with the size of the render windows.
  const mitk::Vector3D& GetCursorPosition() const;

  /// \brief Set the current cursor position on the render window in pixels, normalised with the size of the render windows.
  void SetCursorPosition(const mitk::Vector3D& cursorPosition);

  /// \brief Get the current magnification factor.
  double GetMagnification() const;

  /// \brief Set the current magnification factor.
  void SetMagnification(double magnification);

  /// \brief Sets the flag controlling whether we are listening to the navigation controller events.
  void SetNavigationControllerEventListening(bool enabled);

  /// \brief Gets the flag controlling whether we are listening to the navigation controller events.
  bool GetNavigationControllerEventListening() const;

  /// \brief Sets the flag controlling whether the display interactors are enabled for the render windows.
  void SetDisplayInteractionEnabled(bool enabled);

  /// \brief Gets the flag controlling whether the display interactors are enabled for the render windows.
  bool IsDisplayInteractionEnabled() const;

  /// \brief Only to be used for Thumbnail mode, makes the displayed 2D geometry fit the display window.
  void FitToDisplay();

  /// \brief Returns pointers to the widget planes.
  std::vector<mitk::DataNode*> GetWidgetPlanes();

  /// \brief According to the currently set geometry will return +1, or -1 for the direction to increment the slice number to move "up".
  ///
  /// \see mitkMIDASOrientationUtils.
  int GetSliceUpDirection(MIDASOrientation orientation) const;

protected:

  virtual void paintEvent(QPaintEvent *event);

signals:

  /// \brief Emitted when nodes are dropped on the SingleView widget.
  void NodesDropped(QmitkRenderWindow *window, std::vector<mitk::DataNode*> nodes);
  void SelectedPositionChanged(QmitkMIDASSingleViewWidget *widget, QmitkRenderWindow *window, int sliceNumber);
  void CursorPositionChanged(QmitkMIDASSingleViewWidget *widget, const mitk::Vector3D& cursorPosition);
  void MagnificationChanged(QmitkMIDASSingleViewWidget *widget, double magnification);

protected slots:

  // Called when nodes are dropped on the contained render windows.
  virtual void OnNodesDropped(QmitkMIDASStdMultiWidget *widget, QmitkRenderWindow *window, std::vector<mitk::DataNode*> nodes);
  virtual void OnSelectedPositionChanged(QmitkRenderWindow* window, int sliceNumber);
  virtual void OnCursorPositionChanged(const mitk::Vector3D& cursorPosition);
  virtual void OnMagnificationChanged(double magnification);

private:


  /// \brief Provided here to provide access to the QmitkStdMultiWidget::InitializeStandardViews for friend classes only.
  void InitializeStandardViews(const mitk::Geometry3D * geometry );

  /// \brief This method is called from both constructors to do the construction.
  void Initialize(QString windowName,
                  mitk::RenderingManager* renderingManager = 0,
                  mitk::DataStorage* dataStorage = 0
                 );

  void SetActiveGeometry();
  inline int Index(int index) const
  {
    return (index << 1) + m_IsBound;
  }
  void StorePosition();
  void ResetCurrentPosition();
  void ResetRememberedPositions();

  mitk::DataStorage::Pointer           m_DataStorage;
  mitk::RenderingManager::Pointer      m_RenderingManager;

  QGridLayout                         *m_Layout;
  QmitkMIDASStdMultiWidget            *m_MultiWidget;

  bool                                 m_IsBound;
  mitk::Geometry3D::Pointer            m_UnBoundGeometry;              // This comes from which ever image is dropped, so not visible outside this class.
  mitk::Geometry3D::Pointer            m_BoundGeometry;                // Passed in, when we do "bind", so shared amongst multiple windows.
  mitk::Geometry3D::Pointer            m_ActiveGeometry;               // The one we actually use, which points to either of the two above.

  double                               m_MinimumMagnification;         // Passed in as constructor arguments, so this class unaware of where it came from.
  double                               m_MaximumMagnification;         // Passed in as constructor arguments, so this class unaware of where it came from.

  MIDASView                            m_View;
  MIDASOrientation                     m_Orientation;

  int                                  m_SliceNumbers[MIDAS_ORIENTATION_NUMBER * 2];     // Two for each orientation. Unbound, then bound, alternatingly.
  int                                  m_TimeSliceNumbers[MIDAS_ORIENTATION_NUMBER * 2]; // Two for each orientation. Unbound, then bound, alternatingly.
  mitk::Vector3D                       m_CursorPositions[MIDAS_VIEW_NUMBER * 2]; // Two each for view. Unbound, then bound, alternatingly.
  double                               m_Magnifications[MIDAS_VIEW_NUMBER * 2];    // Two each for view. Unbound, then bound, alternatingly.
  bool                                 m_ViewInitialised[MIDAS_VIEW_NUMBER * 2];         // Two each for view. Unbound, then bound, alternatingly.

  bool                                 m_NavigationControllerEventListening;
  bool                                 m_RememberViewSettingsPerOrientation;
};

#endif