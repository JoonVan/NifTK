/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date$
 Revision          : $Revision$
 Last modified by  : $Author$

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef _MIDASMORPHOLOGICALSEGMENTORPIPELINEINTERFACE_H_INCLUDED
#define _MIDASMORPHOLOGICALSEGMENTORPIPELINEINTERFACE_H_INCLUDED

#include "MorphologicalSegmentorPipelineParams.h"

/**
 * \class MorphologicalSegmentorPipelineInterface
 * \brief Abstract interface to plug ITK pipeline into MITK framework to represent the MIDAS Morphological Segmentor Pipeline.
 *
 * \ingroup midas_morph_editor
 */
class MorphologicalSegmentorPipelineInterface
{
public:

  /// \brief Default no-op constructor.
  MorphologicalSegmentorPipelineInterface() {};

  /// \brief Default no-op destructor.
  ~MorphologicalSegmentorPipelineInterface() {};

  /// \brief Pass all parameters to pipeline.
  virtual void SetParam(MorphologicalSegmentorPipelineParams& p) = 0;

  /// \brief Update the pipeline.
  ///
  /// \param editingFlags array of 4 booleans to say which images are being editted.
  /// \param editingRegion pass in an array of 6 integers containing size[0-2], and index[3-5] for the region being edited.
  virtual void Update(std::vector<bool>& editingFlags, std::vector<int>& editingRegion) = 0;
};

#endif
