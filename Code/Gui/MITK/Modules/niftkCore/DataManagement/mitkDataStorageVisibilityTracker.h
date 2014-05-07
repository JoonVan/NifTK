/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkDataStorageVisibilityTracker_h
#define mitkDataStorageVisibilityTracker_h

#include "niftkCoreExports.h"
#include "mitkDataStoragePropertyListener.h"
#include <itkObject.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>

namespace mitk
{

class BaseRenderer;

/**
 * \class DataStorageVisibilityTracker
 * \brief Class to listen to changes in visibility properties, and to update a list of BaseRenders.
 *
 * This finds use in the Thumbnail window plugin, which tracks visibility properties, and applies
 * them to a single render window, and also the MIDAS Segmentation Viewer widget which tracks
 * visibility properties, and applies them to an orthoviewer.
 */
class NIFTKCORE_EXPORT DataStorageVisibilityTracker : public itk::Object
{

public:

  mitkClassMacro(DataStorageVisibilityTracker, itk::Object);
  itkNewMacro(DataStorageVisibilityTracker);
  mitkNewMacro1Param(DataStorageVisibilityTracker, const mitk::DataStorage::Pointer);

  /// \brief The main Update method.
  void OnPropertyChanged(mitk::DataNode* node, mitk::BaseRenderer* renderer);

  /// \brief Sets the list of renderers to propagate visibility properties onto.
  void SetRenderersToUpdate(const std::vector<mitk::BaseRenderer*>& renderersToUpdate);

  /// \brief Sets the renderers we are tracking.
  void SetRenderersToTrack(const std::vector<mitk::BaseRenderer*>& renderersToTrack);

  /// \brief Set the data storage, passing it onto the contained DataStoragePropertyListener.
  ///
  /// \see DataStorageListener::SetDataStorage
  void SetDataStorage(const mitk::DataStorage::Pointer dataStorage);

  /// \brief We provide facility to ignore nodes, and not adjust their visibility, which is useful for cross hairs.
  void SetNodesToIgnore(const std::vector<mitk::DataNode*>& nodesToIgnore);

  /// \brief Sends a signal with current the property value of all nodes  to the registered listeners.
  void NotifyAll();

protected:

  DataStorageVisibilityTracker();
  DataStorageVisibilityTracker(const mitk::DataStorage::Pointer);
  virtual ~DataStorageVisibilityTracker();

  DataStorageVisibilityTracker(const DataStorageVisibilityTracker&); // Purposefully not implemented.
  DataStorageVisibilityTracker& operator=(const DataStorageVisibilityTracker&); // Purposefully not implemented.

  bool IsIgnored(mitk::DataNode* node);

private:

  void Init(const mitk::DataStorage::Pointer dataStorage);
  mitk::DataStoragePropertyListener::Pointer m_Listener;
  std::vector<mitk::BaseRenderer*> m_RenderersToTrack;
  std::vector<mitk::BaseRenderer*> m_RenderersToUpdate;
  std::vector<mitk::DataNode*> m_NodesToIgnore;
  mitk::DataStorage::Pointer m_DataStorage;
};

} // end namespace

#endif
