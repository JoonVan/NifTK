/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-12-16 09:12:58 +0000 (Fri, 16 Dec 2011) $
 Revision          : $Revision: 8039 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef QMITKTHUMBNAILRENDERWINDOW_H_
#define QMITKTHUMBNAILRENDERWINDOW_H_

#include "niftkQmitkExtExports.h"
#include <QColor>
#include "QmitkRenderWindow.h"
#include "mitkDataStorage.h"
#include "mitkDataNode.h"
#include "mitkCuboid.h"
#include "mitkAffineInteractor.h"

class QmitkMouseEventEater;
class QmitkWheelEventEater;

/**
 * \class QmitkThumbnailRenderWindow
 * \brief Subclass of QmitkRenderWindow to listen to the currently focussed QmitkRenderWindow
 * and provide a zoomed-out view with an overlay of a bounding box to provide the
 * current size of the currently focussed QmitkRenderWindow's viewport size.
 *
 * The client must provide an mitk::DataStorage before using this widget.
 * So the correct usage is to construct the object and immediately set the DataStorage.
 * When the DataStorage is set, the widget also responds to any ChangedNodeEvent events.
 *
 * This class provides methods to set the bounding box colour, opacity, line thickness,
 * and rendering layer. It also has methods as to whether we respond to mouse or wheel events.
 * By default the design was to not allow wheel events, as this would cause the slice to change,
 * which should then be propagated back to all other windows, and we don't know which windows
 * are listening, or need updating. However, with regards to left mouse, click and move events,
 * where the user is selecting the focus, then this is automatically passed to the global
 * mitk::FocusManager which propagates to all registered views. So mouse events default to on.
 *
 * \sa QmitkRenderWindow
 * \sa mitk::DataStorage
 * \sa mitk::FocusManager
 */
class NIFTKQMITKEXT_EXPORT QmitkThumbnailRenderWindow : public QmitkRenderWindow
{
  Q_OBJECT

public:

  /// \brief Constructor creates the bounding box, event filters and registers with the global mitk::FocusManager.
  QmitkThumbnailRenderWindow(QWidget *parent);

  /// \brief Destructor removes various geometry observers, and deletes event filters.
  ~QmitkThumbnailRenderWindow();

  /// \brief A valid dataStorage must be passed in so this method does assert(dataStorage).
  void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

  /// \brief Gets the bounding box color, default is red.
  QColor boundingBoxColor() const;

  /// \brief Sets the bounding box color, default is red.
  void setBoundingBoxColor(QColor &color);

  /// \brief Sets the bounding box color, default is red.
  void setBoundingBoxColor(float r, float g, float b);

  /// \brief Sets the bounding box line thickness, default is 1 pixel, but on some displays (eg. various Linux) may appear wider due to anti-aliasing.
  int boundingBoxLineThickness() const;

  /// \brief Gets the bounding box line thickness, default is 1 pixel.
  void setBoundingBoxLineThickness(int thickness);

  /// \brief Gets the bounding box opacity, default is 1.
  float boundingBoxOpacity() const;

  /// \brief Sets the bounding box opacity, default is 1.
  void setBoundingBoxOpacity(float opacity);

  /// \brief Gets the bounding box layer, default is 99.
  int boundingBoxLayer() const;

  /// \brief Sets the bounding box layer, default is 99.
  void setBoundingBoxLayer(int layer);

  /// \brief Sets the bounding box visibility, default is true.
  void setBoundingBoxVisible(bool visible);

  /// \brief Gets the bounding box visibility, default is true.
  bool boundingBoxVisible() const;

  /// \brief Sets whether to resond to mouse events, default is on.
  void setRespondToMouseEvents(bool on);

  /// \brief Gets whether to resond to mouse events, default is on.
  bool respondToMouseEvents() const;

  /// \brief Sets whether to resond to wheel events, default is off.
  void setRespondToWheelEvents(bool on);

  /// \brief Gets whether to resond to wheel events, default is off.
  bool respondToWheelEvents() const;

  /// \brief Called when a DataStorage Add Event was emmitted and sets m_InDataStorageChanged to true and calls NodeAdded afterwards.
  void NodeAddedProxy(const mitk::DataNode* node);

  /// \brief Called when a DataStorage Change Event was emmitted and sets m_InDataStorageChanged to true and calls NodeChanged afterwards.
  void NodeChangedProxy(const mitk::DataNode* node);

protected:

  /// \brief Called when a DataStorage Add event was emmitted and may be reimplemented by deriving classes.
  virtual void NodeAdded(const mitk::DataNode* node);

  /// \brief Called when a DataStorage Change event was emmitted and may be reimplemented by deriving classes.
  virtual void NodeChanged(const mitk::DataNode* node);

private:

  // Callback for when the focus changes, where we update the thumbnail view to the right window, then call UpdateWorldGeometry(), UpdateBoundingBox() and OnVisibilityChanged().
  void OnFocusChanged();

  // Callback for when the display geometry of a window changes, where we only update the thumbnail bounding box.
  void OnDisplayGeometryChanged();

  // Callback for when the slice selector changes slice, where we change the world geometry to get the right slice.
  void OnSliceChanged(const itk::EventObject & geometrySliceEvent);

  // When the world geometry changes, we have to make the thumbnail match, to get the same slice.
  void UpdateWorldGeometry(bool fitToDisplay);

  // When any visibility flag changes we recompute which objects are visible in this render window.
  void UpdateVisibility();

  // Updates the bounding box by taking the 4 corners of the focussed render window, calling Get3DPoint(), and updating the bounding box.
  void UpdateBoundingBox();

  // Converts 2D pixel point to 3D millimetre point using MITK methods.
  mitk::Point3D Get3DPoint(int x, int y);

  // Internal method, so that any time we need the mitk::DataStorage we go via this method, which checks assert(m_DataStorage).
  mitk::DataStorage::Pointer GetDataStorage();

  // Used for the mitkFocusManager to register callbacks to track the currently focus window.
  unsigned long m_FocusManagerObserverTag;

  // Used for when the focused window display geometry changes.
  unsigned long m_FocusedWindowDisplayGeometryTag;

  // Used for when the focused window changes slice.
  unsigned long m_FocusedWindowSliceSelectorTag;

  // We need to provide access to data storage to listen to Node events.
  mitk::DataStorage::Pointer m_DataStorage;

  // Stores a bounding box node, which this class owns and manages.
  mitk::DataNode::Pointer m_BoundingBoxNode;

  // The actual bounding box, which this class owns and manages.
  mitk::Cuboid::Pointer m_BoundingBox;

  // We do a lot with renderer specific properties, so Im storing the one from the widget, as it is fixed.
  mitk::BaseRenderer::Pointer m_BaseRenderer;

  // This is set to the currently tracked window. We don't construct or own it, so don't delete it.
  vtkRenderWindow *m_TrackedRenderWindow;

  // Keep track of this to register and unregister event listeners.
  mitk::DisplayGeometry::Pointer m_TrackedDisplayGeometry;

  // Keep track of this to register and unregister event listeners.
  mitk::SliceNavigationController::Pointer m_TrackedSliceNavigator;

  // Squash all mouse events.
  QmitkMouseEventEater* m_MouseEventEater;

  // Squash all wheel events.
  QmitkWheelEventEater* m_WheelEventEater;

  // Simply keeps track of whether we are currently processing an update to avoid repeated/recursive calls.
  bool m_InDataStorageChanged;

};


#endif
