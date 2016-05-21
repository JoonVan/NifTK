/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkMIDASPosnTool_h
#define niftkMIDASPosnTool_h

#include "niftkMIDASExports.h"
#include "niftkTool.h"

namespace niftk
{

/**
 * \class MIDASPosnTool
 * \brief Dummy class, as the MIDAS posn tool, just enables you to change the
 * position of the slices in 2 or 3 windows, which is the default behaviour of
 * the MITK ortho-viewer anyway.
 */
class NIFTKMIDAS_EXPORT MIDASPosnTool : public MIDASTool
{

public:
  mitkClassMacro(MIDASPosnTool, MIDASTool);
  itkNewMacro(MIDASPosnTool);

  virtual const char* GetName() const;
  virtual const char** GetXPM() const;

protected:

  MIDASPosnTool(); // purposefully hidden
  virtual ~MIDASPosnTool(); // purposefully hidden

private:

};

}

#endif
