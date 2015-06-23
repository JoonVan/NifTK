/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef SurfaceReconView_h
#define SurfaceReconView_h

#include <QmitkBaseView.h>
#include <niftkSurfaceReconstruction.h>
#include <service/event/ctkEvent.h>
#include "ui_SurfaceReconViewWidget.h"
#include <QFuture>
#include <QFutureWatcher>


/**
 * \class SurfaceReconView
 * \brief User interface to provide a reconstructed surface from video images.
 * \ingroup uk_ac_ucl_cmic_igisurfacerecon_internal
*/
class SurfaceReconView : public QmitkBaseView, public Ui::SurfaceReconViewWidget
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  SurfaceReconView();
  virtual ~SurfaceReconView();

  /**
   * \brief Static view ID = uk.ac.ucl.cmic.igisurfacerecon
   */
  static const char* VIEW_ID;

  /**
   * \brief Returns the view ID.
   */

  virtual std::string GetViewID() const;

protected:

  /**
   *  \brief Called by framework, this method creates all the controls for this view
   */
  virtual void CreateQtPartControl(QWidget *parent);

  /**
   * \brief Called by framework, sets the focus on a specific widget.
   */
  virtual void SetFocus();

  // this should probably go into one of our modules, for easier testing and re-use!
  std::string IncrementNodeName(const std::string& name);

  /** Called via ctk-event bus when user starts an IGI data recording session. */
  void WriteCurrentConfig(const QString& directory) const;

protected slots:

  /**
   * \brief The main method to perform the surface reconstruction.
   */
  void DoSurfaceReconstruction();

private slots:
  
  /**
   * \brief We can listen to the event bus to trigger updates.
   */
  void OnUpdate(const ctkEvent& event);

  // we connect the future to this slot
  void OnBackgroundProcessFinished();

  /** Triggered by igidatasources plugin (and QmitkIGIDataSourceManager) to tell us that recording has started. */
  void OnRecordingStarted(const ctkEvent& event);

private:
  mitk::BaseData::Pointer RunBackgroundReconstruction(niftk::SurfaceReconstruction::ParamPacket param);

  /**
   * \brief Retrieve's the pref values from preference service, and stored in member variables.
   */
  void RetrievePreferenceValues();

  /**
   * \brief BlueBerry's notification about preference changes (e.g. from a preferences dialog).
   */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*);

  /**
   * \brief Delegate all functionality to this class, so we can unit test it outside of the plugin.
   */
  niftk::SurfaceReconstruction::Pointer      m_SurfaceReconstruction;


  QFuture<mitk::BaseData::Pointer>           m_BackgroundProcess;
  QFutureWatcher<mitk::BaseData::Pointer>    m_BackgroundProcessWatcher;
  std::string                                m_BackgroundOutputNodeName;
  std::string                                m_BackgroundLeftNodeName;
  std::string                                m_BackgroundRightNodeName;
  bool                                       m_BackgroundOutputNodeIsVisible;
  std::string                                m_BackgroundErrorMessage;

  // these are coming from the ctk event bus admin. we use them to explicitly unregister ourself.
  qlonglong           m_IGIUpdateSubscriptionID;
  qlonglong           m_IGIRecordingStartedSubscriptionID;
};

#endif // SurfaceReconView_h
