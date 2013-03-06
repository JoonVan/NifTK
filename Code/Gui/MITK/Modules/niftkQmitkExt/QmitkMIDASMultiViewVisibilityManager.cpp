/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkMIDASMultiViewVisibilityManager.h"
#include "QmitkRenderWindow.h"
#include "QmitkMIDASSingleViewWidget.h"
#include "mitkBaseRenderer.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkDataStorageUtils.h"
#include "mitkImageAccessByItk.h"
#include "mitkMIDASTool.h"
#include "mitkMIDASPolyTool.h"
#include "vtkRenderWindow.h"
#include "itkConversionUtils.h"
#include "itkSpatialOrientationAdapter.h"

QmitkMIDASMultiViewVisibilityManager::QmitkMIDASMultiViewVisibilityManager(mitk::DataStorage::Pointer dataStorage)
: m_InDataStorageChanged(false)
, m_AutomaticallyAddChildren(true)
, m_Accumulate(false)
{
  assert(dataStorage);
  m_DataStorage = dataStorage;

  m_DataNodes.clear();
  m_Widgets.clear();
  m_ObserverToVisibilityMap.clear();

  m_DataStorage->AddNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkMIDASMultiViewVisibilityManager, const mitk::DataNode*>
    ( this, &QmitkMIDASMultiViewVisibilityManager::NodeAddedProxy ) );

  m_DataStorage->RemoveNodeEvent.AddListener(
      mitk::MessageDelegate1<QmitkMIDASMultiViewVisibilityManager, const mitk::DataNode*>
    ( this, &QmitkMIDASMultiViewVisibilityManager::NodeRemovedProxy ) );
}

QmitkMIDASMultiViewVisibilityManager::~QmitkMIDASMultiViewVisibilityManager()
{
  if (m_DataStorage.IsNotNull())
  {
    m_DataStorage->AddNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkMIDASMultiViewVisibilityManager, const mitk::DataNode*>
    ( this, &QmitkMIDASMultiViewVisibilityManager::NodeAddedProxy ));

    m_DataStorage->RemoveNodeEvent.RemoveListener(
        mitk::MessageDelegate1<QmitkMIDASMultiViewVisibilityManager, const mitk::DataNode*>
    ( this, &QmitkMIDASMultiViewVisibilityManager::NodeRemovedProxy ));

    m_DataStorage = NULL;
  }
  this->RemoveAllFromObserverToVisibilityMap();
}

void QmitkMIDASMultiViewVisibilityManager::RemoveAllFromObserverToVisibilityMap()
{
  for( std::map<unsigned long, mitk::BaseProperty::Pointer>::iterator iter = m_ObserverToVisibilityMap.begin(); iter != m_ObserverToVisibilityMap.end(); ++iter )
  {
    (*iter).second->RemoveObserver((*iter).first);
  }
  m_ObserverToVisibilityMap.clear();
}

void QmitkMIDASMultiViewVisibilityManager::UpdateObserverToVisibilityMap()
{
  this->RemoveAllFromObserverToVisibilityMap();

  assert(m_DataStorage);

  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull() || it->Value()->GetProperty("visible") == NULL)
    {
      continue;
    }

    bool isHelper = false;
    it->Value()->GetBoolProperty("helper object", isHelper);

    if (isHelper)
    {
      continue;
    }

    /* register listener for changes in visible property */
    itk::ReceptorMemberCommand<QmitkMIDASMultiViewVisibilityManager>::Pointer command = itk::ReceptorMemberCommand<QmitkMIDASMultiViewVisibilityManager>::New();
    command->SetCallbackFunction(this, &QmitkMIDASMultiViewVisibilityManager::UpdateVisibilityProperty);
    m_ObserverToVisibilityMap[it->Value()->GetProperty("visible")->AddObserver( itk::ModifiedEvent(), command )] = it->Value()->GetProperty("visible");
  }
}

void QmitkMIDASMultiViewVisibilityManager::RegisterWidget(QmitkMIDASSingleViewWidget *widget)
{
  widget->SetDataStorage(m_DataStorage);

  std::set<mitk::DataNode*> newNodes;
  m_DataNodes.push_back(newNodes);
  m_Widgets.push_back(widget);
}

void QmitkMIDASMultiViewVisibilityManager::DeRegisterAllWidgets()
{
  this->DeRegisterWidgets(0, m_Widgets.size()-1);
}

