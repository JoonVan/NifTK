/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "IGIOverlayEditor.h"

#include <berryUIException.h>
#include <berryIWorkbenchPage.h>
#include <berryIPreferencesService.h>
#include <berryIPartListener.h>
#include <ctkPluginContext.h>
#include <ctkServiceReference.h>
#include <service/event/ctkEventConstants.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>

#include <QWidget>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QDir>

#include <mitkColorProperty.h>
#include <mitkGlobalInteraction.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

#include <mitkDataStorageEditorInput.h>
#include <mitkIDataStorageService.h>

#include <QmitkIGIOverlayEditor.h>
#include <internal/IGIOverlayEditorPreferencePage.h>
#include <internal/IGIOverlayEditorActivator.h>

const char* IGIOverlayEditor::EDITOR_ID = "org.mitk.editors.igioverlayeditor";


/**
 * \class IGIOverlayEditorPrivate
 * \brief PIMPL pattern implementation of IGIOverlayEditor.
 */
class IGIOverlayEditorPrivate
{
public:

  IGIOverlayEditorPrivate();
  ~IGIOverlayEditorPrivate();

  QmitkIGIOverlayEditor* m_IGIOverlayEditor;
  std::string m_FirstBackgroundColor;
  std::string m_SecondBackgroundColor;
  berry::IPartListener::Pointer m_PartListener;
};


/**
 * \class IGIOverlayWidgetPartListener
 * \brief Used to handle interaction with the contained overlay
 * editor widget when this IGIOverlayEditor is opened/closed etc.
 */
struct IGIOverlayWidgetPartListener : public berry::IPartListener
{
  berryObjectMacro(IGIOverlayWidgetPartListener)

  //---------------------------------------------------------------------------
  IGIOverlayWidgetPartListener(IGIOverlayEditorPrivate* dd)
    : d(dd)
  {}

  //---------------------------------------------------------------------------
  Events::Types GetPartEventTypes() const
  {
    return Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
  }

  //---------------------------------------------------------------------------
  void PartClosed (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == IGIOverlayEditor::EDITOR_ID)
    {
      IGIOverlayEditor::Pointer editor = partRef->GetPart(false).Cast<IGIOverlayEditor>();
      if (d->m_IGIOverlayEditor == editor->GetIGIOverlayEditor())
      {
        // Call editor to turn things off as the widget is being closed.
      }
    }
  }

  //---------------------------------------------------------------------------
  void PartHidden (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == IGIOverlayEditor::EDITOR_ID)
    {
      IGIOverlayEditor::Pointer editor = partRef->GetPart(false).Cast<IGIOverlayEditor>();
      if (d->m_IGIOverlayEditor == editor->GetIGIOverlayEditor())
      {
        // Call editor to turn things off as the widget is being hidden.
      }
    }
  }

  //---------------------------------------------------------------------------
  void PartVisible (berry::IWorkbenchPartReference::Pointer partRef)
  {
    if (partRef->GetId() == IGIOverlayEditor::EDITOR_ID)
    {
      IGIOverlayEditor::Pointer editor = partRef->GetPart(false).Cast<IGIOverlayEditor>();
      if (d->m_IGIOverlayEditor == editor->GetIGIOverlayEditor())
      {
        // Call editor to turn things on as the widget is being made visible.
      }
    }
  }

private:

  IGIOverlayEditorPrivate* const d;

};


//-----------------------------------------------------------------------------
IGIOverlayEditorPrivate::IGIOverlayEditorPrivate()
  : m_IGIOverlayEditor(0)
  , m_PartListener(new IGIOverlayWidgetPartListener(this))
{}


//-----------------------------------------------------------------------------
IGIOverlayEditorPrivate::~IGIOverlayEditorPrivate()
{
}

//-----------------------------------------------------------------------------
IGIOverlayEditor::IGIOverlayEditor()
  : d(new IGIOverlayEditorPrivate)
{
}


//-----------------------------------------------------------------------------
IGIOverlayEditor::~IGIOverlayEditor()
{
  this->GetSite()->GetPage()->RemovePartListener(d->m_PartListener);
}


//-----------------------------------------------------------------------------
QmitkIGIOverlayEditor* IGIOverlayEditor::GetIGIOverlayEditor()
{
  return d->m_IGIOverlayEditor;
}


