/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkNiftyMIDASApplication_h
#define QmitkNiftyMIDASApplication_h

#include <uk_ac_ucl_cmic_niftymidas_Export.h>
#include <QmitkBaseApplication.h>

/**
 * \class QmitkNiftyMIDASApplication
 * \brief Plugin class to start up the NiftyMIDAS application.
 * \ingroup uk_ac_ucl_cmic_niftymidas
 */
class NIFTYMIDAS_EXPORT QmitkNiftyMIDASApplication : public QmitkBaseApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkNiftyMIDASApplication();
  QmitkNiftyMIDASApplication(const QmitkNiftyMIDASApplication& other);

protected:

  /// \brief Derived classes override this to provide a workbench advisor.
  virtual berry::WorkbenchAdvisor* GetWorkbenchAdvisor();

};

#endif /*QMITKNIFTYMIDASAPPLICATION_H_*/
