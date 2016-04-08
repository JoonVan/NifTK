/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkITKRegionParametersDataNodeProperty_h
#define mitkITKRegionParametersDataNodeProperty_h

#include "niftkCoreExports.h"
#include <mitkBaseProperty.h>
#include <algorithm>

namespace mitk
{

/**
 * \class ITKRegionParametersDataNodeProperty
 * \brief MITK data-node property suitable for holding an ITK Region, consisting of a "valid" flag,
 * plus 6 parameters containing the index and size, as a simple vector of integers.
 */
class NIFTKCORE_EXPORT ITKRegionParametersDataNodeProperty : public BaseProperty
{

public:

  mitkClassMacro(ITKRegionParametersDataNodeProperty, BaseProperty);
  itkNewMacro(ITKRegionParametersDataNodeProperty);

  /**
   * \brief Parameters are 6 integers, corresponding to index[X, Y, Z] and size[X, Y, Z].
   */
  typedef std::vector<int> ParametersType;

  /**
   * \brief Get the region parameters from this property object where index[X, Y, Z] = [0-2], and size[X, Y, Z] = [3-5].
   */
  const ParametersType& GetITKRegionParameters() const;

  /**
   * \brief Set the region parameters on this property object where index[X, Y, Z] = [0-2], and size[X, Y, Z] = [3-5].
   */
  void SetITKRegionParameters(const ParametersType& parameters);

  /**
   * \brief Returns true of the size of the volume is at least 1 voxel (eg. 1x1x1).
   */
  bool HasVolume() const;

  /**
   * \brief Sets the index.
   */
  void SetIndex(int x, int y, int z);

  /**
   * \brief Sets the size.
   */
  void SetSize(int x, int y, int z);

  /**
   * \brief Gets the 'valid' status flag.
   */
  bool IsValid() const;

  /**
   * \brief Sets the 'valid' status flag.
   */
  void SetValid(bool valid);

  /**
   * \brief Defined in base class, returns the current value as a string for display in property view.
   */
  virtual std::string GetValueAsString() const override;

  /**
   * \brief Method to set these parameters back to identity, which is [false, 0, 0, 0, 0, 0, 0].
   */
  virtual void Identity();

protected:

  virtual ~ITKRegionParametersDataNodeProperty();
  ITKRegionParametersDataNodeProperty();                                                 // Purposefully hidden.
  ITKRegionParametersDataNodeProperty(const ITKRegionParametersDataNodeProperty& other); // Purposefully hidden.

  /**
   * \see mitk::BaseProperty::IsEqual()
   */
  virtual bool IsEqual(const BaseProperty& property) const override;

  /**
   * \see mitk::BaseProperty::Assign()
   */
  virtual bool Assign(const BaseProperty& ) override;

private:

  ITKRegionParametersDataNodeProperty& operator=(const ITKRegionParametersDataNodeProperty&); // Purposefully not implemented.
  itk::LightObject::Pointer InternalClone() const override;

  ParametersType m_Parameters;
  bool m_IsValid;
};

} // namespace mitk

#endif