void QmitkMIDASMultiViewVisibilityManager::DeRegisterWidgets(unsigned int startWindowIndex, unsigned int endWindowIndex)
{
  for (unsigned int i = startWindowIndex; i <= endWindowIndex; i++)
  {
    this->RemoveNodesFromWindow(i);
  }
  m_DataNodes.erase(m_DataNodes.begin() + startWindowIndex, m_DataNodes.begin() + endWindowIndex+1);
  m_Widgets.erase(m_Widgets.begin() + startWindowIndex, m_Widgets.begin() + endWindowIndex+1);
}

void QmitkMIDASMultiViewVisibilityManager::ClearAllWindows()
{
  this->ClearWindows(0, m_Widgets.size()-1);
}

void QmitkMIDASMultiViewVisibilityManager::ClearWindows(unsigned int startWindowIndex, unsigned int endWindowIndex)
{
  for (unsigned int i = startWindowIndex; i <= endWindowIndex; i++)
  {
    this->RemoveNodesFromWindow(i);
  }
}

void QmitkMIDASMultiViewVisibilityManager::SetAllNodeVisibilityForAllWindows(bool visibility)
{
  assert(m_DataStorage);

  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull() || it->Value()->GetProperty("visible") == NULL)
    {
      continue;
    }
    this->SetNodeVisibilityForAllWindows(it->Value(), visibility);
  }
}

void QmitkMIDASMultiViewVisibilityManager::SetNodeVisibilityForAllWindows(mitk::DataNode* node, bool visibility)
{
  for (unsigned int i = 0; i < m_Widgets.size(); i++)
  {
    this->SetNodeVisibilityForWindow(node, i, visibility);
  }
}


void QmitkMIDASMultiViewVisibilityManager::SetAllNodeVisibilityForWindow(unsigned int widgetIndex, bool visibility)
{
  assert(m_DataStorage);

  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    if (it->Value().IsNull() || it->Value()->GetProperty("visible") == NULL)
    {
      continue;
    }
    this->SetNodeVisibilityForWindow(it->Value(), widgetIndex, visibility);
  }
}

void QmitkMIDASMultiViewVisibilityManager::SetNodeVisibilityForWindow(mitk::DataNode* node, unsigned int widgetIndex, bool visibility)
{
  std::vector<mitk::DataNode*> nodes;
  nodes.push_back(node);
  m_Widgets[widgetIndex]->SetRendererSpecificVisibility(nodes, visibility);
}

int QmitkMIDASMultiViewVisibilityManager::GetIndexFromWindow(QmitkRenderWindow* renderWindow)
{
  int result = -1;

  for (unsigned int i = 0; i < m_Widgets.size(); i++)
  {
    bool contains = m_Widgets[i]->ContainsRenderWindow(renderWindow);
    if (contains)
    {
      result = i;
      break;
    }
  }
  return result;
}

void QmitkMIDASMultiViewVisibilityManager::NodeRemovedProxy( const mitk::DataNode* node )
{
  // Guarantee no recursions when a new node event is thrown in NodeRemoved()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeRemoved(node);
    m_InDataStorageChanged = false;
  }
}

void QmitkMIDASMultiViewVisibilityManager::NodeRemoved( const mitk::DataNode* node)
{
  this->UpdateObserverToVisibilityMap();

  for (unsigned int i = 0; i < m_DataNodes.size(); i++)
  {
    std::set<mitk::DataNode*>::iterator iter;
    iter = m_DataNodes[i].find(const_cast<mitk::DataNode*>(node));
    if (iter != m_DataNodes[i].end())
    {
      m_DataNodes[i].erase(iter);
    }
  }
}

void QmitkMIDASMultiViewVisibilityManager::NodeAddedProxy( const mitk::DataNode* node )
{
  // Guarantee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeAdded(node);
    m_InDataStorageChanged = false;
  }
}

