/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkMultiViewerVisibilityManager.h"
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkImageAccessByItk.h>
#include <itkConversionUtils.h>
#include <itkSpatialOrientationAdapter.h>

#include <niftkDataStorageUtils.h>

#include "niftkSingleViewerWidget.h"

namespace niftk
{

//-----------------------------------------------------------------------------
MultiViewerVisibilityManager::MultiViewerVisibilityManager(mitk::DataStorage::Pointer dataStorage)
  : niftk::DataNodePropertyListener(dataStorage, "visible"),
    m_SelectedViewer(nullptr),
    m_DropType(DNDDISPLAY_DROP_SINGLE),
    m_DefaultWindowLayout(WINDOW_LAYOUT_CORONAL),
    m_InterpolationType(DNDDISPLAY_CUBIC_INTERPOLATION),
    m_AutomaticallyAddChildren(true),
    m_Accumulate(false)
{
}


//-----------------------------------------------------------------------------
MultiViewerVisibilityManager::~MultiViewerVisibilityManager()
{
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::RegisterViewer(SingleViewerWidget* viewer)
{
  std::set<mitk::DataNode*> newNodes;
  m_DataNodesPerViewer.push_back(newNodes);
  m_Viewers.push_back(viewer);

  std::size_t viewerIndex = m_Viewers.size() - 1;

  std::vector<mitk::DataNode*> nodes;

  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetDataStorage()->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    /// We set the renderer specific visibility of the nodes that have global visibility property.
    /// (Regardless if they are visible globally or not.)
    if (it->Value()->GetProperty("visible"))
    {
      nodes.push_back(it->Value());
    }
  }

  m_Viewers[viewerIndex]->SetVisibility(nodes, false);

  this->connect(viewer, SIGNAL(NodesDropped(std::vector<mitk::DataNode*>)), SLOT(OnNodesDropped(std::vector<mitk::DataNode*>)));
  this->connect(viewer, SIGNAL(WindowSelected()), SLOT(OnWindowSelected()));
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::DeregisterViewers(std::size_t startIndex, std::size_t endIndex)
{
  if (endIndex == -1)
  {
    endIndex = m_Viewers.size();
  }
  for (std::size_t i = startIndex; i < endIndex; ++i)
  {
    QObject::disconnect(m_Viewers[i], SIGNAL(NodesDropped(std::vector<mitk::DataNode*>)), this, SLOT(OnNodesDropped(std::vector<mitk::DataNode*>)));
    QObject::disconnect(m_Viewers[i], SIGNAL(WindowSelected()), this, SLOT(OnWindowSelected()));
    this->RemoveNodesFromViewer(i);
  }
  m_DataNodesPerViewer.erase(m_DataNodesPerViewer.begin() + startIndex, m_DataNodesPerViewer.begin() + endIndex);
  m_Viewers.erase(m_Viewers.begin() + startIndex, m_Viewers.begin() + endIndex);
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::ClearViewers(std::size_t startIndex, std::size_t endIndex)
{
  if (endIndex == -1)
  {
    endIndex = m_Viewers.size();
  }
  for (std::size_t i = startIndex; i < endIndex; i++)
  {
    this->RemoveNodesFromViewer(i);
  }
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::OnNodeAdded(mitk::DataNode* node)
{
  // So as each new node is added (i.e. surfaces, point sets, images) we set default visibility to false.
  for (std::size_t viewerIndex = 0; viewerIndex < m_Viewers.size(); ++viewerIndex)
  {
    /// Note:
    /// Do not manage the visibility of the crosshair planes.
    if (!node->GetProperty("renderer"))
    {
      std::vector<mitk::DataNode*> nodes;
      nodes.push_back(node);
      m_Viewers[viewerIndex]->SetVisibility(nodes, false);
    }
  }

  mitk::VtkResliceInterpolationProperty* interpolationProperty =
      dynamic_cast<mitk::VtkResliceInterpolationProperty*>(node->GetProperty("reslice interpolation"));
  if (interpolationProperty)
  {
    if (m_InterpolationType == DNDDISPLAY_NO_INTERPOLATION)
    {
      interpolationProperty->SetInterpolationToNearest();
    }
    else if (m_InterpolationType == DNDDISPLAY_LINEAR_INTERPOLATION)
    {
      interpolationProperty->SetInterpolationToLinear();
    }
    else if (m_InterpolationType == DNDDISPLAY_CUBIC_INTERPOLATION)
    {
      interpolationProperty->SetInterpolationToCubic();
    }
  }

  // Furthermore, if a node has a parent, and that parent is already visible, we add this new node to all the same
  // viewer as its parent. This is useful in segmentation when we add a segmentation (binary) volume that is
  // registered as a child of a grey scale image. If the parent grey scale image is already
  // registered as visible in a viewer, then the child image is made visible, which has the effect of
  // immediately showing the segmented volume.
  mitk::DataNode::Pointer parent = niftk::FindParentGreyScaleImage(this->GetDataStorage(), node);
  if (parent.IsNotNull())
  {
    for (std::size_t i = 0; i < m_DataNodesPerViewer.size(); i++)
    {
      std::set<mitk::DataNode*>::iterator it = m_DataNodesPerViewer[i].find(parent);
      if (it != m_DataNodesPerViewer[i].end())
      {
        this->AddNodeToViewer(i, node);
      }
    }
  }
  else
  {
    /// TODO This should not be handled here.
    if (node->GetName() == std::string("One of FeedbackContourTool's feedback nodes"))
    {
      for (std::size_t viewerIndex = 0; viewerIndex < m_Viewers.size(); ++viewerIndex)
      {
        if (m_Viewers[viewerIndex]->IsFocused())
        {
          this->AddNodeToViewer(viewerIndex, node);
        }
      }
    }
  }

  Superclass::OnNodeAdded(node);
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::OnNodeRemoved(mitk::DataNode* node)
{
  Superclass::OnNodeRemoved(node);

  // This is just to trigger updating the intensity annotations.
  for (std::size_t viewerIndex = 0; viewerIndex < m_Viewers.size(); ++viewerIndex)
  {
    if (m_Viewers[viewerIndex]->IsFocused())
    {
      std::vector<mitk::DataNode*> nodes;
      nodes.push_back(node);
      m_Viewers[viewerIndex]->SetVisibility(nodes, false);
    }
  }

  for (std::size_t i = 0; i < m_DataNodesPerViewer.size(); i++)
  {
    std::set<mitk::DataNode*>::iterator it;
    it = m_DataNodesPerViewer[i].find(const_cast<mitk::DataNode*>(node));
    if (it != m_DataNodesPerViewer[i].end())
    {
      m_DataNodesPerViewer[i].erase(it);
    }
  }
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::OnPropertyChanged(mitk::DataNode* node, const mitk::BaseRenderer* renderer)
{
  /// Note:
  /// The renderer must be 0 because we are listening to the global visibility only.

  for (std::size_t viewerIndex = 0; viewerIndex < m_Viewers.size(); ++viewerIndex)
  {
    if (m_Viewers[viewerIndex]->IsFocused())
    {
      bool globalVisibility = node->IsVisible(0);

      std::set<mitk::DataNode*>::iterator nodesBegin = m_DataNodesPerViewer[viewerIndex].begin();
      std::set<mitk::DataNode*>::iterator nodesEnd = m_DataNodesPerViewer[viewerIndex].end();
      std::set<mitk::DataNode*>::iterator it = std::find(nodesBegin, nodesEnd, node);
      if (it != nodesEnd)
      {
        std::vector<mitk::DataNode*> nodes;
        nodes.push_back(node);
        m_Viewers[viewerIndex]->SetVisibility(nodes, globalVisibility);

//        if (!globalVisibility)
//        {
//          if (m_DataNodesPerViewer[viewerIndex].size() > 1)
//          {
//            std::vector<mitk::DataNode*> newNodes(m_DataNodesPerViewer[viewerIndex].size() - 1);
//            std::vector<mitk::DataNode*>::iterator newNodesIt = std::copy(nodesBegin, it, newNodes.begin());
//            ++it;
//            std::copy(it, nodesEnd, newNodesIt);
//            m_Viewers[viewerIndex]->OnNodesDropped(newNodes);
//          }
//          else
//          {
//            this->RemoveNodesFromViewer(viewerIndex);
//          }
//        }
      }
//      else
//      {
//        if (globalVisibility)
//        {
//          std::vector<mitk::DataNode*> newNodes(m_DataNodesPerViewer[viewerIndex].size() + 1);
//          std::copy(nodesBegin, nodesEnd, newNodes.begin());
//          newNodes[newNodes.size() - 1] = node;
//          m_Viewers[viewerIndex]->OnNodesDropped(newNodes);
//        }
//      }

      /// Only one viewer can be focused at a time.
      break;
    }
  }
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::RemoveNodesFromViewer(int viewerIndex)
{
  SingleViewerWidget *viewer = m_Viewers[viewerIndex];
  assert(viewer);

  std::vector<mitk::DataNode*> nodes;
  std::set<mitk::DataNode*>::iterator iter;

  for (iter = m_DataNodesPerViewer[viewerIndex].begin(); iter != m_DataNodesPerViewer[viewerIndex].end(); iter++)
  {
    nodes.push_back(*iter);
  }

  viewer->SetVisibility(nodes, false);
  m_DataNodesPerViewer[viewerIndex].clear();
}


//-----------------------------------------------------------------------------
int MultiViewerVisibilityManager::GetNodesInViewer(int viewerIndex)
{
  return m_DataNodesPerViewer[viewerIndex].size();
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::AddNodeToViewer(int viewerIndex, mitk::DataNode* node)
{
  SingleViewerWidget* viewer = m_Viewers[viewerIndex];
  assert(viewer);

  m_DataNodesPerViewer[viewerIndex].insert(node);
  node->Modified();

  std::vector<mitk::DataNode*> nodes;
  nodes.push_back(node);

  if (m_AutomaticallyAddChildren)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer possibleChildren = this->GetDataStorage()->GetDerivations(node, NULL, false);
    for (std::size_t i = 0; i < possibleChildren->size(); i++)
    {
      mitk::DataNode* possibleNode = (*possibleChildren)[i];
//      if (possibleNode->IsVisible(0))
      {
        m_DataNodesPerViewer[viewerIndex].insert(possibleNode);
        possibleNode->Modified();

        nodes.push_back(possibleNode);
      }
    }
  }

  viewer->ApplyGlobalVisibility(nodes);
}


//-----------------------------------------------------------------------------
mitk::TimeGeometry::Pointer MultiViewerVisibilityManager::GetTimeGeometry(std::vector<mitk::DataNode*> nodes, int nodeIndex)
{
  mitk::TimeGeometry::Pointer geometry = NULL;
  int indexThatWeActuallyUsed = -1;

  // If nodeIndex < 0, we are choosing the best geometry from all available nodes.
  if (nodeIndex < 0)
  {

    // First try to find an image geometry, and if so, use the first one.
    mitk::Image::Pointer image = NULL;
    for (std::size_t i = 0; i < nodes.size(); i++)
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
      for (std::size_t i = 0; i < nodes.size(); i++)
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
    if (!niftk::IsNodeAGreyScaleImage(nodes[indexThatWeActuallyUsed]))
    {
      mitk::DataNode::Pointer node = FindParentGreyScaleImage(this->GetDataStorage(), nodes[indexThatWeActuallyUsed]);
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
template<typename TPixel, unsigned int VImageDimension>
void
MultiViewerVisibilityManager::GetAsAcquiredOrientation(itk::Image<TPixel, VImageDimension>* itkImage, WindowOrientation& outputOrientation)
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
      outputOrientation = WINDOW_ORIENTATION_AXIAL;
    }
    else
    {
      outputOrientation = WINDOW_ORIENTATION_CORONAL;
    }
  }
  else if (orientationString[0] == 'A' || orientationString[0] == 'P')
  {
    if (orientationString[1] == 'L' || orientationString[1] == 'R')
    {
      outputOrientation = WINDOW_ORIENTATION_AXIAL;
    }
    else
    {
      outputOrientation = WINDOW_ORIENTATION_SAGITTAL;
    }
  }
  else if (orientationString[0] == 'S' || orientationString[0] == 'I')
  {
    if (orientationString[1] == 'L' || orientationString[1] == 'R')
    {
      outputOrientation = WINDOW_ORIENTATION_CORONAL;
    }
    else
    {
      outputOrientation = WINDOW_ORIENTATION_SAGITTAL;
    }
  }
}


//-----------------------------------------------------------------------------
WindowLayout MultiViewerVisibilityManager::GetWindowLayout(std::vector<mitk::DataNode*> nodes)
{

  WindowLayout windowLayout = m_DefaultWindowLayout;
  if (windowLayout == WINDOW_LAYOUT_AS_ACQUIRED)
  {
    // "As Acquired" means you take the orientation of the XY plane
    // in the original image data, so we switch to ITK to work it out.
    WindowOrientation orientation = WINDOW_ORIENTATION_CORONAL;

    mitk::Image::Pointer image = NULL;
    for (std::size_t i = 0; i < nodes.size(); i++)
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
        MITK_ERROR << "MultiViewerVisibilityManager::GetWindowLayout() failed to work out 'As Acquired' orientation." << e.what() << std::endl;
      }
    }
    else
    {
      MITK_ERROR << "MultiViewerVisibilityManager::GetWindowLayout() failed to find an image to work out 'As Acquired' orientation." << std::endl;
    }

    if (orientation == WINDOW_ORIENTATION_AXIAL)
    {
      windowLayout = WINDOW_LAYOUT_AXIAL;
    }
    else if (orientation == WINDOW_ORIENTATION_SAGITTAL)
    {
      windowLayout = WINDOW_LAYOUT_SAGITTAL;
    }
    else if (orientation == WINDOW_ORIENTATION_CORONAL)
    {
      windowLayout = WINDOW_LAYOUT_CORONAL;
    }
    else
    {
      MITK_ERROR << "MultiViewerVisibilityManager::GetWindowLayout() defaulting to window layout " << windowLayout << std::endl;
    }
  }
  return windowLayout;
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::OnWindowSelected()
{
  SingleViewerWidget* selectedViewer = qobject_cast<SingleViewerWidget*>(QObject::sender());

//  if (selectedViewer != m_SelectedViewer)
  {
    m_SelectedViewer = selectedViewer;
    if (selectedViewer->GetTimeGeometry() != nullptr)
    {
      this->UpdateGlobalVisibilities(selectedViewer->GetSelectedRenderWindow()->GetRenderer());
    }
  }
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::UpdateGlobalVisibilities(mitk::BaseRenderer* renderer)
{
  bool wasBlocked = this->SetBlocked(true);
  mitk::DataStorage::SetOfObjects::ConstPointer all = this->GetDataStorage()->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
  {
    /// We set the global visibility of the nodes that the same as the renderer specific visibility.
    mitk::DataNode::Pointer node = it->Value();
    if (!node->GetProperty("renderer"))
    {
      bool rendererSpecificVisibility = node->IsVisible(renderer);
      node->SetVisibility(rendererSpecificVisibility, 0);
    }
  }
  this->SetBlocked(wasBlocked);
}


//-----------------------------------------------------------------------------
void MultiViewerVisibilityManager::OnNodesDropped(std::vector<mitk::DataNode*> nodes)
{
  SingleViewerWidget* viewer = qobject_cast<SingleViewerWidget*>(this->sender());

  int viewerIndex = std::find(m_Viewers.begin(), m_Viewers.end(), viewer) - m_Viewers.begin();
  WindowLayout windowLayout = this->GetWindowLayout(nodes);

  if (viewerIndex != m_Viewers.size())
  {
    for (std::size_t i = 0; i < nodes.size(); i++)
    {
      std::string name;
      if (nodes[i] != 0 && nodes[i]->GetStringProperty("name", name))
      {
        MITK_DEBUG << "Dropped " << nodes.size() << " into viewer[" << viewerIndex <<"], name[" << i << "]=" << name << std::endl;
      }
    }

    if (m_DropType == DNDDISPLAY_DROP_SINGLE)
    {

      MITK_DEBUG << "Dropped single" << std::endl;

      mitk::TimeGeometry::Pointer timeGeometry = this->GetTimeGeometry(nodes, -1);
      if (timeGeometry.IsNull())
      {
        MITK_ERROR << "Error, dropping " << nodes.size() << " nodes into viewer " << viewerIndex << ", could not find geometry which must be a programming bug." << std::endl;
        return;
      }

      // Clear all nodes from the single viewer denoted by viewerIndex (the one that was dropped into).
      if (this->GetNodesInViewer(viewerIndex) > 0 && !this->GetAccumulateWhenDropped())
      {
        this->RemoveNodesFromViewer(viewerIndex);
      }

      // Then set up geometry of that single viewer.
      if (this->GetNodesInViewer(viewerIndex) == 0 || !this->GetAccumulateWhenDropped())
      {
        m_Viewers[viewerIndex]->SetTimeGeometry(timeGeometry.GetPointer());
        m_Viewers[viewerIndex]->SetWindowLayout(windowLayout);
        m_Viewers[viewerIndex]->SetEnabled(true);
      }

      // Then add all nodes into the same viewer denoted by viewerIndex (the one that was dropped into).
      for (std::size_t i = 0; i < nodes.size(); i++)
      {
        this->AddNodeToViewer(viewerIndex, nodes[i]);
      }
    }
    else if (m_DropType == DNDDISPLAY_DROP_MULTIPLE)
    {
      MITK_DEBUG << "Dropped multiple" << std::endl;

      // Work out which viewer we are actually dropping into.
      // We aim to put one object, in each of consecutive viewers.
      // If we hit the end (of the 5x5=25 list), we go back to zero.

      std::size_t dropIndex = viewerIndex;

      for (std::size_t i = 0; i < nodes.size(); i++)
      {
        while (dropIndex < m_Viewers.size() && !m_Viewers[dropIndex]->isVisible())
        {
          // i.e. if the viewer we are in, is not visible, keep looking
          dropIndex++;
        }
        if (dropIndex == m_Viewers.size())
        {
          // give up? Or we could go back to zero?
          dropIndex = 0;
        }

        mitk::TimeGeometry::Pointer timeGeometry = this->GetTimeGeometry(nodes, i);
        if (timeGeometry.IsNull())
        {
          MITK_ERROR << "Error, dropping node " << i << ", from a list of " << nodes.size() << " nodes into viewer " << dropIndex << ", could not find geometry which must be a programming bug." << std::endl;
          return;
        }

        // So we are removing all images that are present from the viewer denoted by dropIndex,
        if (this->GetNodesInViewer(dropIndex) > 0 && !this->GetAccumulateWhenDropped())
        {
          this->RemoveNodesFromViewer(dropIndex);
        }

        // Initialise geometry according to first image
        if (this->GetNodesInViewer(dropIndex) == 0 || !this->GetAccumulateWhenDropped())
        {
          m_Viewers[dropIndex]->SetTimeGeometry(timeGeometry.GetPointer());
          m_Viewers[dropIndex]->SetWindowLayout(windowLayout);
          m_Viewers[dropIndex]->SetEnabled(true);
        }

        // ...and then adding a single image to that viewer, denoted by dropIndex.
        this->AddNodeToViewer(dropIndex, nodes[i]);

        // We need to always increment by at least one viewer, or else infinite loop-a-rama.
        dropIndex++;
      }
    }
    else if (m_DropType == DNDDISPLAY_DROP_ALL)
    {
      MITK_DEBUG << "Dropped thumbnail" << std::endl;

      mitk::TimeGeometry::Pointer timeGeometry = this->GetTimeGeometry(nodes, -1);
      if (timeGeometry.IsNull())
      {
        MITK_ERROR << "Error, dropping " << nodes.size() << " nodes into viewer " << viewerIndex << ", could not find geometry which must be a programming bug." << std::endl;
        return;
      }

      // Clear all nodes from every viewer.
      if (this->GetNodesInViewer(0) > 0 && !this->GetAccumulateWhenDropped())
      {
        this->ClearViewers();
      }

      // Note: Remember that we have window layout = axial, coronal, sagittal, 3D and ortho (+ others maybe)
      // So this thumbnail drop, has to switch to a single orientation. If the current default
      // window layout is not a single slice mode, we need to switch to one.
      WindowOrientation orientation = WINDOW_ORIENTATION_UNKNOWN;
      switch (windowLayout)
      {
      case WINDOW_LAYOUT_AXIAL:
        orientation = WINDOW_ORIENTATION_AXIAL;
        break;
      case WINDOW_LAYOUT_SAGITTAL:
        orientation = WINDOW_ORIENTATION_SAGITTAL;
        break;
      case WINDOW_LAYOUT_CORONAL:
        orientation = WINDOW_ORIENTATION_CORONAL;
        break;
      default:
        orientation = WINDOW_ORIENTATION_AXIAL;
        windowLayout = WINDOW_LAYOUT_AXIAL;
        break;
      }

      // Then we need to check if the number of slices < the number of viewers, if so, we just
      // spread the slices, one per viewer, until we run out of viewers.
      //
      // If we have more slices than viewers, we need to interpolate the number of slices.
      if (this->GetNodesInViewer(viewerIndex) == 0 || !this->GetAccumulateWhenDropped())
      {
        m_Viewers[0]->SetTimeGeometry(timeGeometry.GetPointer());
        m_Viewers[0]->SetWindowLayout(windowLayout);
      }

      int maxSlice = m_Viewers[0]->GetMaxSlice(orientation);
      int numberOfSlices = maxSlice + 1;
      std::size_t viewersToUse = std::min((std::size_t)numberOfSlices, (std::size_t)m_Viewers.size());

      MITK_DEBUG << "Dropping thumbnail, maxSlice=" << maxSlice << ", numberOfSlices=" << numberOfSlices << ", viewersToUse=" << viewersToUse << std::endl;

      // Now decide how we calculate which viewer is showing which slice.
      if (numberOfSlices <= m_Viewers.size())
      {
        // In this method, we have less slices than viewers, so we just spread them in increasing order.
        for (std::size_t i = 0; i < viewersToUse; i++)
        {
          if (this->GetNodesInViewer(i) == 0 || !this->GetAccumulateWhenDropped())
          {
            m_Viewers[i]->SetTimeGeometry(timeGeometry.GetPointer());
            m_Viewers[i]->SetWindowLayout(windowLayout);
            m_Viewers[i]->SetEnabled(true);
          }
          m_Viewers[i]->SetSelectedSlice(orientation, i);
          m_Viewers[i]->FitToDisplay();
          MITK_DEBUG << "Dropping thumbnail, slice=" << i << std::endl;
        }
      }
      else
      {
        // In this method, we have more slices than viewers, so we spread them evenly over the max number of viewers.
        for (std::size_t i = 0; i < viewersToUse; i++)
        {
          if (this->GetNodesInViewer(i) == 0 || !this->GetAccumulateWhenDropped())
          {
            m_Viewers[i]->SetTimeGeometry(timeGeometry.GetPointer());
            m_Viewers[i]->SetWindowLayout(windowLayout);
            m_Viewers[i]->SetEnabled(true);
          }
          int maxSlice = m_Viewers[i]->GetMaxSlice(orientation);
          int numberOfEdgeSlicesToIgnore = static_cast<int>(numberOfSlices * 0.05); // ignore first and last 5 percent, as usually junk/blank.
          int remainingNumberOfSlices = numberOfSlices - (2 * numberOfEdgeSlicesToIgnore);
          float fraction = static_cast<float>(i) / m_Viewers.size();

          int chosenSlice = numberOfEdgeSlicesToIgnore + static_cast<int>(remainingNumberOfSlices * fraction);

          MITK_DEBUG << "Dropping thumbnail, i=" << i \
              << ", maxSlice=" << maxSlice \
              << ", numberOfEdgeSlicesToIgnore=" << numberOfEdgeSlicesToIgnore \
              << ", remainingNumberOfSlices=" << remainingNumberOfSlices \
              << ", fraction=" << fraction \
              << ", chosenSlice=" << chosenSlice << std::endl;
          m_Viewers[i]->SetSelectedSlice(orientation, chosenSlice);
          m_Viewers[i]->FitToDisplay();
        }
      } // end if (which method of spreading thumbnails)

      // Now add the nodes to the right number of viewers.
      for (std::size_t i = 0; i < viewersToUse; i++)
      {
        for (std::size_t j = 0; j < nodes.size(); j++)
        {
          this->AddNodeToViewer(i, nodes[j]);
        }
      }
    } // end if (which method of dropping)
  } // end if (we have valid input)

  this->UpdateGlobalVisibilities(viewer->GetSelectedRenderWindow()->GetRenderer());
}

}
