/*=============================================================================

 NifTK: An image processing toolkit jointly developed by the
             Dementia Research Centre, and the Centre For Medical Image Computing
             at University College London.

 See:        http://dementia.ion.ucl.ac.uk/
             http://cmic.cs.ucl.ac.uk/
             http://www.ucl.ac.uk/

 Last Changed      : $Date: 2012-07-25 07:31:59 +0100 (Wed, 25 Jul 2012) $
 Revision          : $Revision: 9401 $
 Last modified by  : $Author: mjc $

 Original author   : m.clarkson@ucl.ac.uk

 Copyright (c) UCL : See LICENSE.txt in the top level directory for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 ============================================================================*/

#include "QmitkIGITrackerTool.h"
#include <mitkDataNode.h>
#include <mitkBaseData.h>
#include <mitkRenderingManager.h>

#include "mitkIGITestDataUtils.h"

NIFTK_IGITOOL_MACRO(NIFTKIGIGUI_EXPORT, QmitkIGITrackerTool, "IGI Tracker Tool");

//-----------------------------------------------------------------------------
QmitkIGITrackerTool::QmitkIGITrackerTool()
: m_MsgCounter(0)
, m_PointSetsInitialized(false)
, m_UseICP(false)
, m_ImageFiducialsDataNode(NULL)
, m_ImageFiducialsPointSet(NULL)
, m_TrackerFiducialsDataNode(NULL)
, m_TrackerFiducialsPointSet(NULL)
, m_FiducialRegistrationFilter(NULL)
, m_PermanentRegistrationFilter(NULL)
{
  m_FiducialRegistrationFilter = mitk::NavigationDataLandmarkTransformFilter::New();
  m_PermanentRegistrationFilter = mitk::NavigationDataLandmarkTransformFilter::New();
}


//-----------------------------------------------------------------------------
QmitkIGITrackerTool::~QmitkIGITrackerTool()
{
  this->RemoveFiducialsFromDataStorage();

  mitk::DataStorage* dataStorage = this->GetDataStorage();
  if (dataStorage != NULL)
  {
    mitk::DataNode::Pointer toolRep;
    foreach (toolRep, m_ToolRepresentations)
    {
      dataStorage->Remove(toolRep);
    }
  }
}