void QmitkMIDASMultiViewVisibilityManager::NodeAdded( const mitk::DataNode* node)
{
  // TODO: Is there a way round this, because its ugly.
  // Basically, when drawing on an image, you interactively add/remove contours or seeds.
  // So, these objects are not "dropped" into a window, they are like overlays.
  // So, as soon as a drawing tool creates them, they must be visible.
  // Then they are removed from data storage when no longer needed.
  // So, for now, we just make sure they are not processed by this class.
  std::string name = node->GetName();
  if (   name.find("FeedbackContourTool") != std::string::npos
      || name.find("MIDASContourTool") != std::string::npos
      || name.find(mitk::MIDASTool::SEED_POINT_SET_NAME) != std::string::npos
      || name.find(mitk::MIDASTool::CURRENT_CONTOURS_NAME) != std::string::npos
      || name.find(mitk::MIDASTool::REGION_GROWING_IMAGE_NAME) != std::string::npos
      || name.find(mitk::MIDASTool::PRIOR_CONTOURS_NAME) != std::string::npos
      || name.find(mitk::MIDASTool::NEXT_CONTOURS_NAME) != std::string::npos
      || name.find(mitk::MIDASTool::DRAW_CONTOURS_NAME) != std::string::npos
      || name.find(mitk::MIDASTool::MORPH_EDITS_EROSIONS_SUBTRACTIONS) != std::string::npos
      || name.find(mitk::MIDASTool::MORPH_EDITS_EROSIONS_ADDITIONS) != std::string::npos
      || name.find(mitk::MIDASTool::MORPH_EDITS_DILATIONS_SUBTRACTIONS) != std::string::npos
      || name.find(mitk::MIDASTool::MORPH_EDITS_DILATIONS_ADDITIONS) != std::string::npos
      || name.find(mitk::MIDASPolyTool::MIDAS_POLY_TOOL_ANCHOR_POINTS) != std::string::npos
      || name.find(mitk::MIDASPolyTool::MIDAS_POLY_TOOL_PREVIOUS_CONTOUR) != std::string::npos
      || name.find("Paintbrush_Node") != std::string::npos
      )
  {
    return;
  }

  this->UpdateObserverToVisibilityMap();
  this->SetInitialNodeProperties(const_cast<mitk::DataNode*>(node));
}

void QmitkMIDASMultiViewVisibilityManager::SetInitialNodeProperties(mitk::DataNode* node)
{
  // So as each new node is added (i.e. surfaces, point sets, images) we set default visibility to false.
  this->SetNodeVisibilityForAllWindows(node, false);

  // Furthermore, if a node has a parent, and that parent is already visible, we add this new node to all the same
  // windows as its parent. This is useful in segmentation when we add a segmentation (binary) volume that is
  // registered as a child of a grey scale image. If the parent grey scale image is already
  // registered as visible in a window, then the child image is made visible, which has the effect of
  // immediately showing the segmented volume.
  mitk::DataNode::Pointer parent = mitk::FindParentGreyScaleImage(m_DataStorage, node);
  if (parent.IsNotNull())
  {
    for (unsigned int i = 0; i < m_DataNodes.size(); i++)
    {
      std::set<mitk::DataNode*>::iterator iter;
      for (iter = m_DataNodes[i].begin(); iter != m_DataNodes[i].end(); iter++)
      {
        if (*iter == parent)
        {
          bool globalVisibility = false;
          node->GetBoolProperty("visible", globalVisibility);
          this->AddNodeToWindow(i, node, globalVisibility);
        }
      }
    }
  }
}

void QmitkMIDASMultiViewVisibilityManager::UpdateVisibilityProperty(const itk::EventObject&)
{
  // We have to iterate through all nodes registered with DataStorage,
  // and see if the global visibility property should override the renderer specific one for any
  // node that is registered as having been dropped into and of our registered render windows.

  // Outer loop, iterates through all nodes in DataStorage.
  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator dataStorageIterator = all->Begin(); dataStorageIterator != all->End(); ++dataStorageIterator)
  {
    // Make sure each node is non-NULL and has a non-NULL "visible" property and is not a helper object
    if (dataStorageIterator->Value().IsNull() || dataStorageIterator->Value()->GetProperty("visible") == NULL)
    {
      continue;
    }

    bool isHelper = false;
    dataStorageIterator->Value()->GetBoolProperty("helper object", isHelper);

    if (isHelper)
    {
      continue;
    }

    // Then we iterate through our list of windows.
    for (unsigned int i = 0; i < m_DataNodes.size(); i++)
    {

      // And for each window, we have a set of registered nodes.
      std::set<mitk::DataNode*>::iterator nodesPerWindowIter;
      for (nodesPerWindowIter = m_DataNodes[i].begin(); nodesPerWindowIter != m_DataNodes[i].end(); nodesPerWindowIter++)
      {
        if (dataStorageIterator->Value() == (*nodesPerWindowIter))
        {
          bool globalVisibility(false);
          dataStorageIterator->Value()->GetBoolProperty("visible", globalVisibility);

          std::vector<mitk::DataNode*> nodes;
          nodes.push_back(dataStorageIterator->Value());

          m_Widgets[i]->SetRendererSpecificVisibility(nodes, globalVisibility);
        }
      }
    }
  }
}

