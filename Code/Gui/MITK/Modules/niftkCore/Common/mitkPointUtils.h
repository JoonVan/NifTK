/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkPointUtils_h
#define mitkPointUtils_h

#include "niftkCoreExports.h"
#include <mitkVector.h>
#include <mitkPointSet.h>
#include <vtkMatrix4x4.h>
#include <mitkCoordinateAxesData.h>

/**
 * \file mitkPointUtils.h
 * \brief A list of utility methods for working with MIT points and stuff.
 */
namespace mitk {

/**
 * \brief Given a double[3] of x,y,z voxel spacing, calculates a step size along a ray, as 1/3 of the smallest voxel dimension.
 */
NIFTKCORE_EXPORT double CalculateStepSize(double *spacing);

/**
 * \brief Returns true if a and b are different (up to a given tolerance, currently 0.01), and false otherwise.
 */
NIFTKCORE_EXPORT bool AreDifferent(const mitk::Point3D& a, const mitk::Point3D& b);

/**
 * \brief Returns the squared Euclidean distance between a and b.
 */
NIFTKCORE_EXPORT double GetSquaredDistanceBetweenPoints(const mitk::Point3D& a, const mitk::Point3D& b);

/**
 * \brief Gets the RMS error between fixed point set and a moving point set, with optional transform specified.
 *
 * Iterates through the moving point set, and if the corresponding point exists in the fixed point set, 
 * will compute the squared distance error, and accumulate this into the RMS error.
 */
NIFTKCORE_EXPORT double GetRMSErrorBetweenPoints(
  const mitk::PointSet& fixed, 
  const mitk::PointSet& moving, 
  const mitk::CoordinateAxesData * const transform = NULL);

/**
 * \brief Returns as output the vector difference of a-b.
 */
NIFTKCORE_EXPORT void GetDifference(const mitk::Point3D& a, const mitk::Point3D& b, mitk::Point3D& output);

/**
 * \brief Given a vector, will calculate the length.
 */
NIFTKCORE_EXPORT double Length(mitk::Point3D& vector);

/**
 * \brief Given a vector, will normalise it to unit length.
 */
NIFTKCORE_EXPORT void Normalise(mitk::Point3D& vector);

/**
 * \brief Copies a into b.
 */
NIFTKCORE_EXPORT void CopyValues(const mitk::Point3D& a, mitk::Point3D& b);

/**
 * \brief Computes c = a x b, and will normalise a and b to unit length first.
 */
NIFTKCORE_EXPORT void CrossProduct(const mitk::Point3D& a, const mitk::Point3D& b, mitk::Point3D& c);

/**
 * \brief Computes the normal by calculating cross product of (a-b) and (c-b).
 */
NIFTKCORE_EXPORT void ComputeNormalFromPoints(const mitk::Point3D& a, const mitk::Point3D& b, const mitk::Point3D& c, mitk::Point3D& output);

/**
 * \brief Copies input to output, i.e. the output is erased, and re-populated.
 */
NIFTKCORE_EXPORT int CopyPointSets(const mitk::PointSet& input, mitk::PointSet& output);

/**
 * \brief Takes fixed and moving points, and scans for matching ID's and returns 2 point sets with
 * ordered and corresponding points.
 * \return the number of points in the output
 */
NIFTKCORE_EXPORT int FilterMatchingPoints(
    const mitk::PointSet& fixedPointsIn,
    const mitk::PointSet& movingPointsIn,
    mitk::PointSet& fixedPointsOut,
    mitk::PointSet& movingPointsOut
    );

/**
 * \brief Simple method to multiply a mitk::Point3D by a vtkMatrix, if the matrix is not NULL,
 * and otherwise if matrix is NULL, will simply leave the point un-altered.
 * \param isNormal if true, will transform the mitk::Point3D as if it was a surface normal.
 */
NIFTKCORE_EXPORT void TransformPointByVtkMatrix(
    vtkMatrix4x4* matrix,
    const bool& isNormal,
    mitk::Point3D& pointOrNormal
    );

} // end namespace mitk




#endif
