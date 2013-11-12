/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkSurfaceBasedRegistration.h"
#include <niftkVTKIterativeClosestPoint.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <mitkFileIOUtils.h>
#include <mitkCoordinateAxesData.h>
#include <mitkAffineTransformDataNodeProperty.h>
#include <niftkVTKFunctions.h>

namespace mitk
{
 
const int SurfaceBasedRegistration::DEFAULT_MAX_ITERATIONS = 100;
const int SurfaceBasedRegistration::DEFAULT_MAX_POINTS = 100;
const bool SurfaceBasedRegistration::DEFAULT_USE_DEFORMABLE = false;
const bool SurfaceBasedRegistration::DEFAULT_USE_SPATIALFILTER = false;
//-----------------------------------------------------------------------------
SurfaceBasedRegistration::SurfaceBasedRegistration()
:m_MaximumIterations(50)
,m_MaximumNumberOfLandmarkPointsToUse(200)
,m_Method(VTK_ICP)
,m_UseSpatialFilter(DEFAULT_USE_SPATIALFILTER)
,m_Matrix(NULL)
{
  m_Matrix = vtkMatrix4x4::New();
}


//-----------------------------------------------------------------------------
SurfaceBasedRegistration::~SurfaceBasedRegistration()
{
}

//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::RunVTKICP(vtkPolyData* fixedPoly,
                                      vtkPolyData* movingPoly,
                                      vtkMatrix4x4 * transformMovingToFixed)
{
  niftk::VTKIterativeClosestPoint * icp = new  niftk::VTKIterativeClosestPoint();
  icp->SetMaxLandmarks(m_MaximumNumberOfLandmarkPointsToUse);
  icp->SetMaxIterations(m_MaximumIterations);
  icp->SetSource(movingPoly);
  icp->SetTarget(fixedPoly);
  icp->Run();
  vtkMatrix4x4 * temp;
  temp = icp->GetTransform();
  transformMovingToFixed->DeepCopy(temp);
  m_Matrix = vtkMatrix4x4::New();
  m_Matrix->DeepCopy(temp);
}

void SurfaceBasedRegistration::Update(const mitk::DataNode* fixedNode, 
                                       const mitk::DataNode* movingNode,
                                       vtkMatrix4x4 * transformMovingToFixed)
{
  if ( m_Method == VTK_ICP )
  {
    vtkPolyData * fixedPoly = vtkPolyData::New();
    NodeToPolyData ( fixedNode, fixedPoly, m_UseSpatialFilter);
    vtkPolyData * movingPoly = vtkPolyData::New();
    NodeToPolyData ( movingNode, movingPoly, m_UseSpatialFilter);
    RunVTKICP ( fixedPoly, movingPoly, transformMovingToFixed );
  }
  if ( m_Method == DEFORM )
  {
    //Not Implenented
  }

}

//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::PointSetToPolyData (const  mitk::PointSet::Pointer PointsIn, vtkPolyData* PolyOut )
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  int numberOfPoints = PointsIn->GetSize();
  int i = 0 ;
  int PointsFound = 0 ;
  while ( PointsFound < numberOfPoints )
  {
    mitk::Point3D point;
    if ( PointsIn->GetPointIfExists(i,&point) )
    {
      points->InsertNextPoint(point[0],point[1],point[2]);
      PointsFound ++ ;
    }
    i++;
  }
  PolyOut->SetPoints(points);
}


//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::PointSetToPolyData_SpatialFilter (const  mitk::PointSet::Pointer PointsIn, vtkPolyData* PolyOut )
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  //assume input images were HD 1920�1080
  //points in point set are assigned ID = y * width + x
  int width=1920;
  int height=1080;

  //use a patch size of 15 x 9 pixels
  for ( int x = 7 ; x < width ; x += 15 )
  {
    for ( int y = 4 ; y < height ; y += 9 )
    {
      vtkSmartPointer<vtkPoints> temppoints = vtkSmartPointer<vtkPoints>::New();
      for ( int px = - 7 ; px <= 7 ; px ++ )
      {
        for ( int py = -4 ; py <= 4 ; py ++ )
        {
          int index = (y + py) * width + (x+px);
          mitk::Point3D point;
          if ( PointsIn->GetPointIfExists(index,&point) )
          {
            temppoints->InsertNextPoint(point[0],point[1],point[2]);
          }
        }
      }
      if ( temppoints->GetNumberOfPoints() > 0 )
      {
        int median = temppoints->GetNumberOfPoints() /2;
       // std::cerr << "Found " << temppoints->GetNumberOfPoints() << " in patch " << x << "," << y << " Picking point " << median << std::endl;
        points->InsertNextPoint(temppoints->GetPoint(median));
      }
    }
  }
  std::cerr << "Applied spatial Filter, " << points->GetNumberOfPoints() << " points left." << std::endl;

  PolyOut->SetPoints(points);
}