void QmitkMIDASMultiViewVisibilityManager::RemoveNodesFromWindow(int windowIndex)
{
  QmitkMIDASSingleViewWidget *widget = m_Widgets[windowIndex];
  assert(widget);

  std::vector<mitk::DataNode*> nodes;
  std::set<mitk::DataNode*>::iterator iter;

  for (iter = m_DataNodes[windowIndex].begin(); iter != m_DataNodes[windowIndex].end(); iter++)
  {
    nodes.push_back(*iter);
  }

  widget->SetRendererSpecificVisibility(nodes, false);
  m_DataNodes[windowIndex].clear();
}

int QmitkMIDASMultiViewVisibilityManager::GetNodesInWindow(int windowIndex)
{
  int result = m_DataNodes[windowIndex].size();
  return result;
}

void QmitkMIDASMultiViewVisibilityManager::AddNodeToWindow(int windowIndex, mitk::DataNode* node, bool initialVisibility)
{
  QmitkMIDASSingleViewWidget *widget = m_Widgets[windowIndex];
  assert(widget);

  m_DataNodes[windowIndex].insert(node);
  node->Modified();

  std::vector<mitk::DataNode*> nodes;
  nodes.push_back(node);

  if (this->m_AutomaticallyAddChildren)
  {
    assert(m_DataStorage);

    mitk::DataStorage::SetOfObjects::Pointer possibleChildren = mitk::FindDerivedVisibleNonHelperChildren(m_DataStorage, node);
    for (unsigned int i = 0; i < possibleChildren->size(); i++)
    {
      mitk::DataNode* possibleNode = (*possibleChildren)[i];

      m_DataNodes[windowIndex].insert(possibleNode);
      possibleNode->Modified();

      nodes.push_back(possibleNode);
    }
  }

  widget->SetRendererSpecificVisibility(nodes, initialVisibility);

}

mitk::TimeSlicedGeometry::Pointer QmitkMIDASMultiViewVisibilityManager::GetGeometry(std::vector<mitk::DataNode*> nodes, int nodeIndex)
{
  mitk::TimeSlicedGeometry::Pointer geometry = NULL;
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
        geometry = image->GetTimeSlicedGeometry();
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
          geometry = data->GetTimeSlicedGeometry();
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
      geometry = data->GetTimeSlicedGeometry();
      indexThatWeActuallyUsed = nodeIndex;
    }
  }
  // Essentially, the nodeIndex is garbage, so just pick the first one.
  else
  {
    mitk::BaseData::Pointer data = nodes[0]->GetData();
    if (data.IsNotNull())
    {
      geometry = data->GetTimeSlicedGeometry();
      indexThatWeActuallyUsed = 0;
    }
  }

  // In addition, (as MIDAS is an image based viewer), if the node is NOT a greyscale image,
  // we try and search the parents of the node to find a greyscale image and use that one in preference.
  // This assumes that derived datasets, such as point sets, surfaces, segmented volumes are correctly assigned to parents.
  if (indexThatWeActuallyUsed != -1)
  {
    if (!mitk::IsNodeAGreyScaleImage(nodes[indexThatWeActuallyUsed]))
    {
      mitk::DataNode::Pointer node = FindParentGreyScaleImage(this->m_DataStorage, nodes[indexThatWeActuallyUsed]);
      if (node.IsNotNull())
      {
        mitk::BaseData::Pointer data = nodes[0]->GetData();
        if (data.IsNotNull())
        {
          geometry = data->GetTimeSlicedGeometry();
        }
      }
    }
  }
  return geometry;
}

