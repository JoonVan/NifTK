/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkTrackedImageCommand_h
#define mitkTrackedImageCommand_h

#include "niftkIGIExports.h"
#include <mitkDataStorage.h>
#include <vtkMatrix4x4.h>
#include <mitkDataNode.h>
#include <itkObject.h>
#include <itkObjectFactoryBase.h>

namespace mitk {

/**
 * \class TrackedImageCommand
 * \brief Command used to update the position of a tracked image.
 */
class NIFTKIGI_EXPORT TrackedImageCommand : public itk::Object
{
public:

  mitkClassMacro(TrackedImageCommand, itk::Object);
  itkNewMacro(TrackedImageCommand);

  /**
   * \brief Stores the name of a DataNode property, indicating the currently selected image = niftk.trackedimage
   */
  static const char* TRACKED_IMAGE_SELECTED_PROPERTY_NAME;
  
  /**
   * \brief Computes the new position/scaling of the tracked image plane.
   */
  void Update(const mitk::DataNode::Pointer imageNode,
           const mitk::DataNode::Pointer trackingSensorToTrackerNode,
           const vtkMatrix4x4* imageToTrackingSensor,
           const mitk::Point2D& imageScaling
           );

protected:

  TrackedImageCommand(); // Purposefully hidden.
  virtual ~TrackedImageCommand(); // Purposefully hidden.

  TrackedImageCommand(const TrackedImageCommand&); // Purposefully not implemented.
  TrackedImageCommand& operator=(const TrackedImageCommand&); // Purposefully not implemented.

private:

}; // end class

} // end namespace

#endif
