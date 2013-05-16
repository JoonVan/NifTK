/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef vtkFunctions_h
#define vtkFunctions_h

#include <NifTKConfigure.h>
#include <niftkVTKWin32ExportHeader.h>
#include <vtkPolyData.h>
#include <vtkTransform.h>
#include <vtkRandomSequence.h>
#include <vtkCellLocator.h>

/** Returns the Euclidean distance between two 3D points, so a and b must be arrays of length 3. */
extern "C++" NIFTKVTK_WINEXPORT double GetEuclideanDistanceBetweenTwo3DPoints(const double *a, const double *b);

/** Returns the length of a 3D vector, so a must be an array of length 3. */
extern "C++" NIFTKVTK_WINEXPORT double GetLength(const double *a);

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
 * \param transform the transform to hold the result
 * \param xtrans,ytrans,ztrans,xrot,yrot,zrot the multipliers in each of the 6 degrees of freedom
 * \param rng the random number generator
 * \return void
 * */
extern "C++" NIFTKVTK_WINEXPORT void RandomTransform
  (vtkTransform  * transform,
  double xtrans, double ytrans, double ztrans, double xrot, double yrot, double zrot,
  vtkRandomSequence * rng);

/** 
 * \brief Creates a randomly determined vtktransform, using it's own random number generator
 * \param transform the transform to hold the result
 * \param xtrans,ytrans,ztrans,xrot,yrot,zrot the multipliers in each of the 6 degrees of freedom
 * \return void
 * */
extern "C++" NIFTKVTK_WINEXPORT void RandomTransform
  (vtkTransform  * transform,
  double xtrans, double ytrans, double ztrans, double xrot, double yrot, double zrot);

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
 * \param source the two polydata, they need the same number of points
 * \return true if Ok, false if error
 */
extern "C++" NIFTKVTK_WINEXPORT bool DistancesToColorMap ( vtkPolyData * source, vtkPolyData * target );

/**
 * \brief Returns the euclidean distance (in 3D) between a point and the closest point
 * on a polydata mesh
 * \param, the point, and the polydata
 * \return the euclidean distance
 */
extern "C++" NIFTKVTK_WINEXPORT double DistanceToSurface ( double  point[3] , vtkPolyData * target);

/**
 * \brief Returns the euclidean distance (in 3D) between a point and the closest point
 * on a polydata mesh
 * \param, the point, a vtkCellLocator, built from the polydata, and optionally a vtkGenericCell 
 * speed up the process.
 * \return the euclidean distance
 */
extern "C++" NIFTKVTK_WINEXPORT double DistanceToSurface ( double point [3] , vtkCellLocator * targetLocator  , vtkGenericCell * cell = NULL );

/**
 * \brief Calculates the euclidean distance (in 3D) between each point in the 
 * source polydata and the closest point on the target polydata mesh.
 * The result are stored the distances in the scalar values of the source
 * \param, the source and target polydata.
 */
extern "C++" NIFTKVTK_WINEXPORT void DistanceToSurface (vtkPolyData * source, vtkPolyData * target);

#endif // vtkFunctions_h
