/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

/*!
 * \file niftkvtk4PointsReader.h
 * \page niftkvtk4PointsReader
 * \section niftkvtk4PointsReaderSummary Reads points to a polydata object
 *
 * Reads a text file where
 * each line represents a point. The first 3 columns being the 
 * coordinates and the 4th column being a weighting. At present the
 * weighting is discarded, though may be used to clip points. Now has
 * option to not read weights, so should work for a file with 3 columns
 * defining the point data.
 */

#ifndef niftkVTK4PointsReader_h
#define niftkVTK4PointsReader_h

#include <NifTKConfigure.h>
#include "niftkVTKWin32ExportHeader.h"

#include <vtkPolyDataAlgorithm.h>

class NIFTKVTK_WINEXPORT niftkvtk4PointsReader : public vtkPolyDataAlgorithm
{
public:
  static niftkvtk4PointsReader* New();
  vtkTypeMacro(niftkvtk4PointsReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the name of the file from which to read points.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  /*
   * \brief turn on range clipping (discards points that fall outside the set range)
   * \params 
   * Direction to clip in (0 = x, 1 = y , 2 = z, 3 = weight)
   * The minimum clipping value
   * The maximum clipping value 
   */
  void SetClippingOn ( int direction, double min, double max );
  /*
   * \brief turn off range clipping (discards points that fall outside the set range)
   * \params 
   * Direction to clip in (0 = x, 1 = y , 2 = z, 3 = weight)
   */
  void SetClippingOff ( int direction );

  /* \brief
   * Set/Get whether to read the weights (4th) column. Set to false if 
   * the file only has three columns
   */
  vtkSetMacro(m_ReadWeights,bool);
  vtkGetMacro(m_ReadWeights,bool);

protected:
  niftkvtk4PointsReader();
  ~niftkvtk4PointsReader();

  char* FileName;

  int RequestData(vtkInformation*,
                  vtkInformationVector**,
                  vtkInformationVector*);
  bool          m_Clipping[4]; 
  double        m_Min[4];
  double        m_Max[4];
  bool          m_ReadWeights;
};

#endif
