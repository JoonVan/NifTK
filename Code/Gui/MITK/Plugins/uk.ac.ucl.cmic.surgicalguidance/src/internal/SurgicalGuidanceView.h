/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef SurgicalGuidanceView_h
#define SurgicalGuidanceView_h

#include "QmitkBaseLegacyView.h"
#include "QmitkIGIDataSourceManager.h"
#include <berryIBerryPreferences.h>

/**
 * \class SurgicalGuidanceView
 * \brief User interface to provide Image Guided Surgery functionality.
 * \ingroup uk_ac_ucl_cmic_surgicalguidance_internal
*/
class SurgicalGuidanceView : public QmitkBaseLegacyView
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  SurgicalGuidanceView();
  virtual ~SurgicalGuidanceView();

  /// \brief Static view ID = uk.ac.ucl.cmic.surgicalguidance
  static const std::string VIEW_ID;

  /// \brief Returns the view ID.
  virtual std::string GetViewID() const;

protected:

  /// \brief Called by framework, this method creates all the controls for this view
  virtual void CreateQtPartControl(QWidget *parent);

  /// \brief Called by framework, sets the focus on a specific widget.
  virtual void SetFocus();

protected slots:

protected:

private slots:
  
private:

  /// \brief Retrieve's the pref values from preference service, and stored in member variables.
  void RetrievePreferenceValues();

  /// \brief BlueBerry's notification about preference changes (e.g. from a preferences dialog).
  virtual void OnPreferencesChanged(const berry::IBerryPreferences*);

  QmitkIGIDataSourceManager::Pointer  m_DataSourceManager;
};

#endif // SurgicalGuidanceView_h
