/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "mitkMIDASTool.h"

#include <itkCommand.h>

#include <mitkDisplayInteractor.h>
#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include <mitkToolManager.h>

// MicroServices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleRegistry.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

#include "mitkMIDASEventFilter.h"

const std::string mitk::MIDASTool::SEEDS_NAME = "MIDAS_SEEDS";
const std::string mitk::MIDASTool::CONTOURS_NAME = "MIDAS_CURRENT_CONTOURS";
const std::string mitk::MIDASTool::PRIOR_CONTOURS_NAME = "MIDAS_PRIOR_CONTOURS";
const std::string mitk::MIDASTool::NEXT_CONTOURS_NAME = "MIDAS_NEXT_CONTOURS";
const std::string mitk::MIDASTool::DRAW_CONTOURS_NAME = "MIDAS_DRAW_CONTOURS";
const std::string mitk::MIDASTool::REGION_GROWING_NAME = "MIDAS_REGION_GROWING_IMAGE";
const std::string mitk::MIDASTool::INITIAL_SEGMENTATION_NAME = "MIDAS_INITIAL_SEGMENTATION_IMAGE";
const std::string mitk::MIDASTool::INITIAL_SEEDS_NAME = "MIDAS_INITIAL_SEEDS";

//-----------------------------------------------------------------------------
bool mitk::MIDASTool::s_BehaviourStringsLoaded = false;


//-----------------------------------------------------------------------------
mitk::MIDASTool::MIDASTool()
: mitk::FeedbackContourTool("")
, m_AddToPointSetInteractor(NULL)
, m_LastSeenNumberOfSeeds(0)
, m_SeedsChangedTag(0)
, m_IsActivated(false)
, m_BlockNumberOfSeedsSignal(false)
{
}


