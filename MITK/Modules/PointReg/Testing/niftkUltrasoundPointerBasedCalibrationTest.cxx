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

#include <cmath>

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <mitkExceptionMacro.h>
#include <mitkIOUtil.h>
#include <mitkLogMacros.h>
#include <mitkOpenCVMaths.h>
#include <mitkTestingMacros.h>

#include <niftkFileIOUtils.h>
#include <niftkFileHelper.h>
#include <niftkMathsUtils.h>
#include <niftkUltrasoundPointerBasedCalibration.h>

/**
 * \file Test harness for Ultrasound Pointer based calibration (Muratore 2001).
 */
int niftkUltrasoundPointerBasedCalibrationTest(int argc, char * argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("niftkUltrasoundPointerBasedCalibrationTest");

  if (argc != 5)
  {
    mitkThrow() << "Usage: niftkUltrasoundPointerBasedCalibrationTest image.mps sensor.mps expected.4x4 residual" << std::endl;
  }
  mitk::PointSet::Pointer imagePoints = mitk::IOUtil::LoadPointSet(argv[1]);
  mitk::PointSet::Pointer sensorPoints = mitk::IOUtil::LoadPointSet(argv[2]);
  vtkSmartPointer<vtkMatrix4x4> expectedResult = niftk::LoadVtkMatrix4x4FromFile(argv[3]);
  float expectedResidual = atof(argv[4]);

  niftk::UltrasoundPointerBasedCalibration::Pointer calibrator = niftk::UltrasoundPointerBasedCalibration::New();
  calibrator->SetImagePoints(imagePoints);
  calibrator->SetSensorPoints(sensorPoints);

  double residual = calibrator->DoPointerBasedCalibration();

  vtkSmartPointer<vtkMatrix4x4> actualResult = calibrator->GetCalibrationMatrix();
  niftk::SaveVtkMatrix4x4ToFile("/tmp/matt.4x4", *actualResult);

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      MITK_TEST_CONDITION_REQUIRED(niftk::IsCloseToZero(
                                     fabs(expectedResult->GetElement(i, j)
                                          - actualResult->GetElement(i, j)), 0.001),
                                   "Checking element " << i << ", " << j << " is correct, expecting "
                                   << expectedResult->GetElement(i,j) << ", and got "
                                   << actualResult->GetElement(i, j));
    }
  }
  MITK_TEST_CONDITION_REQUIRED(residual < expectedResidual, "Checking residual < " << expectedResidual << ", and got " << residual);
  MITK_TEST_END();
}


