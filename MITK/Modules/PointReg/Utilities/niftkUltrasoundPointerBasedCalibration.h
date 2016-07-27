/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkUltrasoundPointerBasedCalibration_h
#define niftkUltrasoundPointerBasedCalibration_h

#include <niftkPointRegExports.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <mitkPointSet.h>
#include <mitkOperation.h>
#include <itkObject.h>
#include <itkObjectFactoryBase.h>

namespace niftk {

/**
* \class UltrasoundPointerBasedCalibration
* \brief Calibrates an Ultrasound Probe using LM optimisation, as described in Muratore 2001.
*/
class NIFTKPOINTREG_EXPORT UltrasoundPointerBasedCalibration : public itk::Object
{
public:

  mitkClassMacroItkParent(UltrasoundPointerBasedCalibration, itk::Object)
  itkNewMacro(UltrasoundPointerBasedCalibration)

  /**
  * \brief Returns (copies) the calibration matrix (rigid body and scaling)
  */
  vtkSmartPointer<vtkMatrix4x4> GetCalibrationMatrix() const;

  /**
  * \brief Returns (copies) the rigid body transformation.
  */
  vtkSmartPointer<vtkMatrix4x4> GetRigidBodyMatrix() const;

  /**
  * \brief Returns (copies) the scaling transformation.
  */
  vtkSmartPointer<vtkMatrix4x4> GetScalingMatrix() const;

  /**
  * \brief Gives this object a pointer to the Sensor ('fixed') points (they are not copied).
  */
  void SetSensorPoints(mitk::PointSet::Pointer points);

  /**
  * \brief Gives this object a pointer to the Image ('moving') points (they are not copied).
  */
  void SetImagePoints(mitk::PointSet::Pointer points);

  /**
  * \brief Performs calibration.
  * \return RMS residual error.
  *
  * \see GetRegistrationMatrix()
  * \see GetScalingMatrix()
  */
  double DoPointerBasedCalibration();

protected:

  UltrasoundPointerBasedCalibration(); // Purposefully hidden.
  virtual ~UltrasoundPointerBasedCalibration(); // Purposefully hidden.

  UltrasoundPointerBasedCalibration(const UltrasoundPointerBasedCalibration&); // Purposefully not impl.
  UltrasoundPointerBasedCalibration& operator=(const UltrasoundPointerBasedCalibration&); // Purposefully not impl.

private:

  vtkSmartPointer<vtkMatrix4x4> m_ScalingMatrix;
  vtkSmartPointer<vtkMatrix4x4> m_RigidBodyMatrix;
  mitk::PointSet::Pointer       m_UltrasoundImagePoints;
  mitk::PointSet::Pointer       m_SensorPoints;
}; // end class

} // end namespace

#endif
