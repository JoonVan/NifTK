/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef PointRegView_h
#define PointRegView_h

#include <QmitkBaseView.h>
#include <service/event/ctkEvent.h>
#include "ui_PointRegView.h"
#include <vtkSmartPointer.h>

class vtkMatrix4x4;

/**
 * \class PointRegView
 * \brief User interface to provide controls point based registration.
 *
 * This class manages user interaction, but delegates the algorithm to
 * mitk::PointBasedRegistration.
 *
 * \ingroup uk_ac_ucl_cmic_igipointreg_internal
*/
class PointRegView : public QmitkBaseView
{  
  /**
   * this is needed for all Qt objects that should have a Qt meta-object
   * (everything that derives from QObject and wants to have signal/slots)
   */
  Q_OBJECT

public:

  PointRegView();
  virtual ~PointRegView();

  /**
   * \brief Static view ID = uk.ac.ucl.cmic.igipointreg
   */
  static const std::string VIEW_ID;

  /**
   * \brief Returns the view ID.
   */

  virtual std::string GetViewID() const;

protected:

  /**
   *  \brief Called by framework, this method creates all the controls for this view
   */
  virtual void CreateQtPartControl(QWidget *parent) override;

  /**
   * \brief Called by framework, sets the focus on a specific widget.
   */
  virtual void SetFocus() override;

protected slots:

protected:

private slots:
  
  void OnCalculateButtonPressed();
  void OnComposeWithDataButtonPressed();
  void OnSaveToFileButtonPressed();

private:

  /**
   * \brief Retrieve's the pref values from preference service, and stored in member variables.
   */
  void RetrievePreferenceValues();

  /**
   * \brief BlueBerry's notification about preference changes (e.g. from a preferences dialog).
   */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*) override;

  /**
   * \brief All the controls for the main view part.
   */
  Ui::PointRegView *m_Controls;
  vtkSmartPointer<vtkMatrix4x4> m_Matrix;
  bool m_UseICPInitialisation;
  bool m_UsePointIDToMatch;
};

#endif // PointRegView_h