template<typename TPixel, unsigned int VImageDimension>
void
QmitkMIDASMultiViewVisibilityManager::GetAsAcquiredOrientation(
    itk::Image<TPixel, VImageDimension>* itkImage,
    MIDASOrientation &outputOrientation
    )
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;

  typename itk::SpatialOrientationAdapter adaptor;
  typename itk::SpatialOrientation::ValidCoordinateOrientationFlags orientation;
  orientation = adaptor.FromDirectionCosines(itkImage->GetDirection());
  std::string orientationString = itk::ConvertSpatialOrientationToString(orientation);

  if (orientationString[0] == 'L' || orientationString[0] == 'R')
  {
    if (orientationString[1] == 'A' || orientationString[1] == 'P')
    {
      outputOrientation = MIDAS_ORIENTATION_AXIAL;
    }
    else
    {
      outputOrientation = MIDAS_ORIENTATION_CORONAL;
    }
  }
  else if (orientationString[0] == 'A' || orientationString[0] == 'P')
  {
    if (orientationString[1] == 'L' || orientationString[1] == 'R')
    {
      outputOrientation = MIDAS_ORIENTATION_AXIAL;
    }
    else
    {
      outputOrientation = MIDAS_ORIENTATION_SAGITTAL;
    }
  }
  else if (orientationString[0] == 'S' || orientationString[0] == 'I')
  {
    if (orientationString[1] == 'L' || orientationString[1] == 'R')
    {
      outputOrientation = MIDAS_ORIENTATION_CORONAL;
    }
    else
    {
      outputOrientation = MIDAS_ORIENTATION_SAGITTAL;
    }
  }
}

MIDASView QmitkMIDASMultiViewVisibilityManager::GetView(std::vector<mitk::DataNode*> nodes)
{

  MIDASView view = m_DefaultView;
  if (view == MIDAS_VIEW_AS_ACQUIRED)
  {
    // "As Acquired" means you take the orientation of the XY plane
    // in the original image data, so we switch to ITK to work it out.
    MIDASOrientation orientation = MIDAS_ORIENTATION_CORONAL;

    mitk::Image::Pointer image = NULL;
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
      image = dynamic_cast<mitk::Image*>(nodes[i]->GetData());
      if (image.IsNotNull())
      {
        break;
      }
    }
    if (image.IsNotNull() && image->GetDimension() >= 3)
    {
      try
      {
        AccessFixedDimensionByItk_n(image, GetAsAcquiredOrientation, 3, (orientation));
      }
      catch (const mitk::AccessByItkException &e)
      {
        MITK_ERROR << "QmitkMIDASMultiViewVisibilityManager::OnNodesDropped failed to work out 'As Acquired' orientation." << e.what() << std::endl;
      }
    }
    else
    {
      MITK_ERROR << "QmitkMIDASMultiViewVisibilityManager::OnNodesDropped failed to find an image to work out 'As Acquired' orientation." << std::endl;
    }

    if (orientation == MIDAS_ORIENTATION_AXIAL)
    {
      view = MIDAS_VIEW_AXIAL;
    }
    else if (orientation == MIDAS_ORIENTATION_SAGITTAL)
    {
      view = MIDAS_VIEW_SAGITTAL;
    }
    else if (orientation == MIDAS_ORIENTATION_CORONAL)
    {
      view = MIDAS_VIEW_CORONAL;
    }
    else
    {
      MITK_ERROR << "QmitkMIDASMultiViewVisibilityManager::OnNodesDropped defaulting to view=" << view << std::endl;
    }
  }
  return view;
}

