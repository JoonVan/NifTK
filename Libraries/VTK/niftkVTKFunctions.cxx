/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include <math.h>
#include <iostream>
#include <niftkConversionUtils.h>
#include <niftkMathsUtils.h>
#include "niftkVTKFunctions.h"
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkBoxMuellerRandomSequence.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkDoubleArray.h>
#include <vtkLookupTable.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPointData.h>
#include <vtkGenericCell.h>
#include <vtkVersion.h>
#include <vtkMath.h>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

namespace niftk {

//-----------------------------------------------------------------------------
double GetEuclideanDistanceBetweenTwo3DPoints(const double *a, const double *b)
{
  double distance = 0;
  for (int i = 0; i < 3; i++)
  {
    distance += ((a[i]-b[i])*(a[i]-b[i]));
  }
  distance = sqrt(distance);
  return distance;
}


//-----------------------------------------------------------------------------
double GetLength(const double *a)
{
  double length = 0;
  for (int i = 0; i < 3; i++)
  {
    length += (a[i]*a[i]);
  }
  length = sqrt(length);
  return length;
}


//-----------------------------------------------------------------------------
void ScaleVector(const double& scaleFactor, const double* a, double* b)
{
  for (int i = 0; i < 3; ++i)
  {
    b[i] = a[i] * scaleFactor;
  }
}


//-----------------------------------------------------------------------------
void SubtractTwo3DPoints(const double *a, const double *b, double *output)
{
  for (int i = 0; i < 3; i++)
  {
    output[i] = a[i] - b[i];
  }
}


//-----------------------------------------------------------------------------
void AddTwo3DPoints(const double *a, const double *b, double *output)
{
  for (int i = 0; i < 3; i++)
  {
    output[i] = a[i] + b[i];
  }
}


//-----------------------------------------------------------------------------
void Normalise3DPoint(const double *a, const double length, double *output)
{
  for (int i = 0; i < 3; i++)
  {
    if (length > 0)
    {
      output[i] = a[i]/length;
    }
    else
    {
      output[i] = a[i];
    }
  }
}


//-----------------------------------------------------------------------------
void NormaliseToUnitLength(const double *a, double *output)
{
  double length = GetLength(a);
  Normalise3DPoint(a, length, output);
}


//-----------------------------------------------------------------------------
void CrossProductTwo3DVectors(const double *a, const double *b, double *c)
{
  c[0] =        a[1]*b[2] - b[1]*a[2];
  c[1] = -1.0* (a[0]*b[2] - b[0]*a[2]);
  c[2] =        a[0]*b[1] - b[0]*a[1];
}


//-----------------------------------------------------------------------------
void CalculateUnitVector(const double *a, const double* b, double *output)
{
  double normal[3];
  SubtractTwo3DPoints(a, b, normal);

  double length = GetLength(normal);
  Normalise3DPoint(normal, length, output);
}


//-----------------------------------------------------------------------------
double AngleBetweenTwoUnitVectors(const double *a, const double *b)
{
  double cosTheta = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
  double result = acos(cosTheta);
  return result;
}


//-----------------------------------------------------------------------------
double AngleBetweenTwoUnitVectorsInDegrees(const double *a, const double *b)
{
  double result = (AngleBetweenTwoUnitVectors(a, b))*180.0/NIFTK_PI;
  return result;
}


//-----------------------------------------------------------------------------
bool ClipPointToWithinBoundingBox(const double *bounds, double *point)
{
  bool wasClipped = false;

  for (int i = 0; i < 3; i++)
  {
    if (point[i] < bounds[i*2])
    {
      point[i] = bounds[i*2];
      wasClipped = true;
    }
    else if (point[i] > bounds[i*2 + 1])
    {
      point[i] = bounds[i*2 + 1];
      wasClipped = true;
    }
  }

  return wasClipped;
}


//-----------------------------------------------------------------------------
double GetBoundingBoxDiagonalLength(const double *boundingBoxVector6)
{
  double length = 0;
  length += ((boundingBoxVector6[1] - boundingBoxVector6[0]) * (boundingBoxVector6[1] - boundingBoxVector6[0]));
  length += ((boundingBoxVector6[3] - boundingBoxVector6[2]) * (boundingBoxVector6[3] - boundingBoxVector6[2]));
  length += ((boundingBoxVector6[5] - boundingBoxVector6[4]) * (boundingBoxVector6[5] - boundingBoxVector6[4]));
  length = sqrt(length);
  return length;
}


//-----------------------------------------------------------------------------
void CopyDoubleVector(int n, const double *a, double *b)
{
  for (int i = 0; i < n; i++)
  {
    b[i] = a[i];
  }
}


//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTransform> RandomTransformAboutRemoteCentre (
    const double& xtrans, const double& ytrans, const double& ztrans,
    const double& xrot, const double& yrot, const double& zrot,
    vtkRandomSequence& rng,
    vtkSmartPointer<vtkTransform> toCentre,
    const double& scaleSD)
{
  vtkSmartPointer < vtkTransform > transform = vtkSmartPointer<vtkTransform>::New();
  transform->Identity();

  vtkSmartPointer < vtkTransform > randomTransform = niftk::RandomTransform (
      xtrans, ytrans, ztrans, xrot, yrot, zrot, rng, scaleSD );
  vtkSmartPointer < vtkTransform > toOrigin = vtkSmartPointer<vtkTransform>::New();
  toOrigin->DeepCopy (toCentre);
  toOrigin->Inverse();

  transform->PostMultiply();
  transform->Concatenate(toOrigin);
  transform->Concatenate(randomTransform);
  transform->Concatenate(toCentre);

  return transform;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTransform> RandomTransform (
    const double& xtransSD, const double& ytransSD, const double& ztransSD,
    const double& xrotSD, const double& yrotSD, const double& zrotSD,
    vtkRandomSequence& rng,
    const double& scaleSD)
{
  vtkSmartPointer < vtkTransform > transform = vtkSmartPointer<vtkTransform>::New();

  transform->Identity();

  //create covariance vector
  std::vector < double > stddev;
  std::vector < double > covariance;
  std::vector < double > zeros;
  std::vector < double > randomTransform;
  stddev.push_back ( xtransSD );
  stddev.push_back ( ytransSD );
  stddev.push_back ( ztransSD );
  stddev.push_back ( xrotSD );
  stddev.push_back ( yrotSD );
  stddev.push_back ( zrotSD );

  for ( std::vector<double>::iterator it = stddev.begin() ; it < stddev.end() ; ++ it )
  {
    randomTransform.push_back ( (*it) * NormalisedRNG ( &rng ));
    rng.Next();
    zeros.push_back(0.0);
    covariance.push_back ( (*it) * (*it) );
  }

  double currentDistance = niftk::MahalanobisDistance ( zeros, randomTransform, covariance );
  std::cout << "Initial Normalised euclidean distance = " << currentDistance << std::endl;
  double scaleFactor = scaleSD / currentDistance;

  if ( scaleSD > 0 )
  {
    for ( std::vector<double>::iterator it = randomTransform.begin() ; it < randomTransform.end() ; ++ it )
    {
      *it *= scaleFactor ;
    }
    double correctedDistance = niftk::MahalanobisDistance ( zeros, randomTransform, covariance );
    std::cout << "Corrected Normalised euclidean distance = " << correctedDistance << std::endl;
  }

  transform = niftk::RigidTransformFromVector ( randomTransform );
  return transform;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkTransform> RigidTransformFromVector ( std::vector < double > transform )
{
  if ( transform.size() != 6 )
  {
    throw std::runtime_error ( "Vector to define rigid transform requires 6 values");
  }
  vtkSmartPointer <vtkTransform> transformOut = vtkSmartPointer<vtkTransform>::New();
  transformOut->Translate(transform [0],transform [1],transform [2]);
  transformOut->RotateX(transform [3]);
  transformOut->RotateY(transform [4]);
  transformOut->RotateZ(transform [5]);
  return transformOut;
}

//-----------------------------------------------------------------------------
void TranslatePolyData(vtkPolyData* polydata, vtkTransform * transform)
{
  vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
#if VTK_MAJOR_VERSION <= 5
  transformFilter->SetInputConnection(polydata->GetProducerPort());
#else
  transformFilter->SetInputData(polydata);
#endif
  transformFilter->SetTransform(transform);
  transformFilter->Update();

  polydata->ShallowCopy(transformFilter->GetOutput());

}


//-----------------------------------------------------------------------------
void PerturbPolyDataAlongNormal(vtkPolyData * polydata,
  double stdDev, vtkRandomSequence * rng)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->ShallowCopy(polydata->GetPoints());

  vtkDataArray* normals = polydata->GetPointData()->GetNormals();
  double *n;
  double p[3];

  double offset;
  vtkSmartPointer<vtkMinimalStandardRandomSequence> uniRand = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
  uniRand->SetSeed(2);
  vtkIdType pointId = 0;

  for(vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    // Choosing a random point id.
    uniRand->Next();
    pointId = static_cast<vtkIdType>((uniRand->GetValue() * (points->GetNumberOfPoints()-1)));

    points->GetPoint(pointId, p);
    n = normals->GetTuple3(pointId);

    rng->Next();
    offset = NormalisedRNG(rng) * stdDev ;

    p[0] += n[0]*offset;
    p[1] += n[1]*offset;
    p[2] += n[2]*offset;

    points->SetPoint(pointId, p);
  }
  polydata->SetPoints(points);
}


//-----------------------------------------------------------------------------
void PerturbPolyData(vtkPolyData* polydata,
    double xerr, double yerr, double zerr, vtkRandomSequence* rng)
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->ShallowCopy(polydata->GetPoints());

  for(vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    double p[3];
    points->GetPoint(i, p);
    double perturb[3];
    rng->Next();
    perturb[0] = NormalisedRNG(rng) * xerr ;
    rng->Next();
    perturb[1] = NormalisedRNG(rng) * yerr ;
    rng->Next();
    perturb[2] = NormalisedRNG(rng) * zerr ;
    rng->Next();
    for(unsigned int j = 0; j < 3; j++)
    {
      p[j] += perturb[j];
    }
    points->SetPoint(i, p);
  }
  polydata->SetPoints(points);
}


