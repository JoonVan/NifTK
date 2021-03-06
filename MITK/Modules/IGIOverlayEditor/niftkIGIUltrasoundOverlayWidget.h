/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef niftkIGIUltrasoundOverlayWidget_h
#define niftkIGIUltrasoundOverlayWidget_h

#include "ui_niftkIGIUltrasoundOverlayWidget.h"
#include "niftkIGIOverlayEditorExports.h"
#include <QWidget>
#include <mitkColorProperty.h>
#include <mitkDataStorage.h>

class QmitkRenderWindow;

namespace niftk
{
/**
 * \class IGIUltrasoundOverlayWidget
 * \brief A widget that contains a niftk::SingleVideoWidget and a QmitkRenderWindow,
 * (all set to render 3D mode), and several widgets for some basic controls.
 */
class NIFTKIGIOVERLAYEDITOR_EXPORT IGIUltrasoundOverlayWidget : public QWidget,
    public Ui_IGIUltrasoundOverlayWidget
{

  Q_OBJECT

public:

  IGIUltrasoundOverlayWidget(QWidget *parent);
  virtual ~IGIUltrasoundOverlayWidget();

  /**
   * \brief Currently returns the QmitkRenderWindow from the QmitkSingle3DView.
   */
  QmitkRenderWindow* GetActiveQmitkRenderWindow() const;

  /**
   * \brief Returns the niftk::SingleVideoWidget's QmitkRenderWindow with identifier="overlay",
   * and the 3D QmitkRenderWindow with identifier="3d".
   */
  QHash<QString, QmitkRenderWindow *> GetQmitkRenderWindows() const;

  /**
   * \brief Returns the QmitkRenderWindow corresponding to the parameter id.
   * \param id identifer, which for this class must be either "overlay" or "3d".
   * \return QmitkRenderWindow* or NULL if the id does not match.
   */
  QmitkRenderWindow* GetQmitkRenderWindow(const QString &id) const;

  /**
   * \brief Set the full path for the department logo.
   */
  void SetDepartmentLogoPath(const QString& path);

  /**
   * \brief Calls niftk::Single3DViewWidget::EnableDepartmentLogo().
   */
  void EnableDepartmentLogo();

  /**
   * \brief Calls Single3DViewWidget::DisableDepartmentLogo().
   */
  void DisableDepartmentLogo();

  /**
   * \brief Calls Single3DViewWidget::SetGradientBackgroundColors().
   */
  void SetGradientBackgroundColors(const mitk::Color& colour1, const mitk::Color& colour2);

  /**
   * \brief Calls Single3DViewWidget::EnableGradientBackground().
   */
  void EnableGradientBackground();

  /**
   * \brief Calls Single3DViewWidget::DisableGradientBackground().
   */
  void DisableGradientBackground();

  /**
   * \brief Sets the data storage on various widgets, e.g. viewers, combo-boxes.
   */
  void SetDataStorage(mitk::DataStorage* storage);

  /**
   * \brief Called by framework (event from ctkEventAdmin), to indicate that an update should be performed.
   */
  void Update();

  /**
   * \brief Turns on/off clipping before/after the image plane.
   */
  void SetClipToImagePlane(const bool& clipToImagePlane);

private slots:

  void OnLeftOverlayCheckBoxChecked(bool);
  void On3DViewerCheckBoxChecked(bool);
  void OnLeftImageSelected(const mitk::DataNode* node);

private:

  IGIUltrasoundOverlayWidget(const IGIUltrasoundOverlayWidget&);  // Purposefully not implemented.
  void operator=(const IGIUltrasoundOverlayWidget&);  // Purposefully not implemented.

  mitk::DataStorage::Pointer m_DataStorage;
};

} // end namespace

#endif
