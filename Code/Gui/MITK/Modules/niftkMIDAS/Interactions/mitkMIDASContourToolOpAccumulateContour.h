/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkMIDASContourToolOpAccumulateContour_h
#define mitkMIDASContourToolOpAccumulateContour_h

#include "niftkMIDASExports.h"
#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkTool.h>
#include <mitkToolManager.h>
#include <mitkContourModelSet.h>

namespace mitk
{

/**
 * \class MIDASContourToolOpAccumulateContour
 * \brief Operation class to hold data to pass back to this MIDASContourTool,
 * so that this MIDASContourTool can execute the Undo/Redo command.
 */
class NIFTKMIDAS_EXPORT MIDASContourToolOpAccumulateContour: public mitk::Operation
{
public:

  MIDASContourToolOpAccumulateContour(
      mitk::OperationType type,
      bool redo,
      int dataSetNumber,
      mitk::ContourModelSet::Pointer contourSet
      );
  ~MIDASContourToolOpAccumulateContour() {}
  bool IsRedo() const { return m_Redo; }
  int GetDataSetNumber() const { return m_DataSetNumber; }
  mitk::ContourModelSet::Pointer GetContourSet() const { return m_ContourSet;}

private:
  bool m_Redo;
  int  m_DataSetNumber;
  mitk::ContourModelSet::Pointer m_ContourSet;
};

} // end namespace

#endif