//-----------------------------------------------------------------------------
void PerturbPolyData(vtkPolyData* polydata,
    double xerr, double yerr, double zerr)
{
   vtkSmartPointer<vtkBoxMuellerRandomSequence> Gauss_Rand = vtkSmartPointer<vtkBoxMuellerRandomSequence>::New();
   vtkSmartPointer<vtkMinimalStandardRandomSequence> Uni_Rand = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
   Uni_Rand->SetSeed(time(NULL));
   Gauss_Rand->SetUniformSequence(Uni_Rand);
   PerturbPolyData(polydata,xerr, yerr,zerr, Gauss_Rand);
}

//-----------------------------------------------------------------------------
double NormalisedRNG (vtkRandomSequence* rng)
{
  if  ( rng->IsA("vtkMinimalStandardRandomSequence") == 1 )
  {
    return rng->GetValue() - 0.5;
  }
  if ( rng->IsA("vtkBoxMuellerRandomSequence") == 1 )
  {
    return rng->GetValue();
  }
  std::cerr << "WARNING: Unknown random number generator encountered, can't normalise." << std::endl;
  return rng->GetValue();
}


//-----------------------------------------------------------------------------
bool DistancesToColorMap ( vtkPolyData * source, vtkPolyData * target )
{
  if ( source->GetNumberOfPoints() != target->GetNumberOfPoints() )
  {
    return false;
  }
  vtkSmartPointer<vtkDoubleArray> differences = vtkSmartPointer<vtkDoubleArray>::New();
  differences->SetNumberOfComponents(1);
  differences->SetName("Differences");
  double min_dist=0;
  double max_dist=0;
  for ( int i = 0 ; i < source->GetNumberOfPoints() ; i ++ )
  {
    double p[3];
    source->GetPoint(i,p);
    double q[3];
    target->GetPoint(i,q);
    double dist = 0;
    for ( int j = 0 ; j < 3 ; j++ )
    {
      dist += (p[j]-q[j])*(p[j]-q[j]);
    }
    dist = sqrt(dist);
    differences->InsertNextValue(dist);
    if ( i == 0 )
    {
      min_dist=dist;
      max_dist=dist;
    }
    else
    {
      min_dist = dist < min_dist ? dist : min_dist;
      max_dist = dist > max_dist ? dist : max_dist;
    }
   }
   vtkSmartPointer<vtkLookupTable> colorLookupTable = vtkSmartPointer<vtkLookupTable>::New();
   std::cerr << "Max Error = " << max_dist << " mm. Min Error = " << min_dist << " mm." << std::endl;
   colorLookupTable->SetTableRange(min_dist, max_dist);
   colorLookupTable->Build();
   vtkSmartPointer<vtkUnsignedCharArray> colors =vtkSmartPointer<vtkUnsignedCharArray>::New();
   colors->SetNumberOfComponents(3);
   colors->SetName("Colors");

   unsigned char color[3];
   double dcolor[3];

   for ( int i = 0 ; i < source->GetNumberOfPoints() ; i ++ )
   {
     colorLookupTable->GetColor(differences->GetValue(i),dcolor);
     for ( int j = 0 ; j < 3 ; j++ )
     {
       color[j] = static_cast<unsigned char>(255.0 * dcolor[j]);
     }
     colors->InsertNextTupleValue(color);
   }

   source->GetPointData()->SetScalars(colors);
   target->GetPointData()->SetScalars(colors);
   return true;
}