//-----------------------------------------------------------------------------
QString QmitkIGITrackerTool::GetNameWithRom(const QString name)
{
  QString result = name;
  if (!result.endsWith(".rom"))
  {
    result.append(".rom");
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::DataNode* QmitkIGITrackerTool::GetToolRepresentation(const QString toolName)
{

  mitk::DataNode::Pointer result = NULL;

  mitk::DataStorage* dataStorage = this->GetDataStorage();
  if (dataStorage != NULL)
  {
    mitk::DataNode::Pointer tempNode = dataStorage->GetNamedNode(toolName.toStdString());
    if (tempNode.IsNull())
    {
      mitk::Vector3D cp;
      cp[0] = 0;
      cp[1] = 0;
      cp[2] = 7.5;

      tempNode = mitk::CreateConeRepresentation(toolName.toStdString().c_str(), cp);
      tempNode->SetColor(0.4,0.70,0.85);

      dataStorage->Add(tempNode);
      m_ToolRepresentations.insert(toolName, tempNode);
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::EnableTool(const QString &toolName, const bool& enable)
{
  m_EnabledTools.insert(toolName, enable);

  // Send command to enable the given tool.
  CommandDescriptorXMLBuilder attachToolCmd;
  attachToolCmd.setCommandName("AttachTool");
  attachToolCmd.addParameter("ToolName", "QString", toolName);
  attachToolCmd.addParameter("Enabled", "bool", QString::number(enable));

  OIGTLStringMessage::Pointer cmdMsg(new OIGTLStringMessage());
  cmdMsg->setString(attachToolCmd.getXMLAsString());

  qDebug() << "TODO: send message " << attachToolCmd.getXMLAsString();

  QString statusMessage = QString("STATUS: tool ") + toolName + QString(", set to enabled=") + QString::number(enable) + QString("\n");
  emit StatusUpdate(statusMessage);
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::GetToolPosition(const QString &toolName)
{
  if (m_EnabledTools.contains(toolName) && m_EnabledTools.value(toolName))
  {
    OIGTLMessage::Pointer getPos;
    getPos.reset();

    OIGTLTrackingDataMessage::Create_GET(getPos);

    qDebug() << "TODO: send get current position message " << toolName;

    QString statusMessage = QString("STATUS: Requested position from tool ") + toolName + QString("\n");
    emit StatusUpdate(statusMessage);
  }
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::InitializeFiducials()
{

  mitk::DataStorage* dataStorage = this->GetDataStorage();
  if (dataStorage == NULL || m_PointSetsInitialized)
  {
    return;
  }

  m_ImageFiducialsPointSet = mitk::PointSet::New();
  m_ImageFiducialsDataNode = mitk::DataNode::New();
  m_ImageFiducialsDataNode->SetData(m_ImageFiducialsPointSet);

  mitk::Color color;
  color.Set(1.0f, 0.0f, 0.0f);
  m_ImageFiducialsDataNode->SetName("Registration_ImageFiducials");
  m_ImageFiducialsDataNode->SetColor(color);
  m_ImageFiducialsDataNode->SetBoolProperty( "updateDataOnRender", false );


  m_TrackerFiducialsPointSet = mitk::PointSet::New();

  m_TrackerFiducialsDataNode = mitk::DataNode::New();
  m_TrackerFiducialsDataNode->SetData(m_TrackerFiducialsPointSet);

  color.Set(0.0f, 1.0f, 0.0f);
  m_TrackerFiducialsDataNode->SetName("Registration_TrackingFiducials");
  m_TrackerFiducialsDataNode->SetColor(color);
  m_TrackerFiducialsDataNode->SetBoolProperty( "updateDataOnRender", false );

  dataStorage->Add(m_ImageFiducialsDataNode);
  dataStorage->Add(m_TrackerFiducialsDataNode);

  m_PointSetsInitialized = true;
}


//-----------------------------------------------------------------------------
mitk::DataNode* QmitkIGITrackerTool::GetImageFiducialsNode() const
{
  return m_ImageFiducialsDataNode;
}


//-----------------------------------------------------------------------------
mitk::DataNode* QmitkIGITrackerTool::GetTrackerFiducialsNode() const
{
  return m_TrackerFiducialsDataNode;
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::ClearFiducials()
{
  m_ImageFiducialsPointSet->Clear();
  m_TrackerFiducialsPointSet->Clear();
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::AddFiducialsToDataStorage()
{
  mitk::DataStorage* dataStorage = this->GetDataStorage();
  if (dataStorage != NULL)
  {
    if (!dataStorage->Exists(m_ImageFiducialsDataNode))
    {
      dataStorage->Add(m_ImageFiducialsDataNode);
    }
    if (!dataStorage->Exists(m_TrackerFiducialsDataNode))
    {
      dataStorage->Add(m_TrackerFiducialsDataNode);
    }
  }
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::RemoveFiducialsFromDataStorage()
{
  mitk::DataStorage* dataStorage = this->GetDataStorage();
  if (dataStorage != NULL)
  {
    if (dataStorage->Exists(m_ImageFiducialsDataNode))
    {
      dataStorage->Remove(m_ImageFiducialsDataNode);
    }
    if (dataStorage->Exists(m_TrackerFiducialsDataNode))
    {
      dataStorage->Remove(m_TrackerFiducialsDataNode);
    }
  }
}

//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::RegisterFiducials()
{
  if (m_ImageFiducialsPointSet.IsNull() || m_TrackerFiducialsPointSet.IsNull())
  {
    QString warning = QString("WARNING: Registration not possible\n") +
      "Fiducial data objects not found.\n" +
      "Please set 3 or more fiducials in the image and with the tracking system.\n";
    emit StatusUpdate(warning);
    return;
  }

  unsigned int minFiducialCount = 3; // \Todo: move to configurable parameter.

  if ((m_ImageFiducialsPointSet->GetSize() < (int)minFiducialCount)
    || (m_TrackerFiducialsPointSet->GetSize() < (int)minFiducialCount)
    || (m_ImageFiducialsPointSet->GetSize() != m_TrackerFiducialsPointSet->GetSize()))
  {
    QString warning = QString("WARNING: Registration not possible\n") +
        QString("Not enough fiducial pairs found. At least %1 fiducial must ").arg(minFiducialCount) +
        QString("exist for the image and the tracking system respectively.\n") +
        QString("Currently, %1 fiducials exist for the image, %2 fiducials exist for the tracking system").arg(m_ImageFiducialsPointSet->GetSize()).arg(m_TrackerFiducialsPointSet->GetSize());
    emit StatusUpdate(warning);
    return;
  }

  /* now we have two PointSets with enough points to perform a landmark based transform */
  m_FiducialRegistrationFilter->SetUseICPInitialization(m_UseICP);
  m_FiducialRegistrationFilter->SetSourceLandmarks(m_TrackerFiducialsPointSet);
  m_FiducialRegistrationFilter->SetTargetLandmarks(m_ImageFiducialsPointSet);
  m_FiducialRegistrationFilter->Update();

  QString registrationQuality = QString("%0: FRE is %1mm (Std.Dev. %2), \n"
    "RMS error is %3mm,\n"
    "Minimum registration error (best fitting landmark) is  %4mm,\n"
    "Maximum registration error (worst fitting landmark) is %5mm.")
    .arg("Fiducial Registration")
    .arg(m_FiducialRegistrationFilter->GetFRE(), 3, 'f', 3)
    .arg(m_FiducialRegistrationFilter->GetFREStdDev(), 3, 'f', 3)
    .arg(m_FiducialRegistrationFilter->GetRMSError(), 3, 'f', 3)
    .arg(m_FiducialRegistrationFilter->GetMinError(), 3, 'f', 3)
    .arg(m_FiducialRegistrationFilter->GetMaxError(), 3, 'f', 3);

  QString updateMessage = QString("Fiducial Registration complete, FRE: %0, RMS: %1")
    .arg(m_FiducialRegistrationFilter->GetFRE(), 3, 'f', 3)
    .arg(m_FiducialRegistrationFilter->GetRMSError(), 3, 'f', 3);

  QString statusUpdate = registrationQuality + "\n" + updateMessage + "\n";
  emit StatusUpdate(statusUpdate);
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::InterpretMessage(OIGTLMessage::Pointer msg)
{
  if (msg.data() != NULL &&
      (msg->getMessageType() == QString("TRANSFORM") || msg->getMessageType() == QString("TDATA"))
     )
  {
  //  this->DisplayTrackerData(msg);
    this->HandleTrackerData(msg);
  }
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::HandleTrackerData(OIGTLMessage::Pointer msg)
{
  if (msg->getMessageType() == QString("TDATA"))
  {
    OIGTLTrackingDataMessage::Pointer trMsg;
    trMsg = static_cast<OIGTLTrackingDataMessage::Pointer>(msg);

    QString toolName = trMsg->getTrackerToolName();

    // try to find DataNode for tool in DataStorage
		// Not quite, I want to have a list stored of all the 
		// associated data nodes. The nodes are associated manually 
		// when the GUI is is set up
    mitk::DataStorage* ds = this->GetDataStorage();
    if (ds == NULL)
    {
      QString message("ERROR: QmitkIGITrackerTool, DataStorage Access Error: Could not access DataStorage!");
      emit StatusUpdate(message);
      return;
    }

    toolName = this->GetNameWithRom(toolName);
    mitk::DataNode::Pointer tempNode = ds->GetNamedNode(toolName.toStdString().c_str());
    if (tempNode.IsNull())
    {
      QString message = QObject::tr("ERROR: QmitkIGITrackerTool, could not find node %1").arg(toolName);
      emit StatusUpdate(message);
      return;
    }

    // Get the transform from data
    mitk::BaseData * data = tempNode->GetData();
    mitk::AffineTransform3D::Pointer affineTransform = data->GetGeometry()->GetIndexToWorldTransform();

    if (affineTransform.IsNull())
    {
      QString message("ERROR: QmitkIGITrackerTool, AffineTransform IndexToWorldTransform not initialized!");
      emit StatusUpdate(message);
      return;
    }

    float inputTransformMat[4][4];
    trMsg->getMatrix(inputTransformMat);

    mitk::NavigationData::Pointer nd_in  = mitk::NavigationData::New();
    mitk::NavigationData::Pointer nd_out = mitk::NavigationData::New();
    mitk::NavigationData::PositionType p;

    mitk::FillVector3D(p, inputTransformMat[0][3], inputTransformMat[1][3], inputTransformMat[2][3]);
    nd_in->SetPosition(p);

    float * quats = new float[4];
    igtl::MatrixToQuaternion(inputTransformMat, quats);

    mitk::Quaternion mitkQuats(quats[0], quats[1], quats[2], quats[3]);
    nd_in->SetOrientation(mitkQuats);
    nd_in->SetDataValid(true);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    m_FiducialRegistrationFilter->SetInput(nd_in);
    m_FiducialRegistrationFilter->UpdateOutputData(0);
    nd_out = m_FiducialRegistrationFilter->GetOutput();
    nd_out->SetDataValid(true);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    //store the current scaling to set it after transformation
    mitk::Vector3D spacing = data->GetUpdatedTimeSlicedGeometry()->GetSpacing();
    //clear spacing of data to be able to set it again afterwards
    float scale[] = {1.0, 1.0, 1.0};
    data->GetGeometry()->SetSpacing(scale);

    /*now bring quaternion to affineTransform by using vnl_Quaternion*/
    affineTransform->SetIdentity();

    //calculate the transform from the quaternions
    static itk::QuaternionRigidTransform<double>::Pointer quatTransform = itk::QuaternionRigidTransform<double>::New();

    mitk::NavigationData::OrientationType orientation = nd_out->GetOrientation();
    // convert mitk::ScalarType quaternion to double quaternion because of itk bug
    vnl_quaternion<double> doubleQuaternion(orientation.x(), orientation.y(), orientation.z(), orientation.r());
    quatTransform->SetIdentity();
    quatTransform->SetRotation(doubleQuaternion);
    quatTransform->Modified();

    /* because of an itk bug, the transform can not be calculated with float data type.
    To use it in the mitk geometry classes, it has to be transfered to mitk::ScalarType which is float */
    static mitk::AffineTransform3D::MatrixType m;
    mitk::TransferMatrix(quatTransform->GetMatrix(), m);
    affineTransform->SetMatrix(m);

    ///*set the offset by convert from itkPoint to itkVector and setting offset of transform*/
    mitk::Vector3D pos;
    pos.Set_vnl_vector(nd_out->GetPosition().Get_vnl_vector());
    affineTransform->SetOffset(pos);
    affineTransform->Modified();

    //set the transform to data
    data->GetGeometry()->SetIndexToWorldTransform(affineTransform);
    //set the original spacing to keep scaling of the geometrical object
    data->GetGeometry()->SetSpacing(spacing);
    data->GetGeometry()->TransferItkToVtkTransform(); // update VTK Transform for rendering too
    data->GetGeometry()->Modified();
    data->Modified();
    //output->SetDataValid(true); // operation was successful, therefore data of output is valid.

    /*
    QString message = toolName;
    message.append("\n");
    for (int i = 0; i < 4; i++)
    {
      message.append(QObject::tr("%1 %2 %3 %4\n").arg(inputTransformMat[i][0]).arg(inputTransformMat[i][1]).arg(inputTransformMat[i][2]).arg(inputTransformMat[i][3]));
    }
    emit StatusUpdate(message);
    */

    mitk::RenderingManager * renderer = mitk::RenderingManager::GetInstance();
    renderer->ForceImmediateUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);

    //mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_ALL);

  }
}


//-----------------------------------------------------------------------------
void QmitkIGITrackerTool::DisplayTrackerData(OIGTLMessage::Pointer msg)
{
  // Don't print every message, otherwise the UI freezes
  if (msg->getMessageType() == QString("TRANSFORM"))
  {
    OIGTLTransformMessage::Pointer trMsg;
    trMsg = static_cast<OIGTLTransformMessage::Pointer>(msg);

    if (trMsg.data() != NULL)
    {
      // Print stuff
      QString header;
      header.setNum(m_MsgCounter);
      header.prepend("Message num: ");
      header.append("\nMessage from: ");
      header.append(trMsg->getHostName());
      header.append("\nMessage ID: ");
      header.append(QString::number(trMsg->getId()));
      header.append("\n");

      QString matrix = trMsg->getMatrixAsString();
      matrix.append("\n");

      QString message = header + matrix;

      emit StatusUpdate(message);
      qDebug() << "QmitkIGITrackerTool:" << message;
    }
  }
  else if (msg->getMessageType() == QString("TDATA"))
  {
    OIGTLTrackingDataMessage::Pointer trMsg;
    trMsg = static_cast<OIGTLTrackingDataMessage::Pointer>(msg);

    if (trMsg.data() != NULL)
    {
      QString header;
      header.setNum(m_MsgCounter);
      header.prepend("Message num: ");
      header.append("\nMessage from: ");
      header.append(trMsg->getHostName());
      header.append("\nMessage ID: ");
      header.append(QString::number(trMsg->getId()));
      header.append("\nTool ID: ");
      header.append(trMsg->getTrackerToolName());
      header.append("\n");

      QString matrix = trMsg->getMatrixAsString();
      matrix.append("\n");

      QString message = header + matrix;

      emit StatusUpdate(message);
      qDebug() << "QmitkIGITrackerTool:" << message;
    }
  }
}