//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::NodeToPolyData (const  mitk::DataNode* node, vtkPolyData* PolyOut, bool useSpatialFilter)
{
  mitk::Surface::Pointer Surface = NULL;
  mitk::PointSet::Pointer Points = NULL;
  Surface = dynamic_cast<mitk::Surface*>(node->GetData());
  Points = dynamic_cast<mitk::PointSet*>(node->GetData());
  if ( Surface.IsNull() ) 
  {
    if ( useSpatialFilter ) 
    {
      PointSetToPolyData_SpatialFilter ( Points,PolyOut );
    }
    else
    {
      PointSetToPolyData ( Points,PolyOut );
    } 
  }
  else
  {
    vtkPolyData * polytemp = vtkPolyData::New();
    polytemp=Surface->GetVtkPolyData();
    vtkMatrix4x4 * indexToWorld = vtkMatrix4x4::New();
    GetCurrentTransform(node,indexToWorld);
    vtkTransform * transform = vtkTransform::New();
    transform->SetMatrix(indexToWorld);
    vtkTransformPolyDataFilter * transformFilter= vtkTransformPolyDataFilter::New();
    transformFilter->SetInput(polytemp);
    transformFilter->SetOutput(PolyOut);
    transformFilter->SetTransform(transform);
    transformFilter->Update();

  }
}


//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::ApplyTransform (mitk::DataNode::Pointer node)
{
  ApplyTransform(node, m_Matrix);
}


//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::ApplyTransform (mitk::DataNode::Pointer node , vtkMatrix4x4 * matrix)
{
  vtkMatrix4x4 * CurrentMatrix = vtkMatrix4x4::New();
  GetCurrentTransform (node , CurrentMatrix );
  vtkMatrix4x4 * NewMatrix = vtkMatrix4x4::New();
  matrix->Multiply4x4(matrix, CurrentMatrix, NewMatrix);
  mitk::CoordinateAxesData* transform = dynamic_cast<mitk::CoordinateAxesData*>(node->GetData());

  if (transform != NULL)
  {
    mitk::AffineTransformDataNodeProperty::Pointer property = dynamic_cast<mitk::AffineTransformDataNodeProperty*>(node->GetProperty("niftk.transform"));
    if (property.IsNull())
    {
      MITK_ERROR << "LiverSurgeryManager::SetTransformation the node " << node->GetName() << " does not contain the niftk.transform property" << std::endl;
      return;
    }

    transform->SetVtkMatrix(*NewMatrix);
    transform->Modified();

    property->SetTransform(*NewMatrix);
    property->Modified();
  }
  else
  {
    mitk::Geometry3D::Pointer geometry = node->GetData()->GetGeometry();
    if (geometry.IsNotNull())
    {
      geometry->SetIndexToWorldTransformByVtkMatrix(NewMatrix);
      geometry->Modified();
    }
  }
}


//-----------------------------------------------------------------------------
void SurfaceBasedRegistration::GetCurrentTransform (const mitk::DataNode* node, vtkMatrix4x4* Matrix)
{
  mitk::AffineTransform3D::Pointer affineTransform = node->GetData()->GetGeometry()->Clone()->GetIndexToWorldTransform();
  itk::Matrix<float, 3, 3>  matrix;
  itk::Vector<float, 3> offset;
  matrix = affineTransform->GetMatrix();
  offset = affineTransform->GetOffset();

  Matrix->Identity();
  for ( int i = 0 ; i < 3 ; i ++ ) 
  {
    for ( int j = 0 ; j < 3 ; j ++ )
    {
      Matrix->SetElement (i,j,matrix[i][j]);
    }
  }
  for ( int i = 0 ; i < 3 ; i ++ ) 
  {
    Matrix->SetElement (i, 3, offset[i]);
  }

}


} // end namespace