//-----------------------------------------------------------------------------
double DistanceToSurface (  double point[3],  vtkPolyData * target )
{
  vtkSmartPointer<vtkCellLocator> targetLocator = vtkSmartPointer<vtkCellLocator>::New();
  targetLocator->SetDataSet(target);
  targetLocator->BuildLocator();

  return DistanceToSurface (point, targetLocator);
}


//-----------------------------------------------------------------------------
double DistanceToSurface (  double point[3],
     vtkCellLocator * targetLocator, vtkGenericCell * cell )
{
  double NearestPoint [3];
  vtkIdType cellID;
  int SubID;
  double DistanceSquared;

  if ( cell != NULL )
  {
    targetLocator->FindClosestPoint(point, NearestPoint, cell,
        cellID, SubID, DistanceSquared);
  }
  else
  {
    targetLocator->FindClosestPoint(point, NearestPoint,
        cellID, SubID, DistanceSquared);
  }

  return sqrt(DistanceSquared);
}


//-----------------------------------------------------------------------------
void DistanceToSurface(vtkPolyData* source, vtkPolyData* target, vtkSmartPointer<vtkDoubleArray>& result)
{
  result = vtkSmartPointer<vtkDoubleArray>::New();
  result->SetNumberOfComponents(1);
  result->SetName("Distances");

  vtkSmartPointer<vtkCellLocator> targetLocator = vtkSmartPointer<vtkCellLocator>::New();
  targetLocator->SetDataSet(target);
  targetLocator->BuildLocator();

  vtkSmartPointer<vtkGenericCell> cell = vtkSmartPointer<vtkGenericCell>::New();
  double p[3];
  for (int i = 0; i < source->GetNumberOfPoints(); i++)
  {
    source->GetPoint(i, p);
    result->InsertNextValue(DistanceToSurface(p, targetLocator, cell));
  }
}


