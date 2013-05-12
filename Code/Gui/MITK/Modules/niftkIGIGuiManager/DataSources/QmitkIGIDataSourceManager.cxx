/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkIGIDataSourceManager.h"
#include <QMessageBox>
#include <QmitkStdMultiWidget.h>
#include <QDesktopServices>
#include <QDateTime>
#include <mitkDataStorage.h>
#include "mitkIGIDataSource.h"
#include "QmitkIGINiftyLinkDataSource.h"
#include "QmitkIGITrackerTool.h"
#include "QmitkIGIUltrasonixTool.h"
#include "QmitkIGIOpenCVDataSource.h"
#include "QmitkIGIDataSourceGui.h"

#ifdef _USE_NVAPI
#include "QmitkIGINVidiaDataSource.h"
#endif

const QColor QmitkIGIDataSourceManager::DEFAULT_ERROR_COLOUR = QColor(Qt::red);
const QColor QmitkIGIDataSourceManager::DEFAULT_WARNING_COLOUR = QColor(255,127,0); // orange
const QColor QmitkIGIDataSourceManager::DEFAULT_OK_COLOUR = QColor(Qt::green);
const int    QmitkIGIDataSourceManager::DEFAULT_FRAME_RATE = 2; // twice per second
const int    QmitkIGIDataSourceManager::DEFAULT_CLEAR_RATE = 2; // every 2 seconds
const bool   QmitkIGIDataSourceManager::DEFAULT_SAVE_ON_RECEIPT = true;
const bool   QmitkIGIDataSourceManager::DEFAULT_SAVE_IN_BACKGROUND = false;

//-----------------------------------------------------------------------------
QmitkIGIDataSourceManager::QmitkIGIDataSourceManager()
: m_DataStorage(NULL)
, m_StdMultiWidget(NULL)
, m_GridLayoutClientControls(NULL)
, m_NextSourceIdentifier(0)
, m_GuiUpdateTimer(NULL)
, m_ClearDownTimer(NULL)
{
  m_OKColour = DEFAULT_OK_COLOUR;
  m_WarningColour = DEFAULT_WARNING_COLOUR;
  m_ErrorColour = DEFAULT_ERROR_COLOUR;
  m_FrameRate = DEFAULT_FRAME_RATE;
  m_ClearDataRate = DEFAULT_CLEAR_RATE;
  m_DirectoryPrefix = GetDefaultPath();
  m_SaveOnReceipt = DEFAULT_SAVE_ON_RECEIPT;
  m_SaveInBackground = DEFAULT_SAVE_IN_BACKGROUND;
}


