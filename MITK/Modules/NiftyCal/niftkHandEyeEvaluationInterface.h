/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkHandEyeEvaluationInterface_h
#define niftkHandEyeEvaluationInterface_h

#include "niftkNiftyCalExports.h"
#include <string>

namespace niftk
{

NIFTKNIFTYCAL_EXPORT double EvaluateHandeyeFromPoints(const std::string& trackingDir,
                                                      const std::string& pointsDir,
                                                      const std::string& modelFile,
                                                      const std::string& intrinsicsFile,
                                                      const std::string& handeyeFile,
                                                      const std::string& registrationFile,
                                                      const unsigned int& lagInMilliseconds
                                                     );
} // end namespace

#endif
