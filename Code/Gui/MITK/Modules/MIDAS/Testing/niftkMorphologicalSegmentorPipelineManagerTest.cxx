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
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>
#include <mitkDataStorage.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkDataNode.h>
#include <mitkImageAccessByItk.h>
#include <mitkToolManager.h>
#include <mitkGeometry3D.h>
#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>
#include <mitkBaseRenderer.h>
#include <mitkRenderingManager.h>
#include <mitkRenderWindow.h>
#include <mitkFocusManager.h>
#include <mitkGlobalInteraction.h>

#include <niftkMIDASTool.h>
#include <niftkMIDASPaintbrushTool.h>
#include <mitkNifTKCoreObjectFactory.h>
#include <niftkMorphologicalSegmentorPipelineManager.h>

/**
 * \brief Test class for niftkMorphologicalSegmentorPipelineManagerTest.
 */
class niftkMorphologicalSegmentorPipelineManagerTestClass
{

public:

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ToolManager;
  mitk::RenderWindow::Pointer m_RenderWindow;
  mitk::RenderingManager::Pointer m_RenderingManager;
  niftk::MorphologicalSegmentorPipelineManager::Pointer m_PipelineManager;

  //-----------------------------------------------------------------------------
  void Setup(char* argv[])
  {
    std::string fileName = argv[1];

    mitk::GlobalInteraction::GetInstance()->Initialize("niftkMIDASPaintbrushToolClass");

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_ToolManager = mitk::ToolManager::New(m_DataStorage);

    m_PipelineManager = niftk::MorphologicalSegmentorPipelineManager::New();
    m_PipelineManager->SetDataStorage(m_DataStorage);
    m_PipelineManager->SetToolManager(m_ToolManager);

    // Load the single image.
    std::vector<std::string> files;
    files.push_back(fileName);
    mitk::DataStorage::SetOfObjects::Pointer allImages = mitk::IOUtil::Load(files, *(m_DataStorage.GetPointer()));
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(allImages->size(), 1),".. Testing 1 images loaded.");

    m_RenderingManager = mitk::RenderingManager::GetInstance();
    m_RenderingManager->SetDataStorage(m_DataStorage);

    m_RenderWindow = mitk::RenderWindow::New(NULL, "niftkMorphologicalSegmentorPipelineManagerTestClass", m_RenderingManager);

    m_RenderingManager->InitializeViews(m_DataStorage->ComputeVisibleBoundingGeometry3D());
  }


  //-----------------------------------------------------------------------------
  void TestAll()
  {
    MITK_TEST_OUTPUT(<< "Starting TestAll...");

    MITK_TEST_CONDITION( true, ".. Testing force false");

    MITK_TEST_OUTPUT(<< "Finished TestAll...");
  }
};

/**
 * Basic test harness for niftkMorphologicalSegmentorPipelineManager.
 */
int niftkMorphologicalSegmentorPipelineManagerTest(int argc, char * argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("niftkMorphologicalSegmentorPipelineManagerTest");

  // We are testing specifically with image ${NIFTK_DATA_DIR}/Input/volunteers/16856/16856-002-1.img.

  niftkMorphologicalSegmentorPipelineManagerTestClass *testClass = new niftkMorphologicalSegmentorPipelineManagerTestClass();
  testClass->Setup(argv);
  testClass->TestAll();
  delete testClass;
  MITK_TEST_END();
}