//-----------------------------------------------------------------------------
mitk::MIDASTool::~MIDASTool()
{
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::LoadBehaviourStrings()
{
  if (!s_BehaviourStringsLoaded)
  {
    us::Module* thisModule = us::GetModuleContext()->GetModule();

    if (Self::LoadBehaviour("MIDASToolPointSetInteractor.xml", thisModule)
        && Self::LoadBehaviour("MIDASSeedToolPointSetInteractor.xml", thisModule))
    {
      s_BehaviourStringsLoaded = true;
    }
    else
    {
      MITK_ERROR << "State machine factory is not initialised. Use QmitkRegisterClasses().";
    }
  }
}


bool mitk::MIDASTool::LoadBehaviour(const std::string& fileName, us::Module* module)
{
  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetInstance();
  mitk::StateMachineFactory* stateMachineFactory = globalInteraction->GetStateMachineFactory();
  if (stateMachineFactory)
  {
    us::ModuleResource resource =  module->GetResource("Interactions/" + fileName);
    if (!resource.IsValid())
    {
      mitkThrow() << ("Resource not valid. State machine pattern not found:" + fileName);
    }
    us::ModuleResourceStream stream(resource);

    std::istreambuf_iterator<char> eos;
    std::string behaviourString(std::istreambuf_iterator<char>(stream), eos);

    return stateMachineFactory->LoadBehaviorString(behaviourString);
  }
  else
  {
    MITK_ERROR << "State machine factory is not initialised. Use QmitkRegisterClasses().";
    return false;
  }
}


//-----------------------------------------------------------------------------
bool mitk::MIDASTool::FilterEvents(mitk::InteractionEvent* event, mitk::DataNode* dataNode)
{
  bool canHandleIt = MIDASStateMachine::CanHandleEvent(event);
  if (canHandleIt)
  {
    m_LastEventSender = event->GetSender();
  }
  return canHandleIt;
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::InstallEventFilter(mitk::MIDASEventFilter* eventFilter)
{
  mitk::MIDASStateMachine::InstallEventFilter(eventFilter);
  if (m_AddToPointSetInteractor.IsNotNull())
  {
    m_AddToPointSetInteractor->InstallEventFilter(eventFilter);
  }
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::RemoveEventFilter(mitk::MIDASEventFilter* eventFilter)
{
  if (m_AddToPointSetInteractor.IsNotNull())
  {
    m_AddToPointSetInteractor->RemoveEventFilter(eventFilter);
  }
  mitk::MIDASStateMachine::RemoveEventFilter(eventFilter);
}


//-----------------------------------------------------------------------------
const char* mitk::MIDASTool::GetGroup() const
{
  return "MIDAS";
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::Activated()
{
  Superclass::Activated();

  mitk::DataNode* pointSetNode = nullptr;
  mitk::PointSet* pointSet = nullptr;

  // Get the current segmented volume
  mitk::DataNode::Pointer segmentationNode = m_ToolManager->GetWorkingData(SEGMENTATION);

  // Get reference to point set (Seeds). Here we look at derived nodes, that are called SEED_POINT_SET_NAME
  if (segmentationNode.IsNotNull())
  {
    // Find children of the segmented image that are point sets and called POINT_SET_NAME.
    mitk::TNodePredicateDataType<mitk::PointSet>::Pointer isPointSet = mitk::TNodePredicateDataType<mitk::PointSet>::New();
    mitk::DataStorage::SetOfObjects::ConstPointer possibleChildren = m_ToolManager->GetDataStorage()->GetDerivations( segmentationNode, isPointSet );

    if (possibleChildren->size() > 0)
    {
      for(unsigned int i = 0; i < possibleChildren->size(); ++i)
      {
        if ((*possibleChildren)[i]->GetName() == SEEDS_NAME)
        {
          pointSetNode = (*possibleChildren)[i];
          pointSet = dynamic_cast<mitk::PointSet*>((*possibleChildren)[i]->GetData());
          break;
        }
      }
    } // end if children point sets exist
  } // end if working data exists


  // Additionally create an interactor to add points to the point set.
  if (pointSetNode && pointSet)
  {
    m_PointSetNode = pointSetNode;
    m_PointSet = pointSet;

    if (m_AddToPointSetInteractor.IsNull())
    {
      m_AddToPointSetInteractor = mitk::MIDASPointSetInteractor::New("MIDASToolPointSetInteractor", pointSetNode);

//      m_AddToPointSetInteractor = mitk::MIDASPointSetDataInteractor::New();
//      m_AddToPointSetInteractor->LoadStateMachine("MIDASToolPointSetDataInteractor.xml", us::GetModuleContext()->GetModule());
//      m_AddToPointSetInteractor->SetEventConfig("MIDASToolPointSetDataInteractorConfig.xml", us::GetModuleContext()->GetModule());

      std::vector<mitk::MIDASEventFilter*> eventFilters = this->GetEventFilters();
      std::vector<mitk::MIDASEventFilter*>::const_iterator it = eventFilters.begin();
      std::vector<mitk::MIDASEventFilter*>::const_iterator itEnd = eventFilters.end();
      for ( ; it != itEnd; ++it)
      {
        m_AddToPointSetInteractor->InstallEventFilter(*it);
      }

//      m_AddToPointSetInteractor->SetDataNode(m_PointSetNode);

      mitk::GlobalInteraction::GetInstance()->AddInteractor( m_AddToPointSetInteractor );
    }

    m_LastSeenNumberOfSeeds = m_PointSet->GetSize();

    itk::SimpleMemberCommand<mitk::MIDASTool>::Pointer onSeedsModifiedCommand =
      itk::SimpleMemberCommand<mitk::MIDASTool>::New();
    onSeedsModifiedCommand->SetCallbackFunction( this, &mitk::MIDASTool::OnSeedsModified );
    m_SeedsChangedTag = pointSet->AddObserver(itk::ModifiedEvent(), onSeedsModifiedCommand);
  }

  // As a legacy solution the display interaction of the new interaction framework is disabled here  to avoid conflicts with tools
  // Note: this only affects InteractionEventObservers (formerly known as Listeners) all DataNode specific interaction will still be enabled
  m_DisplayInteractorConfigs.clear();
  std::vector<us::ServiceReference<InteractionEventObserver> > listEventObserver = us::GetModuleContext()->GetServiceReferences<InteractionEventObserver>();
  for (std::vector<us::ServiceReference<InteractionEventObserver> >::iterator it = listEventObserver.begin(); it != listEventObserver.end(); ++it)
  {
    mitk::DisplayInteractor* displayInteractor = dynamic_cast<mitk::DisplayInteractor*>(
                                                    us::GetModuleContext()->GetService<InteractionEventObserver>(*it));
    if (displayInteractor)
    {
      if (std::strcmp(displayInteractor->GetNameOfClass(), "DnDDisplayInteractor") == 0)
      {
        // remember the original configuration
        m_DisplayInteractorConfigs.insert(std::make_pair(*it, displayInteractor->GetEventConfig()));
        // here the alternative configuration is loaded
        displayInteractor->SetEventConfig("DnDDisplayConfigMIDASTool.xml", us::GetModuleContext()->GetModule());
      }
    }
  }

  m_IsActivated = true;
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::Deactivated()
{
  m_IsActivated = false;

  // Re-enabling InteractionEventObservers that have been previously disabled for legacy handling of Tools
  // in new interaction framework
  for (std::map<us::ServiceReferenceU, mitk::EventConfig>::iterator it = m_DisplayInteractorConfigs.begin();
       it != m_DisplayInteractorConfigs.end(); ++it)
  {
    if (it->first)
    {
      mitk::DisplayInteractor* displayInteractor = static_cast<mitk::DisplayInteractor*>(
                                               us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(it->first));
      if (displayInteractor)
      {
        if (std::strcmp(displayInteractor->GetNameOfClass(), "DnDDisplayInteractor") == 0)
        {
          // here the regular configuration is loaded again
          displayInteractor->SetEventConfig(it->second);
        }
      }
    }
  }

  m_DisplayInteractorConfigs.clear();

  if (m_PointSet.IsNotNull())
  {
    m_PointSetNode = nullptr;
    m_PointSet->RemoveObserver(m_SeedsChangedTag);
    m_PointSet = nullptr;

    if (m_AddToPointSetInteractor.IsNotNull())
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_AddToPointSetInteractor);

      /// Note:
      /// The interactor is disabled after it is destructed, therefore we have to make sure
      /// that we remove every reference to it. The data node also has a reference to it,
      /// therefore we have to decouple them here.
      /// If we do not do this, the interactor stays active and will keep processing the events.
//      m_AddToPointSetInteractor->SetDataNode(0);

      std::vector<mitk::MIDASEventFilter*> eventFilters = this->GetEventFilters();
      std::vector<mitk::MIDASEventFilter*>::const_iterator it = eventFilters.begin();
      std::vector<mitk::MIDASEventFilter*>::const_iterator itEnd = eventFilters.end();
      for ( ; it != itEnd; ++it)
      {
        m_AddToPointSetInteractor->RemoveEventFilter(*it);
      }

      m_AddToPointSetInteractor = NULL;
    }
  }

  Superclass::Deactivated();
}


//-----------------------------------------------------------------------------
bool mitk::MIDASTool::GetBlockNumberOfSeedsSignal() const
{
  return m_BlockNumberOfSeedsSignal;
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::SetBlockNumberOfSeedsSignal(bool blockNumberOfSeedsSignal)
{
  m_BlockNumberOfSeedsSignal = blockNumberOfSeedsSignal;
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::RenderCurrentWindow(const PositionEvent& positionEvent)
{
  assert( positionEvent.GetSender()->GetRenderWindow() );
  positionEvent.GetSender()->RequestUpdate();
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::RenderAllWindows()
{
  if (m_LastEventSender)
  {
    m_LastEventSender->GetRenderingManager()->RequestUpdateAll();
  }
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::UpdateWorkingDataNodeBoolProperty(int dataIndex, const std::string& name, bool value)
{
  assert(m_ToolManager);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(dataIndex);
  assert(workingNode);

  workingNode->SetBoolProperty(name.c_str(), value);
}


//-----------------------------------------------------------------------------
mitk::DataNode::Pointer mitk::MIDASTool::GetPointSetNode() const
{
  return m_PointSetNode;
}


//-----------------------------------------------------------------------------
mitk::PointSet::Pointer mitk::MIDASTool::GetPointSet() const
{
  return m_PointSet;
}


//-----------------------------------------------------------------------------
void mitk::MIDASTool::OnSeedsModified()
{
  if (m_PointSet.IsNotNull())
  {
    if (m_PointSet->GetSize() != m_LastSeenNumberOfSeeds)
    {
      m_LastSeenNumberOfSeeds = m_PointSet->GetSize();

      if (!m_BlockNumberOfSeedsSignal)
      {
        NumberOfSeedsHasChanged.Send(m_LastSeenNumberOfSeeds);
      }
    }
  }
}
