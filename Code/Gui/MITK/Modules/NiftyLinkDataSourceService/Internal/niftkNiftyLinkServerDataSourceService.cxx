/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkNiftyLinkServerDataSourceService.h"

namespace niftk
{

//-----------------------------------------------------------------------------
niftk::IGIDataSourceLocker NiftyLinkServerDataSourceService::s_Lock;


//-----------------------------------------------------------------------------
NiftyLinkServerDataSourceService::NiftyLinkServerDataSourceService(
    QString factoryName,
    const IGIDataSourceProperties& properties,
    mitk::DataStorage::Pointer dataStorage
    )
: NiftyLinkDataSourceService(QString("NLServer-") + QString::number(s_Lock.GetNextSourceNumber()),
                             factoryName, properties, dataStorage)
{
  int portNumber = 0;
  if(!properties.contains("port"))
  {
    mitkThrow() << "Port number not specified!";
  }
  portNumber = (properties.value("port")).toInt();

  m_Server = new NiftyLinkTcpServer();

  // Register to slots before we start to use it, otherwise you miss error messages.
  bool ok = false;
  ok = QObject::connect(m_Server, SIGNAL(ClientConnected(int)), this, SLOT(OnClientConnected(int)));
  assert(ok);
  ok = QObject::connect(m_Server, SIGNAL(ClientDisconnected(int)), this, SLOT(OnClientDisconnected(int)));
  assert(ok);
  ok = QObject::connect(m_Server, SIGNAL(SocketError(int, QAbstractSocket::SocketError, QString)), this, SLOT(OnSocketError(int, QAbstractSocket::SocketError, QString)));
  assert(ok);
  ok = QObject::connect(m_Server, SIGNAL(MessageReceived(int, niftk::NiftyLinkMessageContainer::Pointer)), this, SLOT(OnMessageReceived(int, niftk::NiftyLinkMessageContainer::Pointer)), Qt::DirectConnection);
  assert(ok);

  m_Server->listen(QHostAddress::LocalHost, portNumber);
  if (!m_Server->isListening())
  {
    mitkThrow() << "Failed to start NiftyLinkTcpServer on port " << portNumber
                << ", due to " << m_Server->errorString().toStdString();
  }

  QString deviceName = this->GetName();
  m_ServerNumber = (deviceName.remove(0, 9)).toInt(); // Should match string NLServer- above
}


//-----------------------------------------------------------------------------
NiftyLinkServerDataSourceService::~NiftyLinkServerDataSourceService()
{
  this->StopCapturing();

  bool ok = false;
  ok = QObject::disconnect(m_Server, SIGNAL(ClientConnected(int)), this, SLOT(OnClientConnected(int)));
  assert(ok);
  ok = QObject::disconnect(m_Server, SIGNAL(ClientDisconnected(int)), this, SLOT(OnClientDisconnected(int)));
  assert(ok);
  ok = QObject::disconnect(m_Server, SIGNAL(SocketError(int, QAbstractSocket::SocketError, QString)), this, SLOT(OnSocketError(int, QAbstractSocket::SocketError, QString)));
  assert(ok);
  ok = QObject::disconnect(m_Server, SIGNAL(MessageReceived(int, niftk::NiftyLinkMessageContainer::Pointer)), this, SLOT(OnMessageReceived(int, niftk::NiftyLinkMessageContainer::Pointer)));
  assert(ok);

  m_Server->Shutdown();
  delete m_Server;

  s_Lock.RemoveSource(m_ServerNumber);
}


//-----------------------------------------------------------------------------
void NiftyLinkServerDataSourceService::OnClientConnected(int portNumber)
{
  this->SetStatus("Client connected");
}


//-----------------------------------------------------------------------------
void NiftyLinkServerDataSourceService::OnClientDisconnected(int portNumber)
{
  this->SetStatus("Client disconnected");
}


//-----------------------------------------------------------------------------
void NiftyLinkServerDataSourceService::OnSocketError(int portNumber, QAbstractSocket::SocketError errorCode, QString errorString)
{
  this->SetStatus("Socket error");
}


//-----------------------------------------------------------------------------
void NiftyLinkServerDataSourceService::OnMessageReceived(int portNumber, niftk::NiftyLinkMessageContainer::Pointer message)
{
  this->SetStatus("Message received");
}

} // end namespace
