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
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <mitkTestingMacros.h>
#include <mitkDataNode.h>
#include <mitkDataStorage.h>
#include <mitkRenderingTestHelper.h>
#include <vtkRegressionTestImage.h>
#include <mitkCoordinateAxesData.h>
#include <mitkCoordinateAxesVtkMapper3D.h>
#include <mitkNifTKCoreObjectFactory.h>

/**
 * \file Test harness for mitk::CoordinateAxesData and mitk::CoordinateAxesVtkMapper3D.
 */
int mitkCoordinateAxesDataRenderingTest(int argc, char * argv[])
{
  // Always start with this, with name of function.
  MITK_TEST_BEGIN("mitkCoordinateAxesDataRenderingTest");

  // Create axes node
  mitk::CoordinateAxesData::Pointer axes = mitk::CoordinateAxesData::New();
  mitk::DataNode::Pointer axesNode = mitk::DataNode::New();
  axesNode->SetData(axes);
  axesNode->SetVisibility(true);
  axesNode->SetIntProperty("size", 1);
  axesNode->SetBoolProperty("show text", true);

  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  matrix->Identity();
  axes->SetVtkMatrix(*matrix);

  // Create a rendering helper - contains window and data storage.
  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);
  renderingHelper.SetMapperID(mitk::BaseRenderer::Standard3D);
  renderingHelper.AddNodeToStorage(axesNode);

  // Set up the view.
  mitk::DataStorage::Pointer storage = renderingHelper.GetDataStorage();
  mitk::TimeGeometry::Pointer geometry =  storage->ComputeBoundingGeometry3D(renderingHelper.GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geometry );

  // Get image out
  renderingHelper.Render();
  renderingHelper.GetVtkRenderer()->ResetCamera();
  renderingHelper.Render();
  //renderingHelper.SaveAsPNG("/scratch0/NOT_BACKED_UP/clarkson/build/NifTK-SuperBuild-Debug/NifTK-build/output.png");

  int retVal = vtkRegressionTestImage( renderingHelper.GetVtkRenderWindow() );
  //retVal meanings: (see VTK/Rendering/vtkTesting.h)
  //0 = test failed
  //1 = test passed
  //2 = test not run
  //3 = something with vtkInteraction
  MITK_TEST_CONDITION( retVal == 1, "Checking test result is 1, but in fact it is:" << retVal);

  MITK_TEST_END();
}

