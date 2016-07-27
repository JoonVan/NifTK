/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkTrackedPointer.h"

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include <mitkOperation.h>
#include <mitkOperationActor.h>
#include <mitkRenderingManager.h>
#include <mitkSurface.h>
#include <mitkUndoController.h>

#include <niftkCoordinateAxesData.h>
#include <niftkPointSetUpdate.h>
#include <niftkPointUtils.h>


const bool mitk::TrackedPointer::UPDATE_VIEW_COORDINATE_DEFAULT(false);
const std::string mitk::TrackedPointer::TRACKED_POINTER_POINTSET_NAME("TrackedPointerPointSet");
const mitk::OperationType mitk::TrackedPointer::OP_UPDATE_POINTSET(9034657);

namespace mitk
{

//-----------------------------------------------------------------------------
TrackedPointer::TrackedPointer()
{
}


//-----------------------------------------------------------------------------
TrackedPointer::~TrackedPointer()
{
  if (m_DataStorage.IsNotNull())
  {
    mitk::DataNode::Pointer pointSetNode = m_DataStorage->GetNamedNode(TRACKED_POINTER_POINTSET_NAME);
    if (pointSetNode.IsNull())
    {
      m_DataStorage->Remove(pointSetNode);
    }
  }
}


//-----------------------------------------------------------------------------
void TrackedPointer::SetDataStorage(const mitk::DataStorage::Pointer& storage)
{
  m_DataStorage = storage;
  this->Modified();
}


//-----------------------------------------------------------------------------
mitk::PointSet::Pointer TrackedPointer::RetrievePointSet()
{
  assert(m_DataStorage);
  mitk::PointSet::Pointer result = NULL;

  mitk::DataNode::Pointer pointSetNode = m_DataStorage->GetNamedNode(TRACKED_POINTER_POINTSET_NAME);
  if (pointSetNode.IsNull())
  {
    result = mitk::PointSet::New();

    pointSetNode = mitk::DataNode::New();
    pointSetNode->SetData(result);
    pointSetNode->SetName(TRACKED_POINTER_POINTSET_NAME);
    m_DataStorage->Add(pointSetNode);
  }
  else
  {
    result = dynamic_cast<mitk::PointSet*>(pointSetNode->GetData());
  }

  return result;
}


//-----------------------------------------------------------------------------
void TrackedPointer::OnGrabPoint(const mitk::Point3D& point)
{
  mitk::PointSet::Pointer currentPointSet = this->RetrievePointSet();

  mitk::UndoStackItem::IncCurrObjectEventId();
  mitk::UndoStackItem::IncCurrGroupEventId();
  mitk::UndoStackItem::ExecuteIncrement();

  niftk::PointSetUpdate* doOp = new niftk::PointSetUpdate(OP_UPDATE_POINTSET, currentPointSet);
  doOp->AppendPoint(point);

  niftk::PointSetUpdate* undoOp = new niftk::PointSetUpdate(OP_UPDATE_POINTSET, currentPointSet);
  mitk::OperationEvent *operationEvent = new mitk::OperationEvent(this, doOp, undoOp, "Update PointSet");
  mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );

  this->ExecuteOperation(doOp);
}


//-----------------------------------------------------------------------------
void TrackedPointer::OnClearPoints()
{
  mitk::PointSet::Pointer currentPointSet = this->RetrievePointSet();

  mitk::UndoStackItem::IncCurrObjectEventId();
  mitk::UndoStackItem::IncCurrGroupEventId();
  mitk::UndoStackItem::ExecuteIncrement();

  niftk::PointSetUpdate* doOp = new niftk::PointSetUpdate(OP_UPDATE_POINTSET, NULL);
  niftk::PointSetUpdate* undoOp = new niftk::PointSetUpdate(OP_UPDATE_POINTSET, currentPointSet);
  mitk::OperationEvent *operationEvent = new mitk::OperationEvent(this, doOp, undoOp, "Update PointSet");
  mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent( operationEvent );

  this->ExecuteOperation(doOp);
}


//-----------------------------------------------------------------------------
void TrackedPointer::ExecuteOperation(mitk::Operation* operation)
{
  assert(m_DataStorage);
  assert(operation);

  switch (operation->GetOperationType())
  {
    case OP_UPDATE_POINTSET:

      niftk::PointSetUpdate* op = static_cast<niftk::PointSetUpdate*>(operation);
      mitk::PointSet::Pointer pointSet = this->RetrievePointSet();
      const mitk::PointSet *newPointSet = op->GetPointSet();

      niftk::CopyPointSets(*newPointSet,  *pointSet);

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    break;
  }
}


//-----------------------------------------------------------------------------
void TrackedPointer::Update(
         const vtkMatrix4x4& tipToPointerTransform,
         const mitk::DataNode::Pointer pointerToWorldNode,
         const mitk::DataNode::Pointer probeModel,
         mitk::Point3D& tipCoordinate
         )
{
  niftk::CoordinateAxesData::Pointer pointerToWorld = dynamic_cast<niftk::CoordinateAxesData*>(pointerToWorldNode->GetData());
  if (pointerToWorld.IsNull())
  {
    MITK_ERROR << "TrackedPointer::Update, invalid pointerToWorldNode";
    return;
  }

  vtkSmartPointer<vtkMatrix4x4> pointerToWorldTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  pointerToWorld->GetVtkMatrix(*pointerToWorldTransform);

  vtkSmartPointer<vtkMatrix4x4> combinedTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  combinedTransform->Identity();

  combinedTransform->Multiply4x4(pointerToWorldTransform, &tipToPointerTransform, combinedTransform);

  if (probeModel.IsNotNull())
  {
    mitk::BaseData::Pointer model = dynamic_cast<mitk::BaseData*>(probeModel->GetData());
    if (model.IsNotNull())
    {
      mitk::BaseGeometry* geometry = model->GetGeometry();
      if (geometry)
      {
        geometry->SetIndexToWorldTransformByVtkMatrix(combinedTransform);
        geometry->Modified();
      }
      model->Modified();
      probeModel->Modified();
    }
  }

  double coordinateIn[4] = {0, 0, 0, 1};
  double coordinateOut[4] = {0, 0, 0, 1};

  coordinateIn[0] = tipCoordinate[0];
  coordinateIn[1] = tipCoordinate[1];
  coordinateIn[2] = tipCoordinate[2];

  combinedTransform->MultiplyPoint(coordinateIn, coordinateOut);

  tipCoordinate[0] = coordinateOut[0];
  tipCoordinate[1] = coordinateOut[1];
  tipCoordinate[2] = coordinateOut[2];
}

//-----------------------------------------------------------------------------
} // end namespace

