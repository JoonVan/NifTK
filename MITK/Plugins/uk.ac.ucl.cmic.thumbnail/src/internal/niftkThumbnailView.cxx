/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkThumbnailView.h"

#include <berryIPreferencesService.h>
#include <berryIBerryPreferences.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>
#include <mitkDataStorage.h>
#include <mitkFocusManager.h>
#include <mitkGlobalInteraction.h>
#include <mitkInteractionEventObserver.h>

#include <niftkThumbnailRenderWindow.h>

#include "niftkPluginActivator.h"
#include "niftkThumbnailViewPreferencePage.h"

#include <QHBoxLayout>


namespace niftk
{

const QString ThumbnailView::VIEW_ID = "uk.ac.ucl.cmic.thumbnail";

class EditorLifeCycleListener : public berry::IPartListener
{
  berryObjectMacro(EditorLifeCycleListener)

public:

  EditorLifeCycleListener(ThumbnailView* thumbnailView)
  : m_ThumbnailView(thumbnailView)
  {
  }

private:

  std::map<mitk::InteractionEventObserver*, bool> m_InteractorWasEnabled;

  Events::Types GetPartEventTypes() const override
  {
    return Events::VISIBLE | Events::HIDDEN | Events::ACTIVATED | Events::DEACTIVATED;
  }

  void PartActivated(const berry::IWorkbenchPartReference::Pointer& partRef) override
  {
    berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();
    if (part == m_ThumbnailView)
    {
      m_InteractorWasEnabled.clear();

      ctkPluginContext* context = PluginActivator::GetInstance()->GetContext();
      QList<ctkServiceReference> interactorRefs = context->getServiceReferences<mitk::InteractionEventObserver>();
      for (ctkServiceReference interactorRef: interactorRefs)
      {
        mitk::InteractionEventObserver* interactor = context->getService<mitk::InteractionEventObserver>(interactorRef);
        if (dynamic_cast<ThumbnailInteractor*>(interactor))
        {
          interactor->Enable();
        }
        else if (interactor->IsEnabled())
        {
          m_InteractorWasEnabled[interactor] = true;
          interactor->Disable();
        }
      }
    }
  }

  void PartDeactivated(const berry::IWorkbenchPartReference::Pointer& partRef) override
  {
    berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();
    if (part == m_ThumbnailView)
    {
      ctkPluginContext* context = PluginActivator::GetInstance()->GetContext();
      QList<ctkServiceReference> interactorRefs = context->getServiceReferences<mitk::InteractionEventObserver>();
      for (ctkServiceReference interactorRef: interactorRefs)
      {
        mitk::InteractionEventObserver* interactor = context->getService<mitk::InteractionEventObserver>(interactorRef);
        if (dynamic_cast<ThumbnailInteractor*>(interactor))
        {
          interactor->Disable();
        }
        else if (m_InteractorWasEnabled[interactor])
        {
          interactor->Enable();
        }
      }

      m_InteractorWasEnabled.clear();
    }
  }

  void PartVisible(const berry::IWorkbenchPartReference::Pointer& partRef) override
  {
    berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

    if (mitk::IRenderWindowPart* renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
    {
      mitk::BaseRenderer* focusedRenderer = mitk::GlobalInteraction::GetInstance()->GetFocus();

      bool found = false;
      /// Note:
      /// We need to look for the focused window among every window of the editor.
      /// The MITK Display has not got the concept of 'selected' window and always
      /// returns the axial window as 'active'. Therefore we cannot use GetActiveQmitkRenderWindow.
      foreach (QmitkRenderWindow* mainWindow, renderWindowPart->GetQmitkRenderWindows().values())
      {
        if (focusedRenderer == mainWindow->GetRenderer())
        {
          m_ThumbnailView->SetTrackedRenderer(focusedRenderer);
          found = true;
          break;
        }
      }

      if (!found)
      {
        QmitkRenderWindow* mainWindow = renderWindowPart->GetActiveQmitkRenderWindow();
        if (mainWindow && mainWindow->isVisible())
        {
          m_ThumbnailView->SetTrackedRenderer(mainWindow->GetRenderer());
        }
      }
    }
  }

  void PartHidden(const berry::IWorkbenchPartReference::Pointer& partRef) override
  {
    berry::IWorkbenchPart* part = partRef->GetPart(false).GetPointer();

    if (mitk::IRenderWindowPart* renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part))
    {
      const mitk::BaseRenderer* trackedRenderer = m_ThumbnailView->GetTrackedRenderer();

      /// Note:
      /// We need to look for the tracked window among every window of the editor.
      /// The MITK Display has not got the concept of 'selected' window and always
      /// returns the axial window as 'active'. Therefore we cannot use GetActiveQmitkRenderWindow.
      foreach (QmitkRenderWindow* mainWindow, renderWindowPart->GetQmitkRenderWindows().values())
      {
        if (mainWindow->GetRenderer() == trackedRenderer)
        {
          m_ThumbnailView->SetTrackedRenderer(0);
          return;
        }
      }
    }
  }

