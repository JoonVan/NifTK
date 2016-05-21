/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkMIDASPaintbrushToolEventInterface_h
#define niftkMIDASPaintbrushToolEventInterface_h

#include <itkObject.h>
#include <itkSmartPointer.h>
#include <itkObjectFactory.h>
#include <mitkOperationActor.h>

namespace niftk
{

class MIDASPaintbrushTool;

/**
 * \class MIDASPaintbrushToolEventInterface
 * \brief Interface class, simply to callback operations onto the MIDASPaintbrushTool.
 */
class MIDASPaintbrushToolEventInterface: public itk::Object, public mitk::OperationActor
{
public:
  typedef MIDASPaintbrushToolEventInterface  Self;
  typedef itk::SmartPointer<const Self>      ConstPointer;
  typedef itk::SmartPointer<Self>            Pointer;

  /// \brief Creates the object via the ITK object factory.
  itkNewMacro(Self);

  /// \brief Sets the tool to callback on to.
  void SetMIDASPaintbrushTool( MIDASPaintbrushTool* paintbrushTool );

  /// \brief Main execution function.
  virtual void  ExecuteOperation(mitk::Operation* op);

protected:
  MIDASPaintbrushToolEventInterface();
  ~MIDASPaintbrushToolEventInterface();
private:
  MIDASPaintbrushTool* m_MIDASPaintbrushTool;
};

}

#endif

