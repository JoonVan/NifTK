/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/
 
#ifndef TrackedImageView_h
#define TrackedImageView_h

#include <niftkBaseView.h>

#include "ui_TrackedImageView.h"
#include <service/event/ctkEvent.h>
#include <vtkSmartPointer.h>

class vtkMatrix4x4;

/**
 * \class TrackedImageView
 * \brief Coordinates an image moving via a tracking transform.
 *
 * This is useful for displaying a tracked ultrasound plane in a 3D scene,
 * such as might be used for a free-hand ultrasound demo.
 *
 * \ingroup uk_ac_ucl_cmic_igitrackedimage_internal
*/
class TrackedImageView : public niftk::BaseView
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  TrackedImageView();
  virtual ~TrackedImageView();

  /**
   * \brief Static view ID = uk.ac.ucl.cmic.igitrackedimage
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

signals:

  /**
   * \brief We publish an update signal on topic "uk/ac/ucl/cmic/IGITRACKEDIMAGEUPDATE" onto the Event Bus so that any other plugin can listen.
   */
  void Updated(const ctkDictionary&);
  
protected slots:

  /**
   * \brief Creates copies of the image, and mappers to display them.
   */
  void OnClonePushButtonClicked();

protected:

private slots:
  
  /**
   * \brief We listen to the event bus to trigger updates.
   */
  void OnUpdate(const ctkEvent& event);

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
  Ui::TrackedImageView *m_Controls;

  /**
   * \brief Member variables for keeping state between button clicks.
   */
  vtkSmartPointer<vtkMatrix4x4>                m_ImageToTrackingSensorTransform;
  vtkSmartPointer<vtkMatrix4x4>                m_EmToOpticalMatrix;

  bool                                         m_ShowCloneImageGroup;
  int                                          m_NameCounter;
  bool                                         m_Show2DWindow;
};

#endif // TrackedImageView_h