//-----------------------------------------------------------------------------
void DistanceToSurface ( vtkPolyData * source, vtkPolyData * target )
{
  vtkSmartPointer<vtkDoubleArray>   distances;
  DistanceToSurface(source, target, distances);

  source->GetPointData()->SetScalars(distances);
}


//-----------------------------------------------------------------------------
std::string WriteMatrix4x4ToString(const vtkMatrix4x4& matrix)
{
  std::stringstream oss;
  for (int i = 0; i < 4; i++)
  {
    oss << matrix.GetElement(i, 0) << " " \
        << matrix.GetElement(i, 1) << " " \
        << matrix.GetElement(i, 2) << " " \
        << matrix.GetElement(i, 3) << std::endl;
  }
  return oss.str();
}


//-----------------------------------------------------------------------------
bool SaveMatrix4x4ToFile (const std::string& fileName, const vtkMatrix4x4& matrix, const bool& silent)
{
  bool successful = false;

  ofstream myfile(fileName.c_str());
  if (myfile.is_open())
  {
    std::string tmp = niftk::WriteMatrix4x4ToString(matrix);
    myfile << tmp;
    myfile.close();
    successful = true;
  }
  else
  {
    if (!silent)
    {
      std::cerr << "SaveMatrix4x4ToFile: failed to save to file '" << fileName << "'" << std::endl;
    }
  }

  return successful;
}


