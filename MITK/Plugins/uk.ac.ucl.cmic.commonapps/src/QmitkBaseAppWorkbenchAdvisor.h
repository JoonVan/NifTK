/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkBaseAppWorkbenchAdvisor_h
#define QmitkBaseAppWorkbenchAdvisor_h

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <uk_ac_ucl_cmic_commonapps_Export.h>
#include <berryQtWorkbenchAdvisor.h>
#include <berryIWorkbenchConfigurer.h>

class QmitkBaseWorkbenchWindowAdvisor;

namespace mitk
{
class DataStorage;
}

/**
 * \class QmitkBaseAppWorkbenchAdvisor
 * \brief Abstract advisor class to set up an initial workbench.
 * \ingroup uk_ac_ucl_cmic_commonapps
 */
class CMIC_QT_COMMONAPPS QmitkBaseAppWorkbenchAdvisor: public berry::QtWorkbenchAdvisor
{
public:

  virtual void Initialize(berry::IWorkbenchConfigurer::Pointer configurer) override;

  /**
   * \brief Called by framework to create the WorkbenchWindowAdvisor,
   * and derived classes should instead override CreateQmitkBaseWorkbenchWindowAdvisor.
   */
  virtual berry::WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      berry::IWorkbenchWindowConfigurer::Pointer configurer) override;

  /**
   * \brief Derived classes should provide a default perspective id
   * corresponding to a valid perspective defined in their application.
   */
  virtual QString GetInitialWindowPerspectiveId() = 0;

  virtual void PostStartup();

  /**
   * Overriden from berry::WorkbenchAdvisor so that we can pop up a dialog box
   * asking the user whether they really want to close the application.
   * @returns true if closing is ok, false otherwise.
   */
  virtual bool PreShutdown() override;

protected:

  /**
   * \brief Derived classes should provide a window Icon resource path
   * corresponding to a valid icon file, described using a Qt resource location.
   */
  virtual QString GetWindowIconResourcePath() const = 0;

  /**
   * \brief Actually creates the derived WorkbenchWindowAdvisor.
   */
  virtual QmitkBaseWorkbenchWindowAdvisor* CreateQmitkBaseWorkbenchWindowAdvisor(
      berry::IWorkbenchWindowConfigurer::Pointer configurer);

  mitk::DataStorage* GetDataStorage();

};

#endif /*QMITKBASEAPPWORKBENCHADVISOR_H_*/