void QmitkMIDASMultiViewVisibilityManager::OnNodesDropped(QmitkRenderWindow *window, std::vector<mitk::DataNode*> nodes)
{

  int windowIndex = this->GetIndexFromWindow(window);
  MIDASView view = this->GetView(nodes);

  if (m_DataStorage.IsNotNull() && windowIndex != -1)
  {
    for (unsigned int i = 0; i < nodes.size(); i++)
    {
      std::string name;
      if (nodes[i] != 0 && nodes[i]->GetStringProperty("name", name))
      {
        MITK_DEBUG << "Dropped " << nodes.size() << " into window[" << windowIndex <<"], name[" << i << "]=" << name << std::endl;
      }
    }

    if (m_DropType == MIDAS_DROP_TYPE_SINGLE)
    {

      MITK_DEBUG << "Dropped single" << std::endl;

      mitk::TimeSlicedGeometry::Pointer geometry = this->GetGeometry(nodes, -1);
      if (geometry.IsNull())
      {
        MITK_ERROR << "Error, dropping " << nodes.size() << " nodes into window " << windowIndex << ", could not find geometry which must be a programming bug." << std::endl;
        return;
      }

      // Clear all nodes from the single window denoted by windowIndex (the one that was dropped into).
      if (this->GetNodesInWindow(windowIndex) > 0 && !this->GetAccumulateWhenDropped())
      {
        this->RemoveNodesFromWindow(windowIndex);
      }

      // Then set up geometry of that single window.
      if (this->GetNodesInWindow(windowIndex) == 0 || !this->GetAccumulateWhenDropped())
      {
        this->m_Widgets[windowIndex]->SetGeometry(geometry.GetPointer());
        this->m_Widgets[windowIndex]->SetView(view, true);
        this->m_Widgets[windowIndex]->SetEnabled(true);
      }

      // Then add all nodes into the same window denoted by windowIndex (the one that was dropped into).
      for (unsigned int i = 0; i < nodes.size(); i++)
      {
        this->AddNodeToWindow(windowIndex, nodes[i]);
      }
    }
    else if (m_DropType == MIDAS_DROP_TYPE_MULTIPLE)
    {
      MITK_DEBUG << "Dropped multiple" << std::endl;

      // Work out which window we are actually dropping into.
      // We aim to put one object, in each of consecutive windows.
      // If we hit the end (of the 5x5=25 list), we go back to zero.

      unsigned int dropIndex = windowIndex;

      for (unsigned int i = 0; i < nodes.size(); i++)
      {
        while (dropIndex < m_Widgets.size() && !m_Widgets[dropIndex]->isVisible())
        {
          // i.e. if the window we are in, is not visible, keep looking
          dropIndex++;
        }
        if (dropIndex == m_Widgets.size())
        {
          // give up? Or we could go back to zero?
          dropIndex = 0;
        }

        mitk::TimeSlicedGeometry::Pointer geometry = this->GetGeometry(nodes, i);
        if (geometry.IsNull())
        {
          MITK_ERROR << "Error, dropping node " << i << ", from a list of " << nodes.size() << " nodes into window " << dropIndex << ", could not find geometry which must be a programming bug." << std::endl;
          return;
        }

        // So we are removing all images that are present from the window denoted by dropIndex,
        if (this->GetNodesInWindow(dropIndex) > 0 && !this->GetAccumulateWhenDropped())
        {
          this->RemoveNodesFromWindow(dropIndex);
        }

        // Initialise geometry according to first image
        if (this->GetNodesInWindow(dropIndex) == 0 || !this->GetAccumulateWhenDropped())
        {
          this->m_Widgets[dropIndex]->SetGeometry(geometry.GetPointer());
          this->m_Widgets[dropIndex]->SetView(view, true);
          this->m_Widgets[dropIndex]->SetEnabled(true);
        }

        // ...and then adding a single image to that window, denoted by dropIndex.
        this->AddNodeToWindow(dropIndex, nodes[i]);

        // We need to always increment by at least one window, or else infinite loop-a-rama.
        dropIndex++;
      }
    }
    else if (m_DropType == MIDAS_DROP_TYPE_ALL)
    {
      MITK_DEBUG << "Dropped thumbnail" << std::endl;

      mitk::TimeSlicedGeometry::Pointer geometry = this->GetGeometry(nodes, -1);
      if (geometry.IsNull())
      {
        MITK_ERROR << "Error, dropping " << nodes.size() << " nodes into window " << windowIndex << ", could not find geometry which must be a programming bug." << std::endl;
        return;
      }

      // Clear all nodes from every window.
      if (this->GetNodesInWindow(0) > 0 && !this->GetAccumulateWhenDropped())
      {
        this->ClearAllWindows();
      }

      // Note: Remember that we have view = axial, coronal, sagittal, 3D and ortho (+ others maybe)
      // So this thumbnail drop, has to switch to a single orientation. If the current default
      // view is not a single slice mode, we need to switch to one.
      MIDASOrientation orientation = MIDAS_ORIENTATION_UNKNOWN;
      switch(view)
      {
      case MIDAS_VIEW_AXIAL:
        orientation = MIDAS_ORIENTATION_AXIAL;
        break;
      case MIDAS_VIEW_SAGITTAL:
        orientation = MIDAS_ORIENTATION_SAGITTAL;
        break;
      case MIDAS_VIEW_CORONAL:
        orientation = MIDAS_ORIENTATION_CORONAL;
        break;
      default:
        orientation = MIDAS_ORIENTATION_AXIAL;
        view = MIDAS_VIEW_AXIAL;
        break;
      }

      // Then we need to check if the number of slices < the number of windows, if so, we just
      // spread the slices, one per window, until we run out of windows.
      //
      // If we have more slices than windows, we need to interpolate the number of slices.
      if (this->GetNodesInWindow(windowIndex) == 0 || !this->GetAccumulateWhenDropped())
      {
        m_Widgets[0]->SetGeometry(geometry.GetPointer());
        m_Widgets[0]->SetView(view, true);
      }

      unsigned int minSlice = m_Widgets[0]->GetMinSlice(orientation);
      unsigned int maxSlice = m_Widgets[0]->GetMaxSlice(orientation);
      unsigned int numberOfSlices = maxSlice - minSlice + 1;
      unsigned int windowsToUse = std::min((unsigned int)numberOfSlices, (unsigned int)m_Widgets.size());

      MITK_DEBUG << "Dropping thumbnail, minSlice=" << minSlice << ", maxSlice=" << maxSlice << ", numberOfSlices=" << numberOfSlices << ", windowsToUse=" << windowsToUse << std::endl;

      // Now decide how we calculate which window is showing which slice.
      if (numberOfSlices <= m_Widgets.size())
      {
        // In this method, we have less slices than windows, so we just spread them in increasing order.
        for (unsigned int i = 0; i < windowsToUse; i++)
        {
          if (this->GetNodesInWindow(i) == 0 || !this->GetAccumulateWhenDropped())
          {
            this->m_Widgets[i]->SetGeometry(geometry.GetPointer());
            this->m_Widgets[i]->SetView(view, true);
            this->m_Widgets[i]->SetEnabled(true);
          }
          this->m_Widgets[i]->SetSliceNumber(orientation, minSlice + i);
          this->m_Widgets[i]->FitToDisplay();
          MITK_DEBUG << "Dropping thumbnail, i=" << i << ", sliceNumber=" << minSlice + i << std::endl;
        }
      }
      else
      {
        // In this method, we have more slices than windows, so we spread them evenly over the max number of windows.
        for (unsigned int i = 0; i < windowsToUse; i++)
        {
          if (this->GetNodesInWindow(i) == 0 || !this->GetAccumulateWhenDropped())
          {
            this->m_Widgets[i]->SetGeometry(geometry.GetPointer());
            this->m_Widgets[i]->SetView(view, true);
            this->m_Widgets[i]->SetEnabled(true);
          }
          unsigned int minSlice = m_Widgets[i]->GetMinSlice(orientation);
          unsigned int maxSlice = m_Widgets[i]->GetMaxSlice(orientation);
          unsigned int numberOfEdgeSlicesToIgnore = numberOfSlices * 0.05; // ignore first and last 5 percent, as usually junk/blank.
          unsigned int remainingNumberOfSlices = numberOfSlices - (2 * numberOfEdgeSlicesToIgnore);
          float fraction = (float)i/(float)(m_Widgets.size());
          unsigned int chosenSlice = numberOfEdgeSlicesToIgnore + remainingNumberOfSlices*fraction;

          MITK_DEBUG << "Dropping thumbnail, i=" << i \
              << ", minSlice=" << minSlice \
              << ", maxSlice=" << maxSlice \
              << ", numberOfEdgeSlicesToIgnore=" << numberOfEdgeSlicesToIgnore \
              << ", remainingNumberOfSlices=" << remainingNumberOfSlices \
              << ", fraction=" << fraction \
              << ", chosenSlice=" << chosenSlice << std::endl;
          m_Widgets[i]->SetSliceNumber(orientation, chosenSlice);
          m_Widgets[i]->FitToDisplay();
        }
      } // end if (which method of spreading thumbnails)

      // Now add the nodes to the right number of Windows.
      for (unsigned int i = 0; i < windowsToUse; i++)
      {
        for (unsigned int j = 0; j < nodes.size(); j++)
        {
          this->AddNodeToWindow(i, nodes[j]);
        }
      }
    } // end if (which method of dropping)
  } // end if (we have valid input)
}
