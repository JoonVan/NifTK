/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2011-11-18 09:05:48 +0000 (Fri, 18 Nov 2011) $
 Revision          : $Revision: 7804 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#ifndef QMITKNIFTYVIEWAPPLICATION_H_
#define QMITKNIFTYVIEWAPPLICATION_H_

#include "mitkQtNiftyViewAppDll.h"
#include <berryIApplication.h>

/**
 * \class QmitkNiftyViewApplication
 * \brief Plugin class to start up the NiftyView application.
 * \ingroup uk_ac_ucl_cmic_gui_qt_niftyview
 */
class CMIC_QT_NIFTYVIEWAPP QmitkNiftyViewApplication : public QObject, public berry::IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  QmitkNiftyViewApplication();
  QmitkNiftyViewApplication(const QmitkNiftyViewApplication& other);
  
  int Start();
  void Stop();
};

#endif /*QMITKNIFTYVIEWAPPLICATION_H_*/
