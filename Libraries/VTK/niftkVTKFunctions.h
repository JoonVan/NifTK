/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkVTKFunctions_h
#define niftkVTKFunctions_h

#include <niftkVTKWin32ExportHeader.h>

#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkRandomSequence.h>
#include <vtkCellLocator.h>
#include <vtkCamera.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>

/**
 * \file vtkFunctions.h
 * \brief Various VTK functions that need sorting into a more sensible arrangement.
 */

namespace niftk {

/** Returns the Euclidean distance between two 3D points, so a and b must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT double GetEuclideanDistanceBetweenTwo3DPoints(const double *a, const double *b);

/** Returns the length of a 3D vector, so a must be an array of length 3. */
extern "C++" NIFTKVTK_WINEXPORT double GetLength(const double *a);

/** Scales the unit vector a by scaleFactor, and writes to b, so a and be must be an array of length 3. */
extern "C++" NIFTKVTK_WINEXPORT void ScaleVector(const double& scaleFactor, const double* a, double* b);

/** Subtracts two 3D points, so a, b and output must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT void SubtractTwo3DPoints(const double *a, const double *b, double *output);

/** Adds two 3D points, so a, b and output must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT void AddTwo3DPoints(const double *a, const double *b, double *output);

/** Normalises a to unit length. */
extern "C++" NIFTKVTK_WINEXPORT void NormaliseToUnitLength(const double *a, double *output);

/** Divides a 3D point by a length, so a and output must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT void Normalise3DPoint(const double *a, const double length, double *output);

/** Takes the cross product of 2 vectors, so a, b and output must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT void CrossProductTwo3DVectors(const double *a, const double *b, double *output);

/** Calculates a unit vector from (a-b), so a, b and output must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT void CalculateUnitVector(const double *a, const double* b, double *output);

/** Calculates the angle in radians between two vectors a and b, which must be of length 3. */
extern "C++" NIFTKVTK_WINEXPORT double AngleBetweenTwoUnitVectors(const double *a, const double *b);

/** Calculates the angle in degrees between two vectors a and b, which must be of length 3. */
extern "C++" NIFTKVTK_WINEXPORT double AngleBetweenTwoUnitVectorsInDegrees(const double *a, const double *b);

/** Makes sure that the supplied point is within the VTK bounding box, by independently clipping the x, y, z coordinate to be within range of the bounds. Returns true if point was clipped and false otherwise. */
extern "C++" NIFTKVTK_WINEXPORT bool ClipPointToWithinBoundingBox(const double *boundingBoxVector6, double *point);

/** Calculates the bounding box diagonal length. */
extern "C++" NIFTKVTK_WINEXPORT double GetBoundingBoxDiagonalLength(const double *boundingBoxVector6);

/** Copies n doubles from a to b, which must be allocated, and at least of length n. */
extern "C++" NIFTKVTK_WINEXPORT void CopyDoubleVector(int n, const double *a, double *b);

/** Perturbs a vtkPolyData, by moving each point along its normal. */
extern "C++" NIFTKVTK_WINEXPORT void PerturbPolyDataAlongNormal(vtkPolyData * polydata,
        double stdDev, vtkRandomSequence * rng);

/**
 * \brief Perturbs the points in a polydata object by random values, using existing random number generator
 * \param polydata the polydata
 * \param xerr,yerr,zerr the multipliers for the random number generator in each direction.
 * \param rng the random number generator
 * \return void
 */
extern "C++" NIFTKVTK_WINEXPORT void PerturbPolyData(vtkPolyData * polydata,
        double xerr, double yerr, double zerr, vtkRandomSequence * rng);

/**
 * \brief Perturbs the points in a polydata object by with random values, intialising and using it's own random number generator
 * \param polydata the polydata
 * \param xerr,yerr,zerr the multipliers for the random number generator in each direction.
 * \return void
 * */
extern "C++" NIFTKVTK_WINEXPORT void PerturbPolyData(vtkPolyData * polydata,
        double xerr, double yerr, double zerr);

/**
 * \brief Translates a polydata object using a transform.
 * \param polydata the polydata
 * \param transform the transform
 * \return void
 * */
extern "C++" NIFTKVTK_WINEXPORT void TranslatePolyData
  (vtkPolyData  * polydata, vtkTransform * transform);

/**
 * \brief Creates a randomly determined vtkTransform, using existing random number geneterator
 * \param xtrans,ytrans,ztrans,xrot,yrot,zrot the multipliers in each of the 6 degrees of freedom
 * \param rng the random number generator
 * \param toCentre a transform defining the desired centre of rotation
 * \param scaleSD if greater than zero the resulting transform is scaled to have this magnitude
 * \return the transform.
 * */
extern "C++" NIFTKVTK_WINEXPORT vtkSmartPointer<vtkTransform> RandomTransformAboutRemoteCentre
  ( const double& xtrans, const double& ytrans, const double& ztrans,
  const double& xrot, const double& yrot, const double& zrot,
  vtkRandomSequence& rng,
  vtkSmartPointer<vtkTransform> toCentre,
  const double& scaleSD);

/**
 * \brief Creates a randomly determined vtkTransform using existing random number geneterator
 * \param xtrans,ytrans,ztrans,xrot,yrot,zrot the multipliers in each of the 6 degrees of freedom
 * \param rng the random number generator
 * \param scaleSD if greater than zero the resulting transform is scaled to have this magnitude
 * \return the transform.
 * */
extern "C++" NIFTKVTK_WINEXPORT vtkSmartPointer<vtkTransform> RandomTransform
  ( const double& xtrans, const double& ytrans, const double& ztrans,
  const double& xrot, const double& yrot, const double& zrot,
  vtkRandomSequence& rng,
  const double& scaleSD);

/**
 * \brief Takes a vector of 6 parameters defining a rigid transform , and returns
 * \brief a 4x4 matrix. Throws runtime_error if incorrect vector length
 * \param A vector of 6 values , 3 translation (x,y,z) followed by 3 rotations (x,y,z)
 * \return A vtkTransform
 */

extern "C++" NIFTKVTK_WINEXPORT vtkSmartPointer<vtkTransform> RigidTransformFromVector
  ( const std::vector<double> transform );

/**
 * \brief Normalises the values returned by a vtk random sequence to be centred on zero
 * \param rng the random number sequence
 * \return The normalised value
 * */
extern "C++" NIFTKVTK_WINEXPORT double NormalisedRNG (vtkRandomSequence * rng);

/**
 * \brief Measures the euclidean distances between the points in two polydata, and sets the
 * \brief scalars in both polydata to a color map to show the differences, min distance red,
 * \brief max distance is blue. Mid distance is green
 * \param source,target the two polydata, they need the same number of points
 * \return true if Ok, false if error
 */
extern "C++" NIFTKVTK_WINEXPORT bool DistancesToColorMap ( vtkPolyData * source, vtkPolyData * target );

/**
 * \brief Returns the euclidean distance (in 3D) between a point and the closest point
 * on a polydata mesh
 * \param point the point
 * \param target and the polydata
 * \return the euclidean distance
 */
extern "C++" NIFTKVTK_WINEXPORT double DistanceToSurface ( double  point[3] , vtkPolyData * target);

/**
 * \brief Returns the euclidean distance (in 3D) between a point and the closest point
 * on a polydata mesh
 * \param point the point
 * \param targetLocator a vtkCellLocator, built from the polydata
 * \param cell  and optionally a vtkGenericCell
 * \return the euclidean distance
 */
extern "C++" NIFTKVTK_WINEXPORT double DistanceToSurface ( double point [3] , vtkCellLocator * targetLocator  , vtkGenericCell * cell = NULL );

/**
 * \brief Calculates the euclidean distance (in 3D) between each point in the
 * source polydata and the closest point on the target polydata mesh.
 * The result are stored the distances in the scalar values of the source
 * \param source,target the source and target polydata.
 */
extern "C++" NIFTKVTK_WINEXPORT void DistanceToSurface (vtkPolyData * source, vtkPolyData * target);

/**
 * \brief Calculates the euclidean distance (in 3D) between each point in the
 * source polydata and the closest point on the target polydata mesh.
 * The result distances are stored in the scalar values passed in.
 * \param source,target the source and target polydata.
 */
extern "C++" NIFTKVTK_WINEXPORT void DistanceToSurface(vtkPolyData* source, vtkPolyData* target, vtkSmartPointer<vtkDoubleArray>& result);

/**
 * \brief Writes matrix out as a string, for use in SaveMatrix4x4ToFile.
 */
extern "C++" NIFTKVTK_WINEXPORT std::string WriteMatrix4x4ToString(const vtkMatrix4x4& matrix);

/**
 * \brief Save the matrix to a plain text file of 4 rows of 4 space separated numbers.
 * \param fileName full path of file name
 * \param matrix a matrix
 * \param bool true if successful and false otherwise
 */
extern "C++" NIFTKVTK_WINEXPORT bool SaveMatrix4x4ToFile (const std::string& fileName, const vtkMatrix4x4& matrix, const bool& silent=false);

/**
 * \brief Loads the matrix from file, or else creates an Identity matrix, and the caller is responsible for deallocation.
 * \param fileName
 */
extern "C++" NIFTKVTK_WINEXPORT vtkSmartPointer<vtkMatrix4x4> LoadMatrix4x4FromFile(const std::string& fileName, const bool& silent=false);

/**
 * \brief Checks matrices for equality.
 * \param tolerance absolute difference between corresponding elements must be less than this number.
 */
extern "C++" NIFTKVTK_WINEXPORT bool MatricesAreEqual(const vtkMatrix4x4& m1, const vtkMatrix4x4& m2, const double& tolerance=0.01);

/**
 * \brief Used to set a vtkCamera to track a 2D image, and sets the camera to parallel projection mode.
 * \param imageSize array of 2 integers containing imageSize[0]=number of pixels in x, imageSize[1]=number of pixels in y of the image
 * \param windowSize array of 2 integers containing width and height of the current window.
 * \param origin array of 3 doubles containing the x,y,z coordinates in 3D space of the origin of the image, presumed to be the centre of the first (0,0) voxel.
 * \param spacing array of 2 doubles containing the x and y spacing in mm.
 * \param xAxis array of 3 doubles containing the x,y,z direction vector describing the x-axis.
 * \param yAxis array of 3 doubles containing the x,y,z direction vector describing the y-axis.
 * \param clippingRange array of 2 doubles containing the near and far clipping range.
 * \param flipYAxis if true we flip the y-axis.
 */
extern "C++" NIFTKVTK_WINEXPORT void SetCameraParallelTo2DImage(
    const int *imageSize,
    const int *windowSize,
    const double *origin,
    const double *spacing,
    const double *xAxis,
    const double *yAxis,
    const double *clippingRange,
    const bool& flipYAxis,
    vtkCamera& camera,
    const double& distanceToFocalPoint = -1000
    );

/**
 * \brief Randomly removes points from the passed polydata until the passed number of points
 * Any cells or surfaces will be deleted as part of the process, leaving only points.
 */
extern "C++" NIFTKVTK_WINEXPORT bool CropPointsFromPolyData(vtkPolyData* PolyData, int Points = 200);

/**
 * \brief Extracts the rotation matrix, and converts to quaternion.
 * \param quaternion must be a pointer to an array of 4 doubles that is already allocated.
 */
extern "C++" NIFTKVTK_WINEXPORT void MatrixToQuaternion(const vtkMatrix4x4& matrix, double* quaternion);

/**
 * \brief Performs spherical linear interpolation.
 * \param beforeRotation quaternion as 4-vector, already allocated.
 * \param afterRotation quaternion as 4-vector, already allocated.
 * \param outputRotation quaternion as 4-vector, already allocated.
 * \param weight between 0 and 1.
 */
extern "C++" NIFTKVTK_WINEXPORT void InterpolateRotation(const double* beforeRotation, const double* afterRotation, const double& weight, double* outputRotation, bool adjustSign /*= true*/);

/**
 * \brief Interpolates between two matrices.
 * \param proportion is defined as between [0 and 1], where 0 gives exactly the before matrix, 1 gives exactly the after matrix, and the proportion is a linear proportion between them to interpolate.
 */
extern "C++" NIFTKVTK_WINEXPORT void InterpolateTransformationMatrix(const vtkMatrix4x4& before, const vtkMatrix4x4& after, const double& proportion, vtkMatrix4x4& interpolated);

} // end namespace

#endif
