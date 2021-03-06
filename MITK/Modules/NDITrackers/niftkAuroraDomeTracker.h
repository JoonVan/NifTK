/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkAuroraDomeTracker_h
#define niftkAuroraDomeTracker_h

#include <niftkNDITrackersExports.h>
#include "niftkPLUSNDITracker.h"

namespace niftk
{

/**
 * \class AuroraDomeTracker
 * \brief RAII object to connect to Aurora Dome tracker.
 */
class NIFTKNDITRACKERS_EXPORT AuroraDomeTracker : public niftk::PLUSNDITracker
{
public:

  mitkClassMacroItkParent(AuroraDomeTracker, niftk::PLUSNDITracker)
  mitkNewMacro4Param(AuroraDomeTracker, mitk::DataStorage::Pointer, std::string, std::string, int)

protected:

  AuroraDomeTracker(mitk::DataStorage::Pointer dataStorage,
                    std::string portName,
                    std::string toolConfigFileName,
                    int baudRate
                    ); // Purposefully hidden.

  virtual ~AuroraDomeTracker(); // Purposefully hidden.

  AuroraDomeTracker(const AuroraDomeTracker&); // Purposefully not implemented.
  AuroraDomeTracker& operator=(const AuroraDomeTracker&); // Purposefully not implemented.

private:

}; // end class

} // end namespace

#endif
