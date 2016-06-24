/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef SurfaceRegView_h
#define SurfaceRegView_h

#include <QmitkBaseView.h>
#include "ui_SurfaceRegView.h"
#include <vtkSmartPointer.h>
#include <niftkICPBasedRegistration.h>
#include <QFuture>
#include <QFutureWatcher>

class vtkMatrix4x4;

/**
 * \class SurfaceRegView
 * \brief User interface to provide controls for surface based registration.
 *
 * This class manages user interaction, but delegates the algorithm to
 * niftk::ICPBasedRegistration.
 *
 * \ingroup uk_ac_ucl_cmic_igisurfacereg_internal
*/
class SurfaceRegView : public QmitkBaseView
{  
  /**
   * this is needed for all Qt objects that should have a Qt meta-object
   * (everything that derives from QObject and wants to have signal/slots)
   */
  Q_OBJECT

public:

  SurfaceRegView();
  virtual ~SurfaceRegView();

  /**
   * \brief Static view ID = uk.ac.ucl.cmic.igisurfacereg
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
  float ComputeDistance(vtkSmartPointer<vtkPolyData> fixed, vtkSmartPointer<vtkPolyData> moving);

private slots:

  void OnCalculateButtonPressed();
  void OnComposeWithDataButtonPressed();
  void OnSaveToFileButtonPressed();

  void DataStorageEventListener(const mitk::DataNode* node);

  void OnComputeDistance();
  void OnBackgroundProcessFinished();

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
  Ui::SurfaceRegView *m_Controls;
  vtkSmartPointer<vtkMatrix4x4> m_Matrix;

  int m_MaxIterations;
  int m_MaxPoints;
  unsigned int m_TLSITerations;
  unsigned int m_TLSPercentage;

  QFuture<float>           m_BackgroundProcess;
  QFutureWatcher<float>    m_BackgroundProcessWatcher;

};

#endif // SurfaceRegView_h
