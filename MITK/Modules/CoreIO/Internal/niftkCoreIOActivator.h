/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkCoreIOActivator_h
#define niftkCoreIOActivator_h

#include <memory>

#include <mitkIFileReader.h>
#include <mitkIFileWriter.h>

#include <usModuleActivator.h>
#include <usModuleContext.h>

#include "niftkCoordinateAxesDataReaderService.h"
#include "niftkCoordinateAxesDataWriterService.h"
#include "niftkLabelMapReader.h"
#include "niftkLabelMapWriter.h"
#include "niftkLookupTableProviderService.h"
#include "niftkPNMReaderService.h"
#include "niftkPNMWriterService.h"

namespace niftk
{

/**
* @class CoreIOActivator
*
* @brief The CoreIOActivator class
*/
class CoreIOActivator : public us::ModuleActivator
{
public:

  CoreIOActivator();
  void Load(us::ModuleContext* context) override;
  void Unload(us::ModuleContext* ) override;

private:

  std::auto_ptr<niftk::CoordinateAxesDataReaderService> m_CoordinateAxesDataReaderService;
  std::auto_ptr<niftk::CoordinateAxesDataWriterService> m_CoordinateAxesDataWriterService;

  std::auto_ptr<niftk::PNMReaderService>                m_PNMReaderService;
  std::auto_ptr<niftk::PNMWriterService>                m_PNMWriterService;

  std::auto_ptr<LookupTableProviderService>             m_LookupTableProviderService;
  std::auto_ptr<niftk::LabelMapReader>                  m_LabelMapReaderService;
  std::auto_ptr<niftk::LabelMapWriter>                  m_LabelMapWriterService;

};

}

#endif