//-----------------------------------------------------------------------------
QmitkIGIDataSourceManager::~QmitkIGIDataSourceManager()
{
  // Stop both timers, to make sure nothing is triggering as we destroy.
  if (m_GuiUpdateTimer != NULL)
  {
    m_GuiUpdateTimer->stop();
  }

  if (m_ClearDownTimer != NULL)
  {
    m_ClearDownTimer->stop();
  }

  // Must delete the current GUI before the sources.
  this->DeleteCurrentGuiWidget();

  // smart pointers should delete the sources, and each source should delete its data.
  m_Sources.clear();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::DeleteCurrentGuiWidget()
{
  if (m_GridLayoutClientControls != NULL)
  {
    QLayoutItem *item = m_GridLayoutClientControls->itemAtPosition(0,0);
    if (item != NULL)
    {
      QWidget *widget = item->widget();
      if (widget != NULL)
      {
        widget->setVisible(false);
      }
      m_GridLayoutClientControls->removeItem(item);
      // this gets rid of the layout item
      delete item;
      // we should kill off the actual widget too
      // side note: that may not be the best way of doing it. currently everytime you
      //  double-click on a source it creates a new gui for it and deletes the previously
      //  active gui. it would be better if guis are cached for as long as the underlying
      //  data source is alive.
      // but for now this.
      delete widget;
    }
    delete m_GridLayoutClientControls;
    m_GridLayoutClientControls = 0;
  }
}


//-----------------------------------------------------------------------------
QString QmitkIGIDataSourceManager::GetDefaultPath()
{
  QString path;
  QDir directory;

  path = QDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
  directory.setPath(path);

  if (!directory.exists())
  {
    path = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    directory.setPath(path);
  }
  if (!directory.exists())
  {
    path = QDesktopServices::storageLocation(QDesktopServices::HomeLocation);
    directory.setPath(path);
  }
  if (!directory.exists())
  {
    path = QDir::currentPath();
    directory.setPath(path);
  }
  if (!directory.exists())
  {
    path = "";
  }
  return path;
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;

  foreach (mitk::IGIDataSource *source, m_Sources)
  {
    source->SetDataStorage(dataStorage);
  }

  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetFramesPerSecond(int framesPerSecond)
{
  if (m_GuiUpdateTimer != NULL)
  {
    int milliseconds = 1000 / framesPerSecond;
    m_GuiUpdateTimer->setInterval(milliseconds);
  }

  m_FrameRate = framesPerSecond;
  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetClearDataRate(int numberOfSeconds)
{
  if (m_ClearDownTimer != NULL)
  {
    int milliseconds = 1000 * numberOfSeconds;
    m_ClearDownTimer->setInterval(milliseconds);
  }

  m_ClearDataRate = numberOfSeconds;
  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetDirectoryPrefix(QString& directoryPrefix)
{
  m_DirectoryPrefix = directoryPrefix;
  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetErrorColour(QColor &colour)
{
  m_ErrorColour = colour;
  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetWarningColour(QColor &colour)
{
  m_WarningColour = colour;
  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::SetOKColour(QColor &colour)
{
  m_OKColour = colour;
  this->Modified();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::setupUi(QWidget* parent)
{
  Ui_QmitkIGIDataSourceManager::setupUi(parent);

  m_PlayPushButton->setIcon(QIcon(":/niftkIGIGuiManagerResources/play.png"));
  m_RecordPushButton->setIcon(QIcon(":/niftkIGIGuiManagerResources/record.png"));
  m_StopPushButton->setIcon(QIcon(":/niftkIGIGuiManagerResources/stop.png"));

  m_PlayPushButton->setEnabled(false); // not ready yet.
  m_RecordPushButton->setEnabled(true);
  m_StopPushButton->setEnabled(false);

  m_GuiUpdateTimer = new QTimer(this);
  m_GuiUpdateTimer->setInterval(1000/(int)(DEFAULT_FRAME_RATE));

  m_ClearDownTimer = new QTimer(this);
  m_ClearDownTimer->setInterval(1000*(int)DEFAULT_CLEAR_RATE);

  m_GridLayoutClientControls = new QGridLayout(m_Frame);
  m_GridLayoutClientControls->setSpacing(0);
  m_GridLayoutClientControls->setContentsMargins(0, 0, 0, 0);

  m_Frame->setContentsMargins(0, 0, 0, 0);

  m_SourceSelectComboBox->addItem("networked tracker");
  m_SourceSelectComboBox->addItem("networked ultrasonix scanner");
  m_SourceSelectComboBox->addItem("local frame grabber");

#ifdef _USE_NVAPI
  m_SourceSelectComboBox->addItem("local NVidia SDI");
#endif

  m_ToolManagerConsoleGroupBox->setCollapsed(true);
  m_ToolManagerConsole->setMaximumHeight(100);
  m_TableWidget->setMaximumHeight(150);

  connect(m_SourceSelectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentIndexChanged(int)));
  connect(m_AddSourcePushButton, SIGNAL(clicked()), this, SLOT(OnAddSource()) );
  connect(m_RemoveSourcePushButton, SIGNAL(clicked()), this, SLOT(OnRemoveSource()) );
  connect(m_TableWidget, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(OnCellDoubleClicked(int, int)) );
  connect(m_RecordPushButton, SIGNAL(clicked()), this, SLOT(OnRecordStart()) );
  connect(m_StopPushButton, SIGNAL(clicked()), this, SLOT(OnRecordStop()) );
  connect(m_GuiUpdateTimer, SIGNAL(timeout()), this, SLOT(OnUpdateGui()) );
  connect(m_ClearDownTimer, SIGNAL(timeout()), this, SLOT(OnCleanData()) );

  m_SourceSelectComboBox->setCurrentIndex(0);
}


//-----------------------------------------------------------------------------
bool QmitkIGIDataSourceManager::IsPortSpecificType()
{
  bool isPortSpecific = false;
  int currentIndex = m_SourceSelectComboBox->currentIndex();

  if (currentIndex == 0 || currentIndex == 1)
  {
    isPortSpecific = true;
  }

  return isPortSpecific;
}


//-----------------------------------------------------------------------------
int QmitkIGIDataSourceManager::GetRowNumberFromIdentifier(int identifier)
{
  int rowNumber = -1;
  for (unsigned int i = 0; i < m_Sources.size(); i++)
  {
    if (m_Sources[i].IsNotNull() && m_Sources[i]->GetIdentifier() == identifier)
    {
      rowNumber = i;
      break;
    }
  }
  return rowNumber;
}


//-----------------------------------------------------------------------------
int QmitkIGIDataSourceManager::GetIdentifierFromRowNumber(int rowNumber)
{
  int identifier = -1;
  if (rowNumber < m_TableWidget->rowCount() && rowNumber < (int)m_Sources.size())
  {
    identifier = m_Sources[rowNumber]->GetIdentifier();
  }
  return identifier;
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::UpdateToolDisplay(int toolIdentifier)
{
  int rowNumber = this->GetRowNumberFromIdentifier(toolIdentifier);

  if (rowNumber >= 0 && rowNumber <  (int)m_Sources.size())
  {
    std::string status = m_Sources[rowNumber]->GetStatus();
    std::string type = m_Sources[rowNumber]->GetType();
    std::string device = m_Sources[rowNumber]->GetName();
    std::string description = m_Sources[rowNumber]->GetDescription();

    int SubTools = m_Sources[rowNumber]->GetNumberOfTools();

    std::list<std::string> ToolList = m_Sources[rowNumber]->GetSubToolList();
    std::string Tool;
    int index=0;
    if ( SubTools > 0 ) 
    {
      foreach ( Tool, ToolList ) 
      {
        //this is horrible
        int thisType = -1;
        if ( type == "Tracker" || type == "Imager" ) 
        {

          if ( type == "Tracker" ) 
          {
            thisType = 0 ; 
          }
          if ( type == "Imager" ) 
          {
            thisType = 1;
          }
          
          mitk::IGIDataSource::Pointer source = m_Sources[rowNumber];
          QmitkIGINiftyLinkDataSource::Pointer NLSource = dynamic_cast< QmitkIGINiftyLinkDataSource*>(source.GetPointer());
          bool ToolAlreadyAdded = false; 
          for (int i = 0 ; i <  (int)m_Sources.size() ; i ++ )
          {
            //FIXME Tools with the same name being tracked by a different tracker 
            //(on a separate port) will confuse this
            if ( m_Sources[i]->GetDescription() == Tool  )
            {
              ToolAlreadyAdded = true;
            }
          }
          
          if ( ! ToolAlreadyAdded ) 
          {
            if ( index == 0 ) 
            {
              description=Tool;
              NLSource->SetDescription(Tool);
            }
            else
            {
              int tempToolIdentifier = AddSource (thisType, NLSource->GetPort(), NLSource->GetSocket());

              int tempRowNumber = this->GetRowNumberFromIdentifier(tempToolIdentifier);
              if ( type == "Tracker" ) 
              {
                mitk::IGIDataSource::Pointer tempsource = m_Sources[tempRowNumber];
                QmitkIGINiftyLinkDataSource::Pointer tempNLSource = dynamic_cast< QmitkIGINiftyLinkDataSource*>(tempsource.GetPointer());
                QmitkIGITrackerTool* TrackerTool = dynamic_cast<QmitkIGITrackerTool*>(tempNLSource.GetPointer());
                m_Sources[tempRowNumber]->SetDescription(Tool);
                TrackerTool->ProcessInitString(dynamic_cast<QmitkIGITrackerTool*>(source.GetPointer())->GetInitString());
              }
              else
              {
                m_Sources[tempRowNumber]->SetType(type);
                m_Sources[tempRowNumber]->SetName(device);
                m_Sources[tempRowNumber]->SetDescription(Tool);
              }
            }
          }
        }
        index++;
      }
    }
    else 
    {
      qDebug() << "there are no sub tools";
    }

    std::vector<std::string> fields;
    fields.push_back(status);
    fields.push_back(type);
    fields.push_back(device);
    fields.push_back(description);

    if (rowNumber == m_TableWidget->rowCount())
    {
      m_TableWidget->insertRow(rowNumber);
    }

    for (unsigned int i = 0; i < fields.size(); i++)
    {
      QTableWidgetItem *item = new QTableWidgetItem(QString::fromStdString(fields[i]));
      item->setTextAlignment(Qt::AlignCenter);
      item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

      m_TableWidget->setItem(rowNumber, i, item);
    }

    m_TableWidget->show();
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnCurrentIndexChanged(int indexNumber)
{
  Q_UNUSED(indexNumber);

  if (this->IsPortSpecificType())
  {
    m_PortNumberSpinBox->setEnabled(true);
  }
  else
  {
    m_PortNumberSpinBox->setEnabled(false);
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnAddSource()
{
  int sourceType = m_SourceSelectComboBox->currentIndex();
  int portNumber = m_PortNumberSpinBox->value();

  if (this->IsPortSpecificType())
  {
    if (m_PortsInUse.contains(portNumber))
    {
      QMessageBox msgBox(QMessageBox::Warning, tr("Server failure"), tr("Could not open socket: already listening on the selected port!"), QMessageBox::Ok);
      msgBox.exec();
      return;
    }
  }
  this->AddSource(sourceType, portNumber);
  return;
}


//------------------------------------------------
int QmitkIGIDataSourceManager::AddSource(int sourceType, int portNumber, NiftyLinkSocketObject* socket)
{
  
  mitk::IGIDataSource::Pointer source = NULL;

  if (sourceType == 0 || sourceType == 1)
  {
    QmitkIGINiftyLinkDataSource::Pointer niftyLinkSource = NULL;
    if (sourceType == 0)
    {
      niftyLinkSource = QmitkIGITrackerTool::New(m_DataStorage, socket);
    }
    else if (sourceType == 1)
    {
      niftyLinkSource = QmitkIGIUltrasonixTool::New(m_DataStorage, socket);
    }
    
    if (niftyLinkSource->ListenOnPort(portNumber))
    {
      m_PortsInUse.insert(portNumber);
    }
    source = niftyLinkSource;
  }
  else if (sourceType == 2)
  {
    source = QmitkIGIOpenCVDataSource::New(m_DataStorage);
  }
#ifdef _USE_NVAPI
  else if (sourceType == 3)
  {
    source = QmitkIGINVidiaDataSource::New(m_DataStorage);
  }
#endif
  else
  {
    std::cerr << "Matt, not implemented yet" << std::endl;
  }

  source->SetIdentifier(m_NextSourceIdentifier);
  m_Sources.push_back(source);

  // Registers this class as a listener to any status updates and connects to UpdateToolDisplay.
  // This means that regardless of the tool type, this class will receive a signal, and then
  // callback to the tool to ask for the necessary data to update the GUI row.
  source->DataSourceStatusUpdated
    += mitk::MessageDelegate1<QmitkIGIDataSourceManager, int>(
        this, &QmitkIGIDataSourceManager::UpdateToolDisplay );

  // Force an update.
  source->DataSourceStatusUpdated.Send(m_NextSourceIdentifier);

  // Increase this so that tools always have new identifier, regardless of what row of the table they are in.
  m_NextSourceIdentifier++;

   // Launch timers
  if (!m_GuiUpdateTimer->isActive())
  {
    m_GuiUpdateTimer->start();
  }
  if (!m_ClearDownTimer->isActive())
  {
    m_ClearDownTimer->start();
  }

  return source->GetIdentifier();
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnRemoveSource()
{
  if (m_TableWidget->rowCount() == 0)
    return;

  int rowIndex = m_TableWidget->currentRow();

  if (rowIndex < 0)
    rowIndex = m_TableWidget->rowCount()-1;

  mitk::IGIDataSource::Pointer source = m_Sources[rowIndex];

  // De-registers this class as a listener to this source.
  source->DataSourceStatusUpdated
    -= mitk::MessageDelegate1<QmitkIGIDataSourceManager, int>(
        this, &QmitkIGIDataSourceManager::UpdateToolDisplay );

  // If it is a networked tool, removes the port number from our list of "ports in use".
  QmitkIGINiftyLinkDataSource::Pointer niftyLinkSource = dynamic_cast<QmitkIGINiftyLinkDataSource*>(source.GetPointer());
  if (niftyLinkSource.IsNotNull())
  {
    int portNumber = niftyLinkSource->GetPort();
    //scan through the other source looking for any others using the same port, 
    //and kill them as well
    for ( int i = 0 ; i < m_TableWidget->rowCount() ; i ++ )
    {
      if ( i != rowIndex ) 
      {
        mitk::IGIDataSource::Pointer tempSource = m_Sources[i];
        QmitkIGINiftyLinkDataSource::Pointer tempNiftyLinkSource = dynamic_cast<QmitkIGINiftyLinkDataSource*>(tempSource.GetPointer());
        if ( tempNiftyLinkSource.IsNotNull() ) 
        {
          int tempPortNumber = tempNiftyLinkSource->GetPort();
          if ( tempPortNumber == portNumber ) 
          {
            tempSource->DataSourceStatusUpdated
             -= mitk::MessageDelegate1<QmitkIGIDataSourceManager, int>(
             this, &QmitkIGIDataSourceManager::UpdateToolDisplay );
            m_TableWidget->removeRow(i);
            m_TableWidget->update();
            m_Sources.erase(m_Sources.begin() + i);
            if ( i < rowIndex ) 
            {
              rowIndex--;
            }
          }
        }
      }
    }
    m_PortsInUse.remove(portNumber);
  }

  m_TableWidget->removeRow(rowIndex);
  m_TableWidget->update();

  // FIXME: this should not delete the gui if it doesnt belong to the to-be-removed source!
  //        but this should be a safe way of cleaning up for now
  this->DeleteCurrentGuiWidget();

  // This destroys the source. It is up to the source to correctly destroy itself,
  // as this class has no idea what the source is or what it contains etc.
  m_Sources.erase(m_Sources.begin() + rowIndex);

  if (m_TableWidget->rowCount() == 0)
  {
    m_GuiUpdateTimer->stop();
    m_ClearDownTimer->stop();
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnCellDoubleClicked(int row, int column)
{
  QmitkIGIDataSourceGui* sourceGui = NULL;

  mitk::IGIDataSource* source = m_Sources[row];
  const std::string classname = source->GetNameOfClass();

  std::string guiClassname = classname + "Gui";

  this->PrintStatusMessage(QString("INFO:Creating ") + QString::fromStdString(guiClassname) + QString(" for ") + QString::fromStdString(classname));

  std::list<itk::LightObject::Pointer> allGUIs = itk::ObjectFactoryBase::CreateAllInstance(guiClassname.c_str());
  for( std::list<itk::LightObject::Pointer>::iterator iter = allGUIs.begin();
       iter != allGUIs.end();
       ++iter )
  {
    if (sourceGui == NULL)
    {
      itk::LightObject* guiObject = (*iter).GetPointer();
      sourceGui = dynamic_cast<QmitkIGIDataSourceGui*>( guiObject );
    }
    else
    {
      MITK_ERROR << "There is more than one GUI for " << classname << " (several factories claim ability to produce a " << guiClassname << " ) " << std::endl;
      return;
    }
  }

  if (sourceGui != NULL)
  {
    this->DeleteCurrentGuiWidget();

    m_GridLayoutClientControls = new QGridLayout(m_Frame);
    m_GridLayoutClientControls->setSpacing(0);
    m_GridLayoutClientControls->setContentsMargins(0, 0, 0, 0);

    sourceGui->SetStdMultiWidget(this->GetStdMultiWidget());
    sourceGui->SetDataSource(source);
    sourceGui->Initialize(NULL);

    m_GridLayoutClientControls->addWidget(sourceGui, 0, 0);
  }
  else
  {
    this->PrintStatusMessage(QString("ERROR:For class ") + QString::fromStdString(classname) + ", could not create GUI class " + QString::fromStdString(guiClassname));
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnUpdateGui()
{
  igtl::TimeStamp::Pointer timeNow = igtl::TimeStamp::New();
  igtlUint64 idNow = GetTimeInNanoSeconds(timeNow);

  emit UpdateGuiStart(idNow);

  if (m_Sources.size() > 0)
  {
    foreach ( mitk::IGIDataSource::Pointer source, m_Sources )
    {
      // Work out row number of source.
      int rowNumber = this->GetRowNumberFromIdentifier(source->GetIdentifier());

      // First tell each source to update data.
      // For example, sources could copy to data storage.
      source->ProcessData(idNow);

      // Now calculate the stats.
      source->UpdateFrameRate();
      float rate = source->GetFrameRate();
      bool isValid = source->GetSuccessfullyProcessing();
      double lag = source->GetCurrentTimeLag(idNow);

      // Update the frame rate number.
      QTableWidgetItem *frameRateItem = new QTableWidgetItem(QString::number(rate));
      frameRateItem->setTextAlignment(Qt::AlignCenter);
      frameRateItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      m_TableWidget->setItem(rowNumber, 4, frameRateItem);

      // Update the lag number.
      QTableWidgetItem *lagItem = new QTableWidgetItem(QString::number(lag));
      lagItem->setTextAlignment(Qt::AlignCenter);
      lagItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
      m_TableWidget->setItem(rowNumber, 5, lagItem);

      // Update the status text.
      m_TableWidget->item(rowNumber, 0)->setText(QString::fromStdString(source->GetStatus()));

      // Update the status icon.
      QTableWidgetItem *tItem = m_TableWidget->item(rowNumber, 0);
      if (!isValid || lag > 1) // lag in seconds. TODO: This should be a preference.
      {
        // Highlight that current row is in error.
        QPixmap pix(22, 22);
        pix.fill(m_ErrorColour);
        tItem->setIcon(pix);
      }
      else
      {
        // Highlight that current row is OK.
        QPixmap pix(22, 22);
        pix.fill(m_OKColour);
        tItem->setIcon(pix);
      }
    }

    emit UpdateGuiFinishedDataSources(idNow);

    // Make sure table is refreshing.
    m_TableWidget->update();

    // Make sure scene rendered.
    mitk::RenderingManager * renderer = mitk::RenderingManager::GetInstance();
    renderer->RequestUpdateAll();

    emit UpdateGuiFinishedFinishedRendering(idNow);

    // Try to encourage rest of event loop to process before the timer swamps it.
    QCoreApplication::processEvents();
  }

  emit UpdateGuiEnd(idNow);
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnCleanData()
{
  // If gui timer is not running, we probably have no sources active.
  if (!m_GuiUpdateTimer->isActive())
  {
    return;
  }

  // If we are active, then simply ask each buffer to clean up in turn.
  foreach ( mitk::IGIDataSource::Pointer source, m_Sources )
  {
    source->CleanBuffer();
  }
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnRecordStart()
{
  // Generate a unique directory folder name.
  QString baseDirectory = m_DirectoryPrefix;

  igtl::TimeStamp::Pointer timeStamp = igtl::TimeStamp::New();

  igtlUint32 seconds;
  igtlUint32 nanoseconds;
  igtlUint64 millis;

  timeStamp->GetTime(&seconds, &nanoseconds);
  millis = (igtlUint64)seconds*1000 + nanoseconds/1000000;

  QDateTime dateTime;
  dateTime.setMSecsSinceEpoch(millis);

  QString formattedTime = dateTime.toString("yyyy-MM-dd-hh-mm-ss-zzz");

  QDir directory(baseDirectory + QDir::separator() + formattedTime);
  m_DirectoryChooser->setCurrentPath(directory.absolutePath());

  foreach ( mitk::IGIDataSource::Pointer source, m_Sources )
  {
    source->ClearBuffer();
    source->SetSavePrefix(directory.absolutePath().toStdString());
    source->SetSavingMessages(true);
    source->SetSaveInBackground(this->m_SaveInBackground);
    source->SetSaveOnReceipt(this->m_SaveOnReceipt);
  }

  m_RecordPushButton->setEnabled(false);
  m_StopPushButton->setEnabled(true);
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::OnRecordStop()
{
  foreach ( mitk::IGIDataSource::Pointer source, m_Sources )
  {
    source->SetSavingMessages(false);
  }

  m_RecordPushButton->setEnabled(true);
  m_StopPushButton->setEnabled(false);
}


//-----------------------------------------------------------------------------
void QmitkIGIDataSourceManager::PrintStatusMessage(const QString& message) const
{
  m_ToolManagerConsole->appendPlainText(message + "\n");
  MITK_INFO << "QmitkIGIDataSourceManager:" << message.toStdString() << std::endl;
}