//-----------------------------------------------------------------------------
QmitkRenderWindow *IGIOverlayEditor::GetActiveQmitkRenderWindow() const
{
  return d->m_IGIOverlayEditor->GetActiveQmitkRenderWindow();
}


//-----------------------------------------------------------------------------
QHash<QString, QmitkRenderWindow *> IGIOverlayEditor::GetQmitkRenderWindows() const
{
  return d->m_IGIOverlayEditor->GetQmitkRenderWindows();
}


//-----------------------------------------------------------------------------
QmitkRenderWindow *IGIOverlayEditor::GetQmitkRenderWindow(const QString &id) const
{
  return d->m_IGIOverlayEditor->GetQmitkRenderWindow(id);
}


//-----------------------------------------------------------------------------
mitk::Point3D IGIOverlayEditor::GetSelectedPosition(const QString & id) const
{
  // Not implemented.
  mitk::Point3D point;
  point[0] = 0;
  point[1] = 0;
  point[2] = 0;
  return point;
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::SetSelectedPosition(const mitk::Point3D &pos, const QString &id)
{
  // Not implemented.
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::EnableDecorations(bool /*enable*/, const QStringList & /*decorations*/)
{
}


//-----------------------------------------------------------------------------
bool IGIOverlayEditor::IsDecorationEnabled(const QString & /*decoration*/) const
{
  return false;
}


//-----------------------------------------------------------------------------
QStringList IGIOverlayEditor::GetDecorations() const
{
  QStringList decorations;
  return decorations;
}


//-----------------------------------------------------------------------------
mitk::SlicesRotator* IGIOverlayEditor::GetSlicesRotator() const
{
  return NULL;
}


//-----------------------------------------------------------------------------
mitk::SlicesSwiveller* IGIOverlayEditor::GetSlicesSwiveller() const
{
  return NULL;
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::EnableSlicingPlanes(bool /*enable*/)
{
}


//-----------------------------------------------------------------------------
bool IGIOverlayEditor::IsSlicingPlanesEnabled() const
{
  return false;
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::EnableLinkedNavigation(bool /*enable*/)
{
}


//-----------------------------------------------------------------------------
bool IGIOverlayEditor::IsLinkedNavigationEnabled() const
{
  return false;
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::CreateQtPartControl(QWidget* parent)
{
  if (d->m_IGIOverlayEditor == 0)
  {
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0,0,0,0);

    d->m_IGIOverlayEditor = new QmitkIGIOverlayEditor(parent);
    layout->addWidget(d->m_IGIOverlayEditor);

    mitk::DataStorage::Pointer ds = this->GetDataStorage();
    d->m_IGIOverlayEditor->SetDataStorage(ds);

    this->GetSite()->GetPage()->AddPartListener(d->m_PartListener);

    QMetaObject::invokeMethod(this, "OnPreferencesChanged", Qt::QueuedConnection);

    this->RequestUpdate();

    // Finally: Listen to update pulse coming off of event bus. This pulse comes from the data manager updating.
    ctkServiceReference ref = mitk::IGIOverlayEditorActivator::getContext()->getServiceReference<ctkEventAdmin>();
    if (ref)
    {
      ctkEventAdmin* eventAdmin = mitk::IGIOverlayEditorActivator::getContext()->getService<ctkEventAdmin>(ref);
      
      ctkDictionary propertiesIGI;
      propertiesIGI[ctkEventConstants::EVENT_TOPIC] = "uk/ac/ucl/cmic/IGIUPDATE";
      eventAdmin->subscribeSlot(this, SLOT(OnIGIUpdate(ctkEvent)), propertiesIGI);
      
      ctkDictionary propertiesTrackedImage;
      propertiesTrackedImage[ctkEventConstants::EVENT_TOPIC] = "uk/ac/ucl/cmic/IGITRACKEDIMAGEUPDATE";
      eventAdmin->subscribeSlot(this, SLOT(OnTrackedImageUpdate(ctkEvent)), propertiesTrackedImage, Qt::DirectConnection);

      ctkDictionary propertiesRecordingStarted;
      propertiesRecordingStarted[ctkEventConstants::EVENT_TOPIC] = "uk/ac/ucl/cmic/IGIRECORDINGSTARTED";
      eventAdmin->subscribeSlot(this, SLOT(OnRecordingStarted(ctkEvent)), propertiesRecordingStarted);
    }
  }
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::OnPreferencesChanged()
{
  this->OnPreferencesChanged(dynamic_cast<berry::IBerryPreferences*>(this->GetPreferences().GetPointer()));
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  // Enable change of logo. If no DepartmentLogo was set explicitly, MBILogo is used.
  // Set new department logo by prefs->Set("DepartmentLogo", "PathToImage");

  std::vector<std::string> keys = prefs->Keys();
  
  for( unsigned int i = 0; i < keys.size(); ++i )
  {
    if( keys[i] == "DepartmentLogo")
    {
      std::string departmentLogoLocation = prefs->Get("DepartmentLogo", "");

      if (departmentLogoLocation.empty())
      {
        d->m_IGIOverlayEditor->DisableDepartmentLogo();
      }
      else
      {
        d->m_IGIOverlayEditor->SetDepartmentLogoPath(departmentLogoLocation);
        d->m_IGIOverlayEditor->EnableDepartmentLogo();
      }
      break;
    }
  }
 
  // Preferences for gradient background
  float color = 255.0;
  QString firstColorName = QString::fromStdString (prefs->GetByteArray(IGIOverlayEditorPreferencePage::FIRST_BACKGROUND_COLOUR, ""));
  QColor firstColor(firstColorName);
  mitk::Color upper;
  if (firstColorName=="") // default values
  {
    upper[0] = 0;
    upper[1] = 0;
    upper[2] = 0;
  }
  else
  {
    upper[0] = firstColor.red() / color;
    upper[1] = firstColor.green() / color;
    upper[2] = firstColor.blue() / color;
  }

  QString secondColorName = QString::fromStdString (prefs->GetByteArray(IGIOverlayEditorPreferencePage::SECOND_BACKGROUND_COLOUR, ""));
  QColor secondColor(secondColorName);
  mitk::Color lower;
  if (secondColorName=="") // default values
  {
    lower[0] = 0;
    lower[1] = 0;
    lower[2] = 0;
  }
  else
  {
    lower[0] = secondColor.red() / color;
    lower[1] = secondColor.green() / color;
    lower[2] = secondColor.blue() / color;
  }
  d->m_IGIOverlayEditor->SetGradientBackgroundColors(upper, lower);
  d->m_IGIOverlayEditor->EnableGradientBackground();

  std::string calibrationFileName = prefs->Get(IGIOverlayEditorPreferencePage::CALIBRATION_FILE_NAME, "");
  d->m_IGIOverlayEditor->SetCalibrationFileName(calibrationFileName);
  d->m_IGIOverlayEditor->SetCameraTrackingMode(prefs->GetBool(IGIOverlayEditorPreferencePage::CAMERA_TRACKING_MODE, true));
  d->m_IGIOverlayEditor->SetClipToImagePlane(prefs->GetBool(IGIOverlayEditorPreferencePage::CLIP_TO_IMAGE_PLANE, true));
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::SetFocus()
{
  if (d->m_IGIOverlayEditor != 0)
  {
    d->m_IGIOverlayEditor->setFocus();
  }
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::OnIGIUpdate(const ctkEvent& event)
{
  d->m_IGIOverlayEditor->Update();
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::OnTrackedImageUpdate(const ctkEvent& event)
{
  d->m_IGIOverlayEditor->Update();
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::WriteCurrentConfig(const QString& directory) const
{
  QFile   infoFile(directory + QDir::separator() + EDITOR_ID + ".txt");
  bool opened = infoFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append);
  if (opened)
  {
    QTextStream   info(&infoFile);
    info.setCodec("UTF-8");
    info << "START: " << QDateTime::currentDateTime().toString() << "\n";
    info << "calibfile=" << QString::fromStdString(d->m_IGIOverlayEditor->GetCalibrationFileName()) << "\n";
  }
}


//-----------------------------------------------------------------------------
void IGIOverlayEditor::OnRecordingStarted(const ctkEvent& event)
{
  QString   directory = event.getProperty("directory").toString();
  if (!directory.isEmpty())
  {
    try
    {
      WriteCurrentConfig(directory);
    }
    catch (...)
    {
      MITK_ERROR << "Caught exception while writing info file! Ignoring it and aborting info file.";
    }
  }
  else
  {
    MITK_WARN << "Received igi-recording-started event without directory information! Ignoring it.";
  }
}
