#/*============================================================================
#
#  NifTK: A software platform for medical image computing.
#
#  Copyright (c) University College London (UCL). All rights reserved.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.
#
#  See LICENSE.txt in the top level directory for details.
#
#============================================================================*/

MITK_CREATE_MODULE_TESTS(LABELS niftkCore)

if(TARGET ${TESTDRIVER})

  mitk_use_modules(TARGET ${TESTDRIVER} PACKAGES VTK|vtkTestingRendering)

  # Test "As Acquired" with known images, default Coronal, which is not needed here, as we should be able to work it out.
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_16856   niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/16856/16856-002-1.img 2 2)
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_01719-a niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-a.img 2 0)
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_01719-s niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-s.img 2 1)
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_01719-c niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-c.img 2 2)

  # Test "As Acquired", changing default to make sure we are not needing it, output should always be sagittal.
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_30257-0  niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/30257/mdeft_nifti/mdeft.1.nii 0 1)
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_30257-1  niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/30257/mdeft_nifti/mdeft.1.nii 1 1)
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_30257-2  niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/30257/mdeft_nifti/mdeft.1.nii 2 1)
  mitkAddCustomModuleTest(MIDAS-mitkAsAcquiredOrientationTest_30257-14 niftkAsAcquiredOrientationTest ${NIFTK_DATA_DIR}/Input/volunteers/30257/mdeft_nifti/mdeft.1.nii 14 1)

  # Test that analyze images load the same as Nifti images. Here we test intensity values, assuming same size.
  mitkAddCustomModuleTest(MIDAS-mitkDRCAnalyzeSameIntensitiesAsNiftiTest_01719-a niftkCompareImagesForEqualityTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-a.img ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-a.nii 0)
  mitkAddCustomModuleTest(MIDAS-mitkDRCAnalyzeSameIntensitiesAsNiftiTest_01719-s niftkCompareImagesForEqualityTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-s.img ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-s.nii 0)
  mitkAddCustomModuleTest(MIDAS-mitkDRCAnalyzeSameIntensitiesAsNiftiTest_01719-c niftkCompareImagesForEqualityTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-c.img ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-c.nii 0)

  # Test that if we load the same Nifti image twice, the "is equals" checks all do return true.
  mitkAddCustomModuleTest(MIDAS-mitkTwoNiftiImagesAreEqualTest niftkCompareImagesForEqualityTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-a.nii ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-a.nii 0)

  # Trac 1696 - Unit tests for niftkImageOrientationUtils - testing things like, which axis is "up", which direction is "up", as these map to A/Z keys etc.
  mitkAddCustomModuleTest(MIDAS-niftkImageOrientationUtilsTest-01719-a niftkImageOrientationUtilsTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-a.img RAI  1 -1 -1 2 0 1)
  mitkAddCustomModuleTest(MIDAS-niftkImageOrientationUtilsTest-01719-s niftkImageOrientationUtilsTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-s.img ASL -1  1 -1 1 2 0)
  mitkAddCustomModuleTest(MIDAS-niftkImageOrientationUtilsTest-01719-c niftkImageOrientationUtilsTest ${NIFTK_DATA_DIR}/Input/volunteers/01719/01719-012-c.img RSP -1 -1  1 1 0 2)
  mitkAddCustomModuleTest(MIDAS-niftkImageOrientationUtilsTest-16856   niftkImageOrientationUtilsTest ${NIFTK_DATA_DIR}/Input/volunteers/16856/16856-002-1.img RSP -1 -1  1 1 0 2)
  mitkAddCustomModuleTest(MIDAS-niftkImageOrientationUtilsTest-30257   niftkImageOrientationUtilsTest ${NIFTK_DATA_DIR}/Input/volunteers/30257/mdeft_nifti/mdeft.1.nii AIL 1 1 -1 1 2 0)

  # Trac 1706 - Dilations break through connection breaker image.
  mitkAddCustomModuleTest(MIDAS-niftkImageUtilsTest-16856 niftkImageUtilsTest ${NIFTK_DATA_DIR}/Input/volunteers/16856/16856-002-1.img)

  # Test AND, and OR of binary masks
  mitkAddCustomModuleTest(BinaryMask-AND niftkBinaryMaskTest ${NIFTK_DATA_DIR}/Input/BinaryMask/input1.png ${NIFTK_DATA_DIR}/Input/BinaryMask/input2.png 0 ${NIFTK_DATA_DIR}/Input/BinaryMask/expectedOutputAnd.png)
  mitkAddCustomModuleTest(BinaryMask-OR niftkBinaryMaskTest ${NIFTK_DATA_DIR}/Input/BinaryMask/input1.png ${NIFTK_DATA_DIR}/Input/BinaryMask/input2.png 1 ${NIFTK_DATA_DIR}/Input/BinaryMask/expectedOutputOr.png)

  # Test datastorageutilts
  mitkAddCustomModuleTest(DataStorageUtils niftkDataStorageUtilsTest  ${NIFTK_DATA_DIR}/Input/CameraCalibration/HandeyeFromDirectory/CertusCalibration/calib.left.handeye.txt)

endif()
