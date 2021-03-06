/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkDataSourcesView.h"
#include "niftkDataSourcesViewActivator.h"
#include <ctkPluginContext.h>
#include <ctkServiceReference.h>
#include <service/event/ctkEventAdmin.h>
#include <service/event/ctkEvent.h>
#include <service/event/ctkEventConstants.h>
#include <cassert>

namespace niftk
{

const QString DataSourcesView::VIEW_ID = "uk.ac.ucl.cmic.igidatasources";

//-----------------------------------------------------------------------------
DataSourcesView::DataSourcesView()
: m_DataSourceManagerWidget(NULL)
, m_SetupWasCalled(false)
{
}


//-----------------------------------------------------------------------------
DataSourcesView::~DataSourcesView()
{
  if (m_SetupWasCalled)
  {
    ctkPluginContext* context = niftk::DataSourcesViewActivator::getContext();
    if (context)
    {
      ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
      if (ref)
      {
        ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
        if (eventAdmin)
        {
          eventAdmin->unpublishSignal(this, SIGNAL(Updated(ctkDictionary)),"uk/ac/ucl/cmic/IGIUPDATE");
          eventAdmin->unpublishSignal(this, SIGNAL(RecordingStarted(ctkDictionary)), "uk/ac/ucl/cmic/IGIRECORDINGSTARTED");
          eventAdmin->unpublishSignal(this, SIGNAL(RecordingStopped(ctkDictionary)), "uk/ac/ucl/cmic/IGIRECORDINGSTOPPED");
        }
      }
    }

    bool ok = false;
    ok = QObject::disconnect(m_DataSourceManagerWidget, SIGNAL(UpdateGuiFinishedDataSources(niftk::IGIDataSourceI::IGITimeType)), this, SLOT(OnUpdateGuiEnd(niftk::IGIDataSourceI::IGITimeType)));
    assert(ok);
    ok = QObject::disconnect(m_DataSourceManagerWidget, SIGNAL(RecordingStarted(QString)), this, SLOT(OnRecordingStarted(QString)));
    assert(ok);
    ok = QObject::disconnect(m_DataSourceManagerWidget, SIGNAL(RecordingStopped()), this, SLOT(OnRecordingStopped()));
    assert(ok);
  }
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnUpdateShouldPause(const ctkEvent& event)
{
  // Comming from CTK event bus.
  m_DataSourceManagerWidget->PauseUpdate();
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnUpdateShouldRestart(const ctkEvent& event)
{
  // Comming from CTK event bus.
  m_DataSourceManagerWidget->RestartUpdate();
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnPreferencesChanged(const berry::IBerryPreferences*)
{
  this->RetrievePreferenceValues();
}


//-----------------------------------------------------------------------------
void DataSourcesView::SetFocus()
{
  m_DataSourceManagerWidget->setFocus();
}


//-----------------------------------------------------------------------------
void DataSourcesView::CreateQtPartControl( QWidget *parent )
{
  m_DataSourceManagerWidget = new IGIDataSourceManagerWidget(this->GetDataStorage(), parent);

  this->RetrievePreferenceValues();

  bool ok = false;
  ok = QObject::connect(m_DataSourceManagerWidget, SIGNAL(UpdateGuiFinishedDataSources(niftk::IGIDataSourceI::IGITimeType)), this, SLOT(OnUpdateGuiEnd(niftk::IGIDataSourceI::IGITimeType)));
  assert(ok);
  ok = QObject::connect(m_DataSourceManagerWidget, SIGNAL(RecordingStarted(QString)), this, SLOT(OnRecordingStarted(QString)), Qt::QueuedConnection);
  assert(ok);
  ok = QObject::connect(m_DataSourceManagerWidget, SIGNAL(RecordingStopped()), this, SLOT(OnRecordingStopped()), Qt::QueuedConnection);
  assert(ok);

  ctkPluginContext* context = niftk::DataSourcesViewActivator::getContext();
  ctkServiceReference ref = context->getServiceReference<ctkEventAdmin>();
  if (ref)
  {
    ctkEventAdmin* eventAdmin = context->getService<ctkEventAdmin>(ref);
    eventAdmin->publishSignal(this, SIGNAL(Updated(ctkDictionary)),"uk/ac/ucl/cmic/IGIUPDATE");
    eventAdmin->publishSignal(this, SIGNAL(RecordingStarted(ctkDictionary)), "uk/ac/ucl/cmic/IGIRECORDINGSTARTED");
    eventAdmin->publishSignal(this, SIGNAL(RecordingStopped(ctkDictionary)), "uk/ac/ucl/cmic/IGIRECORDINGSTOPPED");

    ctkDictionary properties;
    properties[ctkEventConstants::EVENT_TOPIC] = "uk/ac/ucl/cmic/IGIUPDATEPAUSE";
    eventAdmin->subscribeSlot(this, SLOT(OnUpdateShouldPause(ctkEvent)), properties);
    properties[ctkEventConstants::EVENT_TOPIC] = "uk/ac/ucl/cmic/IGIUPDATERESTART";
    eventAdmin->subscribeSlot(this, SLOT(OnUpdateShouldRestart(ctkEvent)), properties);
    properties[ctkEventConstants::EVENT_TOPIC] = "uk/ac/ucl/cmic/IGIFOOTSWITCH2START";
    eventAdmin->subscribeSlot(this, SLOT(OnToggleRecording(ctkEvent)), properties);
  }

  m_SetupWasCalled = true;
}


//-----------------------------------------------------------------------------
void DataSourcesView::RetrievePreferenceValues()
{
  berry::IPreferences::Pointer prefs = GetPreferences();
  if (prefs.IsNotNull())
  {
    QString path = prefs->Get("output directory prefix", "");
    if (path == "")
    {
      path = m_DataSourceManagerWidget->GetDefaultWritablePath();
    }

    int refreshRate = prefs->GetInt("refresh rate", niftk::IGIDataSourceManager::DEFAULT_FRAME_RATE);

    m_DataSourceManagerWidget->SetDirectoryPrefix(path);
    m_DataSourceManagerWidget->SetFramesPerSecond(refreshRate);
  }
  else
  {
    QString defaultPath = m_DataSourceManagerWidget->GetDefaultWritablePath();
    m_DataSourceManagerWidget->SetDirectoryPrefix(defaultPath);
    m_DataSourceManagerWidget->SetFramesPerSecond(niftk::IGIDataSourceManager::DEFAULT_FRAME_RATE);
  }
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnUpdateGuiEnd(niftk::IGIDataSourceI::IGITimeType timeStamp)
{
  ctkDictionary properties;
  properties["timeStamp"] = timeStamp;
  emit Updated(properties);
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnRecordingStarted(QString baseDirectory)
{
  ctkDictionary properties;
  properties["directory"] = baseDirectory;
  emit RecordingStarted(properties);
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnRecordingStopped()
{
  ctkDictionary properties;
  emit RecordingStopped(properties);
}


//-----------------------------------------------------------------------------
void DataSourcesView::OnToggleRecording(const ctkEvent& event)
{
  if (m_DataSourceManagerWidget->IsRecording())
  {
    m_DataSourceManagerWidget->StopRecording();
  }
  else
  {
    m_DataSourceManagerWidget->StartRecording();
  }
}

} // end namespace
