/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkDataNodeBoolPropertyFilter_h
#define niftkDataNodeBoolPropertyFilter_h

#include "niftkCoreExports.h"
#include "niftkDataNodeFilter.h"

#include <mitkDataStorage.h>

namespace niftk
{

class BaseRenderer;

/**
 * \class DataNodeBoolPropertyFilter
 *
 * \brief A filter that contains a list of renderers, and returns true if the node has
 * a specific boolean property set to true for those filters.
 */
class NIFTKCORE_EXPORT DataNodeBoolPropertyFilter : public DataNodeFilter
{

public:

  mitkClassMacro(DataNodeBoolPropertyFilter, DataNodeFilter)
  itkNewMacro(DataNodeBoolPropertyFilter)

  /// \brief Sets the property name used for filtering.
  itkSetMacro(PropertyName, std::string);

  /// \brief Gets the property name used for filtering.
  itkGetMacro(PropertyName, std::string);

  ///
  /// \brief Method to decide if the node should be passed.
  ///
  /// \param node a candidate node
  /// \return bool true if the node should pass and false otherwise.
  virtual bool Pass(const mitk::DataNode* node) override;

  /// \brief Sets the list of renderers to check.
  void SetRenderers(std::vector<mitk::BaseRenderer*>& list);

  /// \brief Sets the DataStorage against which to check.
  void SetDataStorage(mitk::DataStorage::Pointer storage);

protected:

  DataNodeBoolPropertyFilter();
  virtual ~DataNodeBoolPropertyFilter();

  DataNodeBoolPropertyFilter(const DataNodeBoolPropertyFilter&); // Purposefully not implemented.
  DataNodeBoolPropertyFilter& operator=(const DataNodeBoolPropertyFilter&); // Purposefully not implemented.

private:

  std::vector<mitk::BaseRenderer*> m_Renderers;
  mitk::DataStorage::Pointer m_DataStorage;
  std::string m_PropertyName;

};

}

#endif
