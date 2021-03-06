/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>
#include <cstdlib>

#include <mitkTestingMacros.h>
#include <mitkDataStorage.h>
#include <mitkStandaloneDataStorage.h>

#include <niftkDataStorageUtils.h>
#include <niftkAffineTransformDataNodeProperty.h>

bool CompareMatrix ( vtkMatrix4x4* matrix, double* result, double tolerance)
{
  double residual = 0.0;
  for ( unsigned int i = 0 ; i < 4 ;  ++i )
  {
    for ( unsigned int j = 0 ; j < 4 ; ++j)
    {
      residual +=  std::fabs(matrix->GetElement (i,j) - result [i*4 + j]);
    }
  }

  if ( residual <= tolerance )
  {
    return true;
  }

  return false;
}

namespace niftk
{

//-----------------------------------------------------------------------------
void TestLoadMatrixOrCreateDefault( std::string filename )
{
  mitk::DataStorage::Pointer dataStorage;
  dataStorage = mitk::StandaloneDataStorage::New();

  bool helperObject=true;
  niftk::LoadMatrixOrCreateDefault ("nonsense.nonsense", "TestMatrix" , helperObject, dataStorage);

  mitk::DataNode::Pointer node;

  node=dataStorage->GetNamedNode("TestMatrix");
  MITK_TEST_CONDITION_REQUIRED ( node.IsNotNull() , "Testing node exists" );
  bool isHelper;
  node->GetBoolProperty ("helper object", isHelper);
  MITK_TEST_CONDITION_REQUIRED ( isHelper , "Testing node is helper object");

  dataStorage->Remove(node);
  node=dataStorage->GetNamedNode("TestMatrix");
  MITK_TEST_CONDITION_REQUIRED ( node.IsNull() , "Testing node successfully removed" );

  helperObject=false;
  niftk::LoadMatrixOrCreateDefault ("nonsense.nonsense", "TestMatrix" , helperObject, dataStorage);

  node=dataStorage->GetNamedNode("TestMatrix");
  node->GetBoolProperty ("helper object", isHelper);
  MITK_TEST_CONDITION_REQUIRED ( ! isHelper , "Testing node is not helper object");

  std::string propertyName = "niftk.transform";
  niftk::AffineTransformDataNodeProperty::Pointer affTransProp;

  affTransProp= static_cast<niftk::AffineTransformDataNodeProperty*>(node->GetProperty(propertyName.c_str()));

  MITK_TEST_CONDITION_REQUIRED ( affTransProp, "Testing that node has affine transform property");
  vtkSmartPointer<vtkMatrix4x4> matrix;

  matrix = &affTransProp->GetTransform();
  MITK_TEST_CONDITION_REQUIRED ( matrix != NULL, "Testing that node has a matrix");

  double result [16]={ 1.0,0.0,0.0,0.0,
                       0.0,1.0,0.0,0.0,
                       0.0,0.0,1.0,0.0,
                       0.0,0.0,0.0,1.0};
  MITK_TEST_CONDITION_REQUIRED ( CompareMatrix (matrix, result,0.0) , "Testing that element 0,0 is 1.0");

  niftk::LoadMatrixOrCreateDefault (filename, "TestMatrix" , helperObject, dataStorage);
  node=dataStorage->GetNamedNode("TestMatrix");
  affTransProp= static_cast<niftk::AffineTransformDataNodeProperty*>(node->GetProperty(propertyName.c_str()));
  matrix = &affTransProp->GetTransform();
  double actualResult[16] = {0.9835, -0.0196, 0.1796, 11.1213,
                             0.1501, -0.4652, -0.8724, 8.0067,
                             0.1007, 0.8850, -0.4546, -6.4524,
                             0.0, 0.0, 0.0, 1.0};
  MITK_TEST_CONDITION_REQUIRED ( CompareMatrix (matrix, actualResult,0.0)  , "Testing that " << filename << " was loaded." );


}

}
/**
 * Basic test harness for niftkDataStorageUtils
 */
int niftkDataStorageUtilsTest(int argc, char * argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("niftkDataStorageUtilsTest");

  niftk::TestLoadMatrixOrCreateDefault(argv[1]);

  MITK_TEST_END();
}
