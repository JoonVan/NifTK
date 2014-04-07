/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef mitkNamedLookupTableProperty_h
#define mitkNamedLookupTableProperty_h

#include "niftkCoreExports.h"
#include <mitkLookupTableProperty.h>

namespace mitk {

/**
 * \class NamedLookupTableProperty
 * \brief Provides a property so we can see the lookup table name in the property window.
 * \ingroup uk_ac_ucl_cmic_imagelookuptables_internal
 */
class NIFTKCORE_EXPORT NamedLookupTableProperty : public mitk::LookupTableProperty
{

public:

  mitkClassMacro(NamedLookupTableProperty, mitk::LookupTableProperty);
  itkNewMacro(NamedLookupTableProperty);
  mitkNewMacro2Param(NamedLookupTableProperty, const std::string&, const mitk::LookupTable::Pointer);

  virtual std::string GetValueAsString() const;

  itkSetStringMacro(Name);
  itkGetStringMacro(Name);

protected:

  virtual ~NamedLookupTableProperty();
  NamedLookupTableProperty();
  NamedLookupTableProperty(const NamedLookupTableProperty& other);
  NamedLookupTableProperty(const std::string& name, const mitk::LookupTable::Pointer lut);

private:

  NamedLookupTableProperty& operator=(const NamedLookupTableProperty&); // Purposefully not implemented
  itk::LightObject::Pointer InternalClone() const;

  virtual bool IsEqual(const BaseProperty& property) const;
  virtual bool Assign(const BaseProperty& property);

  std::string m_Name;
};

} // namespace mitk

#endif
