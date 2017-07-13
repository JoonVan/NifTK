/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkDataStorageUtils.h"

#include <mitkApplyTransformMatrixOperation.h>
#include <mitkDataStorage.h>
#include <mitkInteractionConst.h>
#include <mitkNodePredicateDataType.h>
#include <mitkOperationEvent.h>
#include <mitkUndoController.h>

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

#include "niftkAffineTransformDataNodeProperty.h"
#include "niftkCoordinateAxesData.h"
#include "niftkFileIOUtils.h"

namespace niftk
{

//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindFirstParentImage(const mitk::DataStorage* storage, const mitk::DataNode::Pointer node, bool lookForBinary)
{
  mitk::DataNode::Pointer result = NULL;

  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = storage->GetSources( node, isImage );

  for (unsigned int i = 0; i < possibleParents->size(); i++)
  {

    mitk::DataNode* possibleNode = (*possibleParents)[i];

    bool isBinary;
    possibleNode->GetBoolProperty("binary", isBinary);

    if (isBinary == lookForBinary)
    {
      result = possibleNode;
    }
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::DataStorage::SetOfObjects::Pointer FindDerivedVisibleNonHelperChildren(const mitk::DataStorage* storage, const mitk::DataNode::Pointer node)
{
  mitk::DataStorage::SetOfObjects::Pointer results = mitk::DataStorage::SetOfObjects::New();

  unsigned int counter = 0;
  mitk::DataStorage::SetOfObjects::ConstPointer possibleChildren = storage->GetDerivations( node, NULL, false);
  for (unsigned int i = 0; i < possibleChildren->size(); i++)
  {
    mitk::DataNode* possibleNode = (*possibleChildren)[i];

    bool isVisible = false;
    possibleNode->GetBoolProperty("visible", isVisible);

    bool isHelper = false;
    possibleNode->GetBoolProperty("helper object", isHelper);

    if (isVisible && !isHelper)
    {
      results->InsertElement(counter, possibleNode);
      counter++;
    }
  }
  return results;
}


//-----------------------------------------------------------------------------
mitk::DataStorage::SetOfObjects::Pointer FindDerivedImages(const mitk::DataStorage* storage, const mitk::DataNode::Pointer node, bool lookForBinary )
{
  mitk::DataStorage::SetOfObjects::Pointer results = mitk::DataStorage::SetOfObjects::New();

  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer possibleChildren = storage->GetDerivations( node, isImage, true);

  unsigned int counter = 0;
  for (unsigned int i = 0; i < possibleChildren->size(); i++)
  {

    mitk::DataNode* possibleNode = (*possibleChildren)[i];

    bool isBinary;
    possibleNode->GetBoolProperty("binary", isBinary);

    if (isBinary == lookForBinary)
    {
      results->InsertElement(counter, possibleNode);
      counter++;
    }
  }

  return results;
}


//-----------------------------------------------------------------------------
bool IsNodeAHelperObject(const mitk::DataNode* node)
{
  bool result = false;

  if (node != NULL)
  {
    bool isHelper(false);
    if (node->GetBoolProperty("helper object", isHelper) && isHelper)
    {
      result = true;
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
bool IsNodeAGreyScaleImage(const mitk::DataNode::Pointer node)
{
  bool result = false;

  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    if (image.IsNotNull())
    {
      bool isBinary(false);
      bool foundBinaryProperty = node->GetBoolProperty("binary", isBinary);

      if ((foundBinaryProperty && !isBinary) || !foundBinaryProperty)
      {
        result = true;
      }
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
bool IsNodeABinaryImage(const mitk::DataNode::Pointer node)
{
  bool result = false;

  if (node.IsNotNull())
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    if (image.IsNotNull())
    {
      bool isBinary(false);
      bool foundBinaryProperty = node->GetBoolProperty("binary", isBinary);

      if(foundBinaryProperty && isBinary)
      {
        result = true;
      }
    }
  }

  return result;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindNthImage(const std::vector<mitk::DataNode*> &nodes, int n, bool lookForBinary)
{
  if (nodes.empty()) return NULL;

  int numberOfMatchingNodesFound = 0;

  for(unsigned int i = 0; i < nodes.size(); ++i)
  {
      bool isImage(false);
      if (nodes.at(i)->GetData())
      {
        isImage = dynamic_cast<mitk::Image*>(nodes.at(i)->GetData()) != NULL;
      }

      bool isBinary;
      nodes.at(i)->GetBoolProperty("binary", isBinary);

      if (isImage && isBinary == lookForBinary)
      {
        numberOfMatchingNodesFound++;
        if (numberOfMatchingNodesFound == n)
        {
          return nodes.at(i);
        }
      }
  }
  return NULL;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindNthGreyScaleImage(const std::vector<mitk::DataNode*> &nodes, int n )
{
  return FindNthImage(nodes, n, false);
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindNthBinaryImage(const std::vector<mitk::DataNode*> &nodes, int n )
{
  return FindNthImage(nodes, n, true);
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindFirstGreyScaleImage(const std::vector<mitk::DataNode*> &nodes )
{
  return FindNthGreyScaleImage(nodes, 1);
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindFirstBinaryImage(const std::vector<mitk::DataNode*> &nodes )
{
  return FindNthBinaryImage(nodes, 1);
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindFirstParent(const mitk::DataStorage* storage, const mitk::DataNode::Pointer node)
{
  mitk::DataNode::Pointer result = NULL;

  mitk::DataStorage::SetOfObjects::ConstPointer possibleParents = storage->GetSources(node);
  if (possibleParents->size() > 0)
  {
    result = (*possibleParents)[0];
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer FindParentGreyScaleImage(const mitk::DataStorage* storage, const mitk::DataNode::Pointer node)
{
  mitk::DataNode::Pointer result = NULL;

  if (node.IsNotNull())
  {
    mitk::DataNode::Pointer nodeToCheck = node;
    mitk::DataNode::Pointer parent = NULL;
    do
    {
      parent = FindFirstParent(storage, nodeToCheck);
      if (parent.IsNotNull())
      {
        if (IsNodeAGreyScaleImage(parent))
        {
          result = parent;
          break;
        }
        else
        {
          nodeToCheck = parent;
        }
      }
    } while (parent.IsNotNull());
  }
  return result;
}


//-----------------------------------------------------------------------------
mitk::DataStorage::SetOfObjects::Pointer FindNodesStartingWith(const mitk::DataStorage* dataStorage, const std::string prefix)
{
  mitk::DataStorage::SetOfObjects::Pointer results = mitk::DataStorage::SetOfObjects::New();

  unsigned int counter = 0;
  mitk::DataStorage::SetOfObjects::ConstPointer all = dataStorage->GetAll();
  for (unsigned int i = 0; i < all->size(); i++)
  {
    mitk::DataNode* possibleNode = (*all)[i];
    if (possibleNode->GetName().compare(0, prefix.length(), prefix) == 0)
    {
      results->InsertElement(counter, possibleNode);
      counter++;
    }
  }

  return results;
}


//-----------------------------------------------------------------------------
mitk::TimeGeometry::Pointer GetPreferredGeometry(const mitk::DataStorage* dataStorage, const std::vector<mitk::DataNode*>& nodes, const int& nodeIndex)
{
  mitk::TimeGeometry::Pointer geometry = NULL;

  int indexThatWeActuallyUsed = -1;

  // If nodeIndex < 0, we are choosing the best geometry from all available nodes.
  if (nodeIndex < 0)
  {

    // First try to find an image geometry, and if so, use the first one.
    mitk::Image::Pointer image = NULL;
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
      image = dynamic_cast<mitk::Image*>(nodes[i]->GetData());
      if (image.IsNotNull())
      {
        geometry = image->GetTimeGeometry();
        indexThatWeActuallyUsed = i;
        break;
      }
    }

    // Failing that, use the first geometry available.
    if (geometry.IsNull())
    {
      for (unsigned int i = 0; i < nodes.size(); i++)
      {
        mitk::BaseData::Pointer data = nodes[i]->GetData();
        if (data.IsNotNull())
        {
          geometry = data->GetTimeGeometry();
          indexThatWeActuallyUsed = i;
          break;
        }
      }
    }
  }
  // So, the caller has nominated a specific node, lets just use that one.
  else if (nodeIndex >= 0 && nodeIndex < (int)nodes.size())
  {
    mitk::BaseData::Pointer data = nodes[nodeIndex]->GetData();
    if (data.IsNotNull())
    {
      geometry = data->GetTimeGeometry();
      indexThatWeActuallyUsed = nodeIndex;
    }
  }
  // Essentially, the nodeIndex is garbage, so just pick the first one.
  else
  {
    mitk::BaseData::Pointer data = nodes[0]->GetData();
    if (data.IsNotNull())
    {
      geometry = data->GetTimeGeometry();
      indexThatWeActuallyUsed = 0;
    }
  }

  // In addition, if the node is NOT a greyscale image, we try and search the parents
  // of the node to find a greyscale image and use that one in preference.
  // This assumes that derived datasets, such as point sets, surfaces, segmented
  // volumes are correctly assigned to parents.
  if (indexThatWeActuallyUsed != -1)
  {
    if (!IsNodeAGreyScaleImage(nodes[indexThatWeActuallyUsed]))
    {
      mitk::DataNode::Pointer node = FindParentGreyScaleImage(dataStorage, nodes[indexThatWeActuallyUsed]);
      if (node.IsNotNull())
      {
        mitk::BaseData::Pointer data = nodes[0]->GetData();
        if (data.IsNotNull())
        {
          geometry = data->GetTimeGeometry();
        }
      }
    }
  }
  return geometry;
}


//-----------------------------------------------------------------------------
void LoadMatrixOrCreateDefault(
    const std::string& fileName,
    const std::string& nodeName,
    const bool& helperObject,
    mitk::DataStorage* dataStorage)
{
  vtkSmartPointer<vtkMatrix4x4> matrix = LoadVtkMatrix4x4FromFile(fileName);
  if (matrix.GetPointer() == NULL)
  {
    matrix = vtkSmartPointer<vtkMatrix4x4>::New();
    matrix->Identity();
  }

  mitk::DataNode::Pointer node = dataStorage->GetNamedNode(nodeName);
  if (node.IsNull())
  {
    node = mitk::DataNode::New();
    node->SetName(nodeName);
    node->SetBoolProperty("show text", false);
    node->SetIntProperty("size", 10);
    node->SetVisibility(false); // by default we don't need to see it.
    node->SetBoolProperty("helper object", helperObject);
  }

  std::string propertyName = "niftk.transform";
  AffineTransformDataNodeProperty::Pointer affTransProp = static_cast<AffineTransformDataNodeProperty*>(node->GetProperty(propertyName.c_str()));
  if (affTransProp.IsNull())
  {
    affTransProp = AffineTransformDataNodeProperty::New();
    node->SetProperty(propertyName.c_str(), affTransProp);
  }
  affTransProp->SetTransform(*matrix);

  CoordinateAxesData::Pointer coordinateAxes = dynamic_cast<CoordinateAxesData*>(node->GetData());
  if (coordinateAxes.IsNull())
  {
    coordinateAxes = CoordinateAxesData::New();
    node->SetData(coordinateAxes);
  }
  coordinateAxes->SetVtkMatrix(*matrix);

  if (!dataStorage->Exists(node))
  {
    dataStorage->Add(node);
  }
  node->Modified();
}


//-----------------------------------------------------------------------------
void GetCurrentTransformFromNode ( const mitk::DataNode::Pointer& node , vtkMatrix4x4& outputMatrix )
{
  if (node.IsNull())
  {
    mitkThrow() << "In GetCurrentTransform, node is NULL";
  }

  mitk::AffineTransform3D::Pointer affineTransform = node->GetData()->GetGeometry()->GetIndexToWorldTransform();
  itk::Matrix<double, 3, 3>  matrix;
  itk::Vector<double, 3> offset;
  matrix = affineTransform->GetMatrix();
  offset = affineTransform->GetOffset();

  outputMatrix.Identity();
  for ( int i = 0 ; i < 3 ; i ++ )
  {
    for ( int j = 0 ; j < 3 ; j ++ )
    {
      outputMatrix.SetElement (i, j, matrix[i][j]);
    }
  }
  for ( int i = 0 ; i < 3 ; i ++ )
  {
    outputMatrix.SetElement (i, 3, offset[i]);
  }
}


//-----------------------------------------------------------------------------
void ComposeTransformWithNode(const vtkMatrix4x4& transform, mitk::DataNode::Pointer& node)
{
  if (node.IsNull())
  {
    mitkThrow() << "In ComposeTransformWithNode, node is NULL";
  }

  vtkSmartPointer<vtkMatrix4x4> currentMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  GetCurrentTransformFromNode(node, *currentMatrix);

  vtkSmartPointer<vtkMatrix4x4> newMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  newMatrix->Multiply4x4(&transform, currentMatrix, newMatrix);

  ApplyTransformToNode(*newMatrix, node);
}


//-----------------------------------------------------------------------------
void ApplyTransformToNode(const vtkMatrix4x4& transform, mitk::DataNode::Pointer& node)
{
  if (node.IsNull())
  {
    mitkThrow() << "In ApplyTransformToNode, node is NULL";
  }

  mitk::BaseData::Pointer baseData = node->GetData();
  if (baseData.IsNull())
  {
    mitkThrow() << "In ApplyTransformToNode, baseData is NULL";
  }

  mitk::BaseGeometry* geometry = baseData->GetGeometry();
  if (!geometry)
  {
    mitkThrow() << "In ApplyTransformToNode, geometry is NULL";
  }

  CoordinateAxesData::Pointer axes = dynamic_cast<CoordinateAxesData*>(node->GetData());
  if (axes.IsNotNull())
  {
    AffineTransformDataNodeProperty::Pointer property = dynamic_cast<AffineTransformDataNodeProperty*>(node->GetProperty("niftk.transform"));
    if (property.IsNotNull())
    {
      property->SetTransform(transform);
      property->Modified();
    }
  }

  vtkSmartPointer<vtkMatrix4x4> nonConstTransform = vtkSmartPointer<vtkMatrix4x4>::New();
  nonConstTransform->DeepCopy(&transform);

  mitk::Point3D dummyPoint;
  dummyPoint.Fill(0);

  mitk::ApplyTransformMatrixOperation* doOp = new mitk::ApplyTransformMatrixOperation(mitk::OpAPPLYTRANSFORMMATRIX, nonConstTransform, dummyPoint);

  if (mitk::UndoController::GetCurrentUndoModel() != NULL)
  {
    vtkSmartPointer<vtkMatrix4x4> inverse = vtkSmartPointer<vtkMatrix4x4>::New();
    inverse->DeepCopy(&transform);
    inverse->Invert();

    mitk::ApplyTransformMatrixOperation* undoOp = new mitk::ApplyTransformMatrixOperation(mitk::OpAPPLYTRANSFORMMATRIX, inverse, dummyPoint);
    mitk::OperationEvent* operationEvent = new mitk::OperationEvent(geometry, doOp, undoOp, "ApplyTransformToNode");
    mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(operationEvent);
    geometry->ExecuteOperation(doOp);
  }
  else
  {
    geometry->ExecuteOperation(doOp);
    delete doOp;
  }
}

}
