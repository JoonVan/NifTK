/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-09-08 16:23:32 +0100 (Thu, 08 Sep 2011) $
 Revision          : $Revision: 7267 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@cs.ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/
#ifndef ITKDRCANALYZEIMAGEIO3160_H
#define ITKDRCANALYZEIMAGEIO3160_H

#include "niftkMitkExtExports.h"
#include "itkAnalyzeImageIO3160.h"

namespace itk {

/**
 * \class DRCAnalyzeImageIO
 * \brief Subclass of AnalyzeImageIO3160, to read Dementia Research Centre (DRC) Analyze format,
 * which is incorrectly flipped. If you call this->SetDRCMode(true) (also default), it will do DRC specific
 * functionality, and if you call SetDRCMode(false), it will revert to standard ITK functionality.
 */
class NIFTKMITKEXT_EXPORT ITK_EXPORT DRCAnalyzeImageIO3160: public AnalyzeImageIO3160 {

public:

  /** Standard class typedefs. */
  typedef DRCAnalyzeImageIO3160 Self;
  typedef AnalyzeImageIO3160 Superclass;
  typedef SmartPointer<Self> Pointer;

  /** Method for creation through the object factory. */
  itkNewMacro( Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(DRCAnalyzeImageIO3160, Superclass);

  /** Returns true if we are using DRC mode, and false otherwise. Defaults to true. */
  bool GetDRCMode() const { return m_DRCMode; }

  /** Sets the DRC mode flag. Defaults to true. */
  void SetDRCMode(bool b) { m_DRCMode = b; }

  /** Overriden from base class. */
  virtual void ReadImageInformation();

protected:

  DRCAnalyzeImageIO3160();
  ~DRCAnalyzeImageIO3160();

  void PrintSelf(std::ostream& os, Indent indent) const;

  /** This should be protected in base class, but it isn't. */
  ImageIOBase::ByteOrder CheckAnalyzeEndian(const struct dsr &temphdr);

  /** This should be protected in base class, but it isn't. */
  void SwapHeaderBytesIfNecessary( ImageIOBase::ByteOrder& byteOrder, struct dsr * const imageheader );

  typedef enum
  {
    ITK_DRC_ANALYZE_ORIENTATION_RAI_AXIAL              = 0, /** Denotes an axial data orientation x=R->L, y=A->P, z=I->S */
    ITK_DRC_ANALYZE_ORIENTATION_RSP_CORONAL            = 1, /** Denotes a coronal data orientation, x=R->L, y=S->I, z=P->A */
    ITK_DRC_ANALYZE_ORIENTATION_ASL_SAGITTAL           = 2, /** Denotes a sagittal data orientation, x=A->P, y=S->I, z=L->R */
    ITK_DRC_ANALYZE_ORIENTATION_RPI_AXIAL_FLIPPED      = 3, /** THESE ARE NOT YET CORRECTLY HANDLED - No available test data. */
    ITK_DRC_ANALYZE_ORIENTATION_RIP_CORONAL_FLIPPED    = 4, /** THESE ARE NOT YET CORRECTLY HANDLED - No available test data. */
    ITK_DRC_ANALYZE_ORIENTATION_AIL_SAGITTAL_FLIPPED   = 5  /** THESE ARE NOT YET CORRECTLY HANDLED - No available test data. */
  } ValidDRCAnalyzeOrientationFlags;

private:

  DRCAnalyzeImageIO3160(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  /** Simple flag to determine if we are doing DRC mode. */
  bool m_DRCMode;

};

} // end namespace itk

#endif // ITKDRCANALYZEIMAGEIO_H