//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMatrix4x4> LoadMatrix4x4FromFile(const std::string& fileName, const bool& silent)
{
  vtkSmartPointer<vtkMatrix4x4> result = vtkSmartPointer<vtkMatrix4x4>::New();
  result->Identity();

  if (fileName.size() > 0)
  {
    ifstream myfile(fileName.c_str());
    if (myfile.is_open())
    {
      for (int i = 0; i < 4; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          double value;
          myfile >> value;

          result->SetElement(i, j, value);
        }
      }
    }
    else
    {
      if (!silent)
      {
        std::cerr << "LoadMatrix4x4FromFile: failed to open file '" << fileName << "'" << std::endl;
      }
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
bool MatricesAreEqual(const vtkMatrix4x4& m1, const vtkMatrix4x4& m2, const double& tolerance)
{
  bool result = true;

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (fabs(m1.GetElement(i,j) - m2.GetElement(i,j)) > tolerance)
      {
        result = false;
        break;
      }
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
void SetCameraParallelTo2DImage(
    const int *imageSize,
    const int *windowSize,
    const double *origin,
    const double *spacing,
    const double *xAxis,
    const double *yAxis,
    const double *clippingRange,
    const bool& flipYAxis,
    vtkCamera& camera,
    const double& distanceToFocalPoint
    )
{
  double focalPoint[3] = {0, 0, 1};
  double position[3] = {0, 0, 0};
  double viewUp[3] = {0, 1, 0};
  double xAxisUnitVector[3] = {1, 0, 0};
  double yAxisUnitVector[3] = {0, 1, 0};
  double zAxisUnitVector[3] = {0, 0, 1};
  double distanceAlongX = 1;
  double distanceAlongY = 1;
  double vectorAlongX[3] = {1, 0, 0};
  double vectorAlongY[3] = {0, 1, 0};
  double vectorAlongZ[3] = {0, 0, 1};

  double viewUpScaleFactor = 1.0e9;
  if ( flipYAxis )
  {
    viewUpScaleFactor *= -1;
  }

  NormaliseToUnitLength(xAxis, xAxisUnitVector);
  NormaliseToUnitLength(yAxis, yAxisUnitVector);
  CrossProductTwo3DVectors(xAxisUnitVector, yAxisUnitVector, zAxisUnitVector);

  distanceAlongX = ( spacing[0] * (imageSize[0] - 1) ) / 2.0;
  distanceAlongY = ( spacing[1] * (imageSize[1] - 1) ) / 2.0;

  ScaleVector(distanceAlongX,       xAxisUnitVector, vectorAlongX);
  ScaleVector(distanceAlongY,       yAxisUnitVector, vectorAlongY);
  ScaleVector(distanceToFocalPoint, zAxisUnitVector, vectorAlongZ);

  for ( unsigned int i = 0; i < 3; ++i)
  {
    focalPoint[i] = origin[i] + vectorAlongX[i] + vectorAlongY[i];
  }

  AddTwo3DPoints(focalPoint, vectorAlongZ, position);
  ScaleVector(viewUpScaleFactor, vectorAlongY, viewUp);

  double imageWidth = imageSize[0]*spacing[0];
  double imageHeight = imageSize[1]*spacing[1];

  double widthRatio = imageWidth / windowSize[0];
  double heightRatio = imageHeight / windowSize[1];

  double scale = 1;
  if (widthRatio > heightRatio)
  {
    scale = 0.5*imageWidth*((double)windowSize[1]/(double)windowSize[0]);
  }
  else
  {
    scale = 0.5*imageHeight;
  }

  camera.SetPosition(position);
  camera.SetFocalPoint(focalPoint);
  camera.SetViewUp(viewUp);
  camera.SetParallelProjection(true);
  camera.SetParallelScale(scale);
  camera.SetClippingRange(clippingRange);
}


//-----------------------------------------------------------------------------
bool CropPointsFromPolyData(vtkPolyData* PolyData, int Points)
{
  vtkSmartPointer<vtkMinimalStandardRandomSequence> rng = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();
  rng->SetSeed(time(NULL));

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkPoints> pointsout = vtkSmartPointer<vtkPoints>::New();
  points->ShallowCopy(PolyData->GetPoints());

  for ( int i = 0 ; i < Points ; i ++ )
  {
    int PointToGet = static_cast<int>(rng->GetValue() * PolyData->GetNumberOfPoints());
    rng->Next();
    pointsout->InsertNextPoint(points->GetPoint(PointToGet));
  }
  PolyData->SetPoints(pointsout);
  return true;
}


//-----------------------------------------------------------------------------
void MatrixToQuaternion(const vtkMatrix4x4& matrix, double* quaternion)
{
  double rotation[3][3] = {{0,0,0}, {0,0,0}, {0,0,0}};

  for (unsigned int r = 0; r < 3; r++)
  {
    rotation[r][0] = matrix.GetElement(r,0);
    rotation[r][1] = matrix.GetElement(r,1);
    rotation[r][2] = matrix.GetElement(r,2);
  }

  vtkMath::Matrix3x3ToQuaternion(rotation, quaternion);
}


//-----------------------------------------------------------------------------
void InterpolateRotation(const double* beforeRotation, const double* afterRotation, const double& weight, double* outputRotation, bool adjustSign)
{
  double cosTheta = beforeRotation[0]*afterRotation[0]
                   +beforeRotation[1]*afterRotation[1]
                   +beforeRotation[2]*afterRotation[2]
                   +beforeRotation[3]*afterRotation[3];

  double afterRotn[4];

  if (adjustSign && (cosTheta < static_cast<double>(0.0)))
  {
    cosTheta = -cosTheta;
    afterRotn[0] = -afterRotation[0];
    afterRotn[1] = -afterRotation[1];
    afterRotn[2] = -afterRotation[2];
    afterRotn[3] = -afterRotation[3];
  }
  else
  {
    afterRotn[0] = afterRotation[0];
    afterRotn[1] = afterRotation[1];
    afterRotn[2] = afterRotation[2];
    afterRotn[3] = afterRotation[3];
  }

  double scaleFactorForBefore, scaleFactorForAfter;

  if (((double)1.0 - cosTheta) > (double)0.0001) // 0.0001 -> some epsillon
  {
    double theta, sinTheta;
    theta = acos( cosTheta );
    sinTheta = sin( theta );
    scaleFactorForBefore  = sin( ((double)1.0 - weight) * theta ) / sinTheta;
    scaleFactorForAfter  = sin( weight * theta ) / sinTheta;
  }
  else
  {
    // Very close, do linear interp (because it's faster)
    scaleFactorForBefore = (double)1.0 - weight;
    scaleFactorForAfter = weight;
  }

  // Output is some proportion of 'before' and some proportion of 'after'.
  for (int i = 0; i < 4; i++)
  {
    outputRotation[i] = scaleFactorForBefore * beforeRotation[i] + scaleFactorForAfter * afterRotn[i];
  }
}


//-----------------------------------------------------------------------------
void InterpolateTransformationMatrix(const vtkMatrix4x4& before, const vtkMatrix4x4& after, const double& proportion, vtkMatrix4x4& interpolated)
{
  if (proportion == 0)
  {
    interpolated.DeepCopy(&before);
    return;
  }
  if (proportion == 1)
  {
    interpolated.DeepCopy(&after);
    return;
  }

  double beforeRotation[4];
  niftk::MatrixToQuaternion(before, beforeRotation);

  double afterRotation[4];
  niftk::MatrixToQuaternion(after, afterRotation);

  double interpolatedRotationQuaternion[4] = {0, 0, 0, 0};
  niftk::InterpolateRotation(beforeRotation, afterRotation, proportion, interpolatedRotationQuaternion, true);

  double interpolatedRotation[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
  vtkMath::QuaternionToMatrix3x3(interpolatedRotationQuaternion, interpolatedRotation);

  interpolated.Identity();
  double notProportion = 1.0 - proportion;

  for (int i = 0; i < 3; i++)
  {
    interpolated.SetElement(i, 0, interpolatedRotation[i][0]);
    interpolated.SetElement(i, 1, interpolatedRotation[i][1]);
    interpolated.SetElement(i, 2, interpolatedRotation[i][2]);
    interpolated.SetElement(i, 3, (before.GetElement(i, 3)*notProportion + after.GetElement(i, 3)*proportion));
  }
}

//-----------------------------------------------------------------------------
} // end namespace


