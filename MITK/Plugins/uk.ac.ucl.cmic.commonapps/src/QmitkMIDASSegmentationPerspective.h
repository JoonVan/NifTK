/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkMIDASSegmentationPerspective_h
#define QmitkMIDASSegmentationPerspective_h

#include <uk_ac_ucl_cmic_commonapps_Export.h>
#include <berryIPerspectiveFactory.h>

/**
 * \class QmitkMIDASSegmentationPerspective
 * \brief Perspective to arrange widgets as would be suitable for MIDAS applications,
 * where the standard view has up to 5x5 independent windows.
 * \ingroup uk_ac_ucl_cmic_commonapps_internal
 *
 * Note: We have to load at least one view component, to get an editor created.
 */
class CMIC_QT_COMMONAPPS QmitkMIDASSegmentationPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)
  
public:

  QmitkMIDASSegmentationPerspective();
  QmitkMIDASSegmentationPerspective(const QmitkMIDASSegmentationPerspective& other);
  
  void CreateInitialLayout(berry::IPageLayout::Pointer layout) override;

};

#endif
