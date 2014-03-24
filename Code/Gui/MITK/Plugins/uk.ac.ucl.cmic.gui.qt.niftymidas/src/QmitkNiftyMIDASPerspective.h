/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkNiftyMIDASPerspective_h
#define QmitkNiftyMIDASPerspective_h

#include <uk_ac_ucl_cmic_gui_qt_niftymidas_Export.h>
#include <berryIPerspectiveFactory.h>

/**
 * \class QmitkNiftyMIDASPerspective
 * \brief Perspective to arrange widgets as would be suitable for MIDAS applications,
 * where the standard  has up to 5x5 independent windows.
 * \ingroup uk_ac_ucl_cmic_gui_qt_nifty_internal
 *
 * Note: We have to load at least one  component, to get an editor created.
 */
class CMIC_QT_NIFTYMIDASAPP QmitkNiftyMIDASPerspective : public QObject, public berry::IPerspectiveFactory
{
  Q_OBJECT
  Q_INTERFACES(berry::IPerspectiveFactory)

public:

  QmitkNiftyMIDASPerspective();
  QmitkNiftyMIDASPerspective(const QmitkNiftyMIDASPerspective& other);

  void CreateInitialLayout(berry::IPageLayout::Pointer layout);

};

#endif