  ThumbnailView* m_ThumbnailView;

};


//-----------------------------------------------------------------------------
ThumbnailView::ThumbnailView()
: m_FocusManagerObserverTag(-1)
, m_ThumbnailWindow(0)
, m_TrackOnlyMainWindows(true)
, m_EditorLifeCycleListener(new EditorLifeCycleListener(this))
{
  m_RenderingManager = mitk::RenderingManager::New();
  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();
  m_RenderingManager->SetDataStorage(dataStorage);
}


//-----------------------------------------------------------------------------
ThumbnailView::~ThumbnailView()
{
  this->GetSite()->GetPage()->RemovePartListener(m_EditorLifeCycleListener.data());

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  if (focusManager != NULL)
  {
    focusManager->RemoveObserver(m_FocusManagerObserverTag);
    m_FocusManagerObserverTag = -1;
  }

  if (m_ThumbnailWindow)
  {
    delete m_ThumbnailWindow;
  }
}


//-----------------------------------------------------------------------------
void ThumbnailView::CreateQtPartControl(QWidget* parent)
{
  if (!m_ThumbnailWindow)
  {
    m_ThumbnailWindow = new ThumbnailRenderWindow(parent, m_RenderingManager);
    QHBoxLayout* layout = new QHBoxLayout(parent);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_ThumbnailWindow);
    parent->setLayout(layout);

    this->RetrievePreferenceValues();

    mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
    if (focusManager != NULL)
    {
      itk::SimpleMemberCommand<ThumbnailView>::Pointer onFocusChangedCommand =
        itk::SimpleMemberCommand<ThumbnailView>::New();
      onFocusChangedCommand->SetCallbackFunction( this, &ThumbnailView::OnFocusChanged );

      m_FocusManagerObserverTag = focusManager->AddObserver(mitk::FocusEvent(), onFocusChangedCommand);
    }

    this->GetSite()->GetPage()->AddPartListener(m_EditorLifeCycleListener.data());

    mitk::IRenderWindowPart* selectedEditor = this->GetSelectedEditor();
    if (selectedEditor)
    {
      bool found = false;

      mitk::BaseRenderer* focusedRenderer = focusManager->GetFocused();

      /// Note:
      /// We need to look for the focused window among every window of the editor.
      /// The MITK Display has not got the concept of 'selected' window and always
      /// returns the axial window as 'active'. Therefore we cannot use GetActiveQmitkRenderWindow.
      foreach (QmitkRenderWindow* mainWindow, selectedEditor->GetQmitkRenderWindows().values())
      {
        if (focusedRenderer == mainWindow->GetRenderer())
        {
          this->SetTrackedRenderer(focusedRenderer);
          found = true;
          break;
        }
      }

      if (!found)
      {
        QmitkRenderWindow* mainWindow = selectedEditor->GetActiveQmitkRenderWindow();
        if (mainWindow && mainWindow->isVisible())
        {
          this->SetTrackedRenderer(mainWindow->GetRenderer());
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
void ThumbnailView::SetFocus()
{
  m_ThumbnailWindow->setFocus();
}


//-----------------------------------------------------------------------------
void ThumbnailView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
  this->RetrievePreferenceValues();
}


//-----------------------------------------------------------------------------
void ThumbnailView::RetrievePreferenceValues()
{
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  assert( prefService );

  berry::IPreferences::Pointer prefs = prefService->GetSystemPreferences()->Node("/uk.ac.ucl.cmic.thumbnail");

  assert( prefs );

  int thickness = prefs->GetInt(ThumbnailViewPreferencePage::THUMBNAIL_BOX_THICKNESS, 1);
  int layer = prefs->GetInt(ThumbnailViewPreferencePage::THUMBNAIL_BOX_LAYER, 99);
  double opacity = prefs->GetDouble(ThumbnailViewPreferencePage::THUMBNAIL_BOX_OPACITY, 1);

  m_ThumbnailWindow->SetBoundingBoxLineThickness(thickness);
  m_ThumbnailWindow->SetBoundingBoxOpacity(opacity);
  m_ThumbnailWindow->SetBoundingBoxLayer(layer);

  bool onlyMainWindowsWereTracked = m_TrackOnlyMainWindows;
  m_TrackOnlyMainWindows = prefs->GetBool(ThumbnailViewPreferencePage::THUMBNAIL_TRACK_ONLY_MAIN_WINDOWS, true);

  if (m_TrackOnlyMainWindows != onlyMainWindowsWereTracked)
  {
    if (m_TrackOnlyMainWindows)
    {
      mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
      if (renderWindowPart)
      {
        QmitkRenderWindow* mainWindow = renderWindowPart->GetActiveQmitkRenderWindow();
        if (mainWindow && mainWindow->GetRenderer()->GetMapperID() == mitk::BaseRenderer::Standard2D)
        {
          m_ThumbnailWindow->SetTrackedRenderer(mainWindow->GetRenderer());
        }
      }
    }
    else
    {
      mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
      if (focusManager)
      {
        mitk::BaseRenderer::Pointer focusedRenderer = focusManager->GetFocused();
        if (focusedRenderer != m_ThumbnailWindow->GetRenderer()
            && focusedRenderer.IsNotNull()
            && focusedRenderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
        {
          m_ThumbnailWindow->SetTrackedRenderer(focusedRenderer);
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
void ThumbnailView::OnFocusChanged()
{
  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  if (!focusManager)
  {
    return;
  }

  mitk::BaseRenderer::Pointer focusedRenderer = focusManager->GetFocused();
  if (focusedRenderer == m_ThumbnailWindow->GetRenderer()
      || focusedRenderer.IsNull()
      || focusedRenderer->GetMapperID() != mitk::BaseRenderer::Standard2D)
  {
    return;
  }

  if (m_TrackOnlyMainWindows)
  {
    /// Track only render windows of the main display (aka. editor).
    mitk::IRenderWindowPart* renderWindowPart = this->GetRenderWindowPart();
    if (!renderWindowPart)
    {
      return;
    }

    /// Note:
    /// In the MITK display the active window is always the axial, therefore it is not
    /// enough to check if the focused renderer is that of the active window, but we have
    /// to go through all the renderers to check if the focused renderer is among them.
    bool found = false;
    foreach (QmitkRenderWindow* mainWindow, renderWindowPart->GetQmitkRenderWindows().values())
    {
      if (mainWindow->GetRenderer() == focusedRenderer)
      {
        found = true;
      }
    }

    if (!found)
    {
      return;
    }
  }

  this->SetTrackedRenderer(focusedRenderer);
}


//-----------------------------------------------------------------------------
mitk::BaseRenderer* ThumbnailView::GetTrackedRenderer() const
{
  return m_ThumbnailWindow->GetTrackedRenderer();
}


//-----------------------------------------------------------------------------
void ThumbnailView::SetTrackedRenderer(mitk::BaseRenderer* renderer)
{
  m_ThumbnailWindow->SetTrackedRenderer(renderer);
}


//-----------------------------------------------------------------------------
mitk::IRenderWindowPart* ThumbnailView::GetSelectedEditor()
{
  berry::IWorkbenchPage::Pointer page = this->GetSite()->GetPage();

  // Returns the active editor if it implements mitk::IRenderWindowPart
  mitk::IRenderWindowPart* renderWindowPart =
      dynamic_cast<mitk::IRenderWindowPart*>(page->GetActiveEditor().GetPointer());

  if (!renderWindowPart)
  {
    // No suitable active editor found, check visible editors
    foreach (berry::IEditorReference::Pointer editor, page->GetEditorReferences())
    {
      berry::IWorkbenchPart::Pointer part = editor->GetPart(false);
      if (page->IsPartVisible(part))
      {
        renderWindowPart = dynamic_cast<mitk::IRenderWindowPart*>(part.GetPointer());
        break;
      }
    }
  }

  return renderWindowPart;
}

}
