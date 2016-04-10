/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef vtkLookupTableUtils_h
#define vtkLookupTableUtils_h

#include "niftkCoreExports.h"
#include <vector>
#include <QColor>

/**
 * \file vtkLookupTableUtils.h
 * \brief General functions for altering vtkLookupTables
 */
 
  class vtkLookupTable;
  
namespace mitk
{
  /**
   * Basic label typedefs
   */
  typedef std::pair<int, std::string> LabelType;
  typedef std::vector<LabelType> LabelsListType;  
  typedef std::vector<QColor> ColorListType;
  
  /**
   * \brief Replace the color in the LookupTable at the given index
   */
  NIFTKCORE_EXPORT vtkLookupTable* ChangeColor(vtkLookupTable* lut, int value, const QColor& newColor);

  /**
   * \brief Swap colors in the LookupTable at the given indices. 
   */
  NIFTKCORE_EXPORT vtkLookupTable* SwapColors(vtkLookupTable* lut, int value1, int value2);

  /**
   * \brief Resize the LookupTable while retaining all of the previously set values.
   * Newly added values are assigned the nan color.
   */
  NIFTKCORE_EXPORT vtkLookupTable* ResizeLookupTable(vtkLookupTable* lut, double newMaximum);

  NIFTKCORE_EXPORT vtkLookupTable* CreateEmptyLookupTable(const QColor& lowColor = QColor(0, 0, 0, 0), const QColor& highColor = QColor(0, 0, 0, 0));
  
}

#endif // vtkLookupTableUtils_h
