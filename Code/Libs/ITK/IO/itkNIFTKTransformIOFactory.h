/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef itkNIFTKTransformIOFactory_h
#define itkNIFTKTransformIOFactory_h

#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#include <itkObjectFactoryBase.h>
#include <itkTransformIOBase.h>
#include <niftkITKWin32ExportHeader.h>

namespace itk
{
/** \class NIFTKTransformIOFactory
   * \brief Create instances of UCLTransformIO objects using an object factory.
   */
class NIFTKITK_WINEXPORT ITK_EXPORT NIFTKTransformIOFactory : public ObjectFactoryBase
{
public:
  /** Standard class typedefs. */
  typedef NIFTKTransformIOFactory    Self;
  typedef ObjectFactoryBase        Superclass;
  typedef SmartPointer<Self>       Pointer;
  typedef SmartPointer<const Self> ConstPointer;

  /** Class methods used to interface with the registered factories. */
  virtual const char* GetITKSourceVersion(void) const;
  virtual const char* GetDescription(void) const;

  /** Method for class instantiation. */
  itkFactorylessNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(NIFTKTransformIOFactory, ObjectFactoryBase);

protected:
  NIFTKTransformIOFactory();
  ~NIFTKTransformIOFactory();
  virtual void PrintSelf(std::ostream& os, Indent indent) const;

private:
  NIFTKTransformIOFactory(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


} // end namespace itk

#endif
