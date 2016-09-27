/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkFileIOUtils_h
#define niftkFileIOUtils_h

#include "niftkCoreExports.h"

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkVector.h>


namespace niftk
{

/// \file mitkFileIOUtils.h
/// \brief Various file IO stuff, like loading transformations from file.
bool LoadDoublesFromFile(const std::string& fileName, std::vector<double>& output);

/// \brief Loads a 2D point from file, returning true if successful and false otherwise.
NIFTKCORE_EXPORT bool Load2DPointFromFile(const std::string& fileName, mitk::Point2D& point);

/// \brief Loads a 3D point from file, returning true if successful and false otherwise.
NIFTKCORE_EXPORT bool Load3DPointFromFile(const std::string& fileName, mitk::Point3D& point);

/// \brief Load a plain text file of 4 rows of 4 space separated numbers into a vtkMatrix4x4.
/// \param fileName full path of file name
/// \return vtkSmartPointer<vtkMatrix4x4> that the caller is responsible for
NIFTKCORE_EXPORT vtkSmartPointer<vtkMatrix4x4> LoadVtkMatrix4x4FromFile(const std::string &fileName);

/// \brief Save the matrix to a plain text file of 4 rows of 4 space separated numbers.
/// \param fileName full path of file name
/// \param matrix a matrix
/// \return true if successful and false otherwise
NIFTKCORE_EXPORT bool SaveVtkMatrix4x4ToFile (const std::string& fileName, const vtkMatrix4x4& matrix);

/// \brief Loads all point sets from directory.
NIFTKCORE_EXPORT std::vector<mitk::PointSet::Pointer> LoadPointSetsFromDirectory(const std::string fullDirectoryName);

/// \brief Loads a list of timestamps from a file.
NIFTKCORE_EXPORT void LoadTimeStampData(const std::string& fileName, std::set<unsigned long long>& outputTimeStamps);

}

#endif
