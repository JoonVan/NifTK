/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkPointBasedRegistration.h"
#include <limits>
#include <mitkPointUtils.h>
#include <mitkNavigationDataLandmarkTransformFilter.h>
#include <mitkArunLeastSquaresPointRegistrationWrapper.h>


const bool mitk::PointBasedRegistration::DEFAULT_USE_ICP_INITIALISATION(false);
const bool mitk::PointBasedRegistration::DEFAULT_USE_POINT_ID_TO_MATCH(false);
const bool mitk::PointBasedRegistration::DEFAULT_USE_SVD_BASED_METHOD(true);
const bool mitk::PointBasedRegistration::DEFAULT_STRIP_NAN_FROM_INPUT(true);

namespace mitk
{

//-----------------------------------------------------------------------------
PointBasedRegistration::PointBasedRegistration()
: m_UseICPInitialisation(DEFAULT_USE_ICP_INITIALISATION)
, m_UsePointIDToMatchPoints(DEFAULT_USE_POINT_ID_TO_MATCH)
, m_UseSVDBasedMethod(DEFAULT_USE_SVD_BASED_METHOD)
, m_StripNaNFromInput(DEFAULT_STRIP_NAN_FROM_INPUT)
{
}


//-----------------------------------------------------------------------------
PointBasedRegistration::~PointBasedRegistration()
{
}


//-----------------------------------------------------------------------------
bool PointBasedRegistration::Update(
    const mitk::PointSet::Pointer fixedPointSet,
    const mitk::PointSet::Pointer movingPointSet,
    vtkMatrix4x4& outputTransform,
    double& fiducialRegistrationError) const

{

  assert(fixedPointSet);
  assert(movingPointSet);

  bool isSuccessful = false;

  fiducialRegistrationError = std::numeric_limits<double>::max();
  outputTransform.Identity();

  mitk::PointSet::Pointer filteredFixedPoints = mitk::PointSet::New();
  mitk::PointSet::Pointer filteredMovingPoints = mitk::PointSet::New();
  mitk::PointSet::Pointer noNaNFixedPoints = mitk::PointSet::New();
  mitk::PointSet::Pointer noNaNMovingPoints = mitk::PointSet::New();
  mitk::PointSet* fixedPoints = fixedPointSet;
  mitk::PointSet* movingPoints = movingPointSet;

  bool useICPInit = m_UseICPInitialisation;

  if (m_StripNaNFromInput)
  {
    int fixedPointsRemoved = mitk::RemoveNaNPoints(*fixedPointSet, *noNaNFixedPoints);
    int movingPointsRemoved = mitk::RemoveNaNPoints(*movingPointSet, *noNaNMovingPoints);

    if ( fixedPointsRemoved != 0 ) 
    {
      MITK_INFO << "Removed " << fixedPointsRemoved << " NaN points from fixed data";
    }
    if ( movingPointsRemoved != 0 ) 
    {
      MITK_INFO << "Removed " << movingPointsRemoved << " NaN points from moving data";
    }

    fixedPoints = noNaNFixedPoints;
    movingPoints = noNaNMovingPoints;

  }
  if (m_UsePointIDToMatchPoints)
  {

    int numberOfFilteredPoints = mitk::FilterMatchingPoints(*fixedPoints,
                                                            *movingPoints,
                                                            *filteredFixedPoints,
                                                            *filteredMovingPoints
                                                            );

    if (numberOfFilteredPoints >= 3)
    {
      fixedPoints = filteredFixedPoints;
      movingPoints = filteredMovingPoints;
    }
    else
    {
      MITK_DEBUG << "mitk::PointBasedRegistration: filteredFixedPoints size=" << filteredFixedPoints->GetSize() << ", filteredMovingPoints size=" << filteredMovingPoints->GetSize() << ", abandoning use of filtered data sets.";
      return isSuccessful;
    }
  }

  if (fixedPoints->GetSize() < 3 || movingPoints->GetSize() < 3)
  {
    MITK_DEBUG << "mitk::PointBasedRegistration:: fixedPoints size=" << fixedPoints->GetSize() << ", movingPoints size=" << movingPoints->GetSize() << ", abandoning point based registration";
    return isSuccessful;
  }

  if (fixedPoints->GetSize() != movingPoints->GetSize() && !m_UseICPInitialisation && !m_UseSVDBasedMethod)
  {
    MITK_DEBUG << "mitk::PointBasedRegistration: Switching to use ICP Initialisation for mitk::NavigationDataLandmarkTransformFilter";
    useICPInit = true;
  }

  if (m_UseSVDBasedMethod)
  {
    mitk::ArunLeastSquaresPointRegistrationWrapper::Pointer registration = mitk::ArunLeastSquaresPointRegistrationWrapper::New();
    isSuccessful = registration->Update(fixedPoints, movingPoints, outputTransform, fiducialRegistrationError);

    if (!isSuccessful)
    {
      MITK_ERROR << "mitk::PointBasedRegistration: SVD method failed" << std::endl;
      return isSuccessful;
    }
  }
  else
  {
    mitk::NavigationDataLandmarkTransformFilter::LandmarkTransformType::ConstPointer transform = NULL;
    mitk::NavigationDataLandmarkTransformFilter::LandmarkTransformType::MatrixType rotationMatrix;
    mitk::NavigationDataLandmarkTransformFilter::LandmarkTransformType::OffsetType translationVector;
    rotationMatrix.SetIdentity();
    translationVector.Fill(0);

    mitk::NavigationDataLandmarkTransformFilter::Pointer transformFilter = mitk::NavigationDataLandmarkTransformFilter::New();
    transformFilter->SetUseICPInitialization(m_UseICPInitialisation);
    transformFilter->SetTargetLandmarks(fixedPoints);
    transformFilter->SetSourceLandmarks(movingPoints);
    transformFilter->Update();

    MITK_INFO << "PointBasedRegistration: FRE=" << transformFilter->GetFRE() << "mm (Std. Dev. " << transformFilter->GetFREStdDev() << ")" << std::endl;
    MITK_INFO << "PointBasedRegistration: RMS=" << transformFilter->GetRMSError() << "mm " << std::endl;
    MITK_INFO << "PointBasedRegistration: min=" << transformFilter->GetMinError() << "mm" << std::endl;
    MITK_INFO << "PointBasedRegistration: max=" << transformFilter->GetMaxError() << "mm" << std::endl;

    fiducialRegistrationError = transformFilter->GetFRE();
    transform = transformFilter->GetLandmarkTransform();
    rotationMatrix = transform->GetMatrix();
    translationVector = transform->GetOffset();

    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        outputTransform.SetElement(i, j, rotationMatrix[i][j]);
      }
      outputTransform.SetElement(i, 3, translationVector[i]);
    }

    // The ICP method doesn't really have an accept / fail criteria.
    // For now we are mainly using the SVD method above. So, we just assume this bit is always successful.
    isSuccessful = true;
  }
  return isSuccessful;
}

} // end namespace

