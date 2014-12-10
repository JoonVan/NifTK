/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkMIDASPolyToolOpUpdateFeedbackContour_h
#define mitkMIDASPolyToolOpUpdateFeedbackContour_h

#include "niftkMIDASExports.h"
#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkTool.h>
#include <mitkToolManager.h>
#include <mitkContourModel.h>
#include <mitkPlaneGeometry.h>

namespace mitk
{

/**
 * \class MIDASPolyToolOpUpdateFeedbackContour
 * \brief Operation class to hold data to pass back to this MIDASPolyTool,
 * so that the MIDASPolyTool can execute the Undo/Redo command.
 */
class NIFTKMIDAS_EXPORT MIDASPolyToolOpUpdateFeedbackContour: public mitk::Operation
{
public:

  MIDASPolyToolOpUpdateFeedbackContour(
      mitk::OperationType type,
      unsigned int pointId,
      const mitk::Point3D &point,
      mitk::ContourModel* contour,
      const mitk::PlaneGeometry* geometry
      );

  ~MIDASPolyToolOpUpdateFeedbackContour();

  unsigned int GetPointId() const;

  const mitk::Point3D GetPoint() const;

  mitk::ContourModel* GetContour() const;

  const mitk::PlaneGeometry* GetPlaneGeometry();

private:

  unsigned int m_PointId;
  const mitk::Point3D m_Point;
  mitk::ContourModel::Pointer m_Contour;
  const mitk::PlaneGeometry* m_PlaneGeometry;

};

}

#endif
