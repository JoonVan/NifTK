/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "QmitkCommonAppsApplicationPlugin.h"
#include "QmitkCommonAppsApplicationPreferencePage.h"
#include "QmitkNiftyViewApplicationPreferencePage.h"

#if (_MSC_VER == 1700)
// Visual Studio 2012 does not provide the std::isnan function in cmath but _isnan in float.h.
#include <float.h>
#else
#include <cmath>
#endif

#include <berryPlatform.h>
#include <berryIPreferencesService.h>

#include <mitkCoreServices.h>
#include <mitkIPropertyExtensions.h>
#include <mitkFloatPropertyExtension.h>
#include <mitkProperties.h>
#include <mitkVersion.h>
#include <mitkLogMacros.h>
#include <mitkDataStorage.h>
#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkGlobalInteraction.h>
#include <mitkImageAccessByItk.h>
#include <mitkRenderingModeProperty.h>
#include <niftkNamedLookupTableProperty.h>
#include <mitkExceptionMacro.h>
#include <itkStatisticsImageFilter.h>
#include <itkCommand.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

#include <service/cm/ctkConfigurationAdmin.h>
#include <service/cm/ctkConfiguration.h>

#include <usModule.h>
#include <usModuleRegistry.h>
#include <usModuleContext.h>
#include <usModuleInitialization.h>

#include <QFileInfo>
#include <QDateTime>
#include <QtPlugin>

#include <NifTKConfigure.h>
#include <mitkDataStorageUtils.h>


US_INITIALIZE_MODULE

QmitkCommonAppsApplicationPlugin* QmitkCommonAppsApplicationPlugin::s_Inst = 0;

//-----------------------------------------------------------------------------
QmitkCommonAppsApplicationPlugin::QmitkCommonAppsApplicationPlugin()
: m_Context(NULL)
, m_DataStorageServiceTracker(NULL)
, m_InDataStorageChanged(false)
{
  s_Inst = this;
}


//-----------------------------------------------------------------------------
QmitkCommonAppsApplicationPlugin::~QmitkCommonAppsApplicationPlugin()
{
}


//-----------------------------------------------------------------------------
QmitkCommonAppsApplicationPlugin* QmitkCommonAppsApplicationPlugin::GetDefault()
{
  return s_Inst;
}


//-----------------------------------------------------------------------------
ctkPluginContext* QmitkCommonAppsApplicationPlugin::GetPluginContext() const
{
  return m_Context;
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::SetPluginContext(ctkPluginContext* context)
{
  m_Context = context;
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::start(ctkPluginContext* context)
{
  berry::AbstractUICTKPlugin::start(context);
  this->SetPluginContext(context);
  this->RegisterQmitkCommonAppsExtensions();
  this->RegisterDataStorageListener();
  this->BlankDepartmentalLogo();

  // Get the MitkCore module context.
  us::ModuleContext* mitkCoreContext = us::ModuleRegistry::GetModule(1)->GetModuleContext();

  mitk::IPropertyExtensions* propertyExtensions = mitk::CoreServices::GetPropertyExtensions(mitkCoreContext);
  mitk::FloatPropertyExtension::Pointer opacityPropertyExtension = mitk::FloatPropertyExtension::New(0.0, 1.0);
  propertyExtensions->AddExtension("Image Rendering.Lowest Value Opacity", opacityPropertyExtension.GetPointer());
  propertyExtensions->AddExtension("Image Rendering.Highest Value Opacity", opacityPropertyExtension.GetPointer());
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::stop(ctkPluginContext* context)
{
  this->UnregisterDataStorageListener();
}


//-----------------------------------------------------------------------------
const mitk::DataStorage* QmitkCommonAppsApplicationPlugin::GetDataStorage()
{
  mitk::DataStorage::Pointer dataStorage = NULL;

  if (m_DataStorageServiceTracker != NULL)
  {
    mitk::IDataStorageService* dsService = m_DataStorageServiceTracker->getService();
    if (dsService != 0)
    {
      dataStorage = dsService->GetDataStorage()->GetDataStorage();
    }
  }

  return dataStorage;
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterDataStorageListener()
{
  m_DataStorageServiceTracker = new ctkServiceTracker<mitk::IDataStorageService*>(m_Context);
  m_DataStorageServiceTracker->open();

  this->GetDataStorage()->AddNodeEvent.AddListener
      ( mitk::MessageDelegate1<QmitkCommonAppsApplicationPlugin, const mitk::DataNode*>
        ( this, &QmitkCommonAppsApplicationPlugin::NodeAddedProxy ) );

  this->GetDataStorage()->RemoveNodeEvent.AddListener
      ( mitk::MessageDelegate1<QmitkCommonAppsApplicationPlugin, const mitk::DataNode*>
        ( this, &QmitkCommonAppsApplicationPlugin::NodeRemovedProxy ) );

}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::UnregisterDataStorageListener()
{
  if (m_DataStorageServiceTracker != NULL)
  {

    this->GetDataStorage()->AddNodeEvent.RemoveListener
        ( mitk::MessageDelegate1<QmitkCommonAppsApplicationPlugin, const mitk::DataNode*>
          ( this, &QmitkCommonAppsApplicationPlugin::NodeAddedProxy ) );

    this->GetDataStorage()->RemoveNodeEvent.RemoveListener
        ( mitk::MessageDelegate1<QmitkCommonAppsApplicationPlugin, const mitk::DataNode*>
          ( this, &QmitkCommonAppsApplicationPlugin::NodeRemovedProxy ) );

    m_DataStorageServiceTracker->close();
    delete m_DataStorageServiceTracker;
    m_DataStorageServiceTracker = NULL;
  }
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterHelpSystem()
{
  ctkServiceReference cmRef = m_Context->getServiceReference<ctkConfigurationAdmin>();
  ctkConfigurationAdmin* configAdmin = 0;
  if (cmRef)
  {
    configAdmin = m_Context->getService<ctkConfigurationAdmin>(cmRef);
  }

  // Use the CTK Configuration Admin service to configure the BlueBerry help system
  if (configAdmin)
  {
    ctkConfigurationPtr conf = configAdmin->getConfiguration("org.blueberry.services.help", QString());
    ctkDictionary helpProps;
    QString urlHomePage = this->GetHelpHomePageURL();
    helpProps.insert("homePage", urlHomePage);
    conf->update(helpProps);
    m_Context->ungetService(cmRef);
  }
  else
  {
    MITK_WARN << "Configuration Admin service unavailable, cannot set home page url.";
  }
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterQmitkCommonAppsExtensions()
{
  // Probably you want stuff in your application specific sub-class.
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::BlankDepartmentalLogo()
{
  // Blank the departmental logo for now.
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer logoPref = prefService->GetSystemPreferences()->Node("org.mitk.editors.stdmultiwidget");
  logoPref->Put("DepartmentLogo", "");
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::NodeAddedProxy(const mitk::DataNode *node)
{
  // guarantee no recursions when a new node event is thrown in NodeAdded()
  if (!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeAdded(node);
    m_InDataStorageChanged = false;
  }
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::NodeAdded(const mitk::DataNode *constNode)
{
  mitk::DataNode::Pointer node = const_cast<mitk::DataNode*>(constNode);
  this->RegisterInterpolationProperty("uk.ac.ucl.cmic.commonapps", node);
  this->RegisterBinaryImageProperties("uk.ac.ucl.cmic.commonapps", node);
  this->RegisterImageRenderingModeProperties("uk.ac.ucl.cmic.commonapps", node);
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::NodeRemovedProxy(const mitk::DataNode *node)
{
  // guarantee no recursions when a new node event is thrown in NodeRemoved()
  if (!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeRemoved(node);
    m_InDataStorageChanged = false;
  }
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::NodeRemoved(const mitk::DataNode *constNode)
{
  mitk::DataNode::Pointer node = const_cast<mitk::DataNode*>(constNode);

  // Removing observers on a node thats being deleted?

  if (mitk::IsNodeAGreyScaleImage(node))
  {
    std::map<mitk::DataNode*, unsigned long int>::iterator lowestIter;
    lowestIter = m_NodeToLowestOpacityObserverMap.find(node);

    std::map<mitk::DataNode*, unsigned long int>::iterator highestIter;
    highestIter = m_NodeToHighestOpacityObserverMap.find(node);

    if (lowestIter != m_NodeToLowestOpacityObserverMap.end())
    {
      if (highestIter != m_NodeToHighestOpacityObserverMap.end())
      {
        mitk::BaseProperty::Pointer lowestIsOpaqueProperty = node->GetProperty("Image Rendering.Lowest Value Opacity");
        lowestIsOpaqueProperty->RemoveObserver(lowestIter->second);

        mitk::BaseProperty::Pointer highestIsOpaqueProperty = node->GetProperty("Image Rendering.Highest Value Opacity");
        highestIsOpaqueProperty->RemoveObserver(highestIter->second);

        m_NodeToLowestOpacityObserverMap.erase(lowestIter->first);
        m_NodeToHighestOpacityObserverMap.erase(highestIter->first);
        m_PropertyToNodeMap.erase(lowestIsOpaqueProperty.GetPointer());
        m_PropertyToNodeMap.erase(highestIsOpaqueProperty.GetPointer());
      }
    }
  }
}


//-----------------------------------------------------------------------------
template<typename TPixel, unsigned int VImageDimension>
void
QmitkCommonAppsApplicationPlugin
::ITKGetStatistics(
    itk::Image<TPixel, VImageDimension> *itkImage,
    float &min,
    float &max,
    float &mean,
    float &stdDev)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::StatisticsImageFilter<ImageType> FilterType;

  typename FilterType::Pointer filter = FilterType::New();
  filter->SetInput(itkImage);
  filter->UpdateLargestPossibleRegion();
  min = filter->GetMinimum();
  max = filter->GetMaximum();
  mean = filter->GetMean();
  stdDev = filter->GetSigma();
}


//-----------------------------------------------------------------------------
berry::IPreferences::Pointer QmitkCommonAppsApplicationPlugin::GetPreferencesNode(const QString& preferencesNodeName)
{
  berry::IPreferences::Pointer result(NULL);

  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  if (prefService)
  {
    result = prefService->GetSystemPreferences()->Node(preferencesNodeName);
  }

  return result;
}

//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterLevelWindowProperty(
    const QString& preferencesNodeName, mitk::DataNode *node)
{
  if (mitk::IsNodeAGreyScaleImage(node))
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    berry::IPreferences::Pointer prefNode = this->GetPreferencesNode(preferencesNodeName);

    if (prefNode.IsNotNull() && image.IsNotNull())
    {
      int minRange = prefNode->GetDouble(QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_RANGE_LOWER_BOUND_NAME, 0);
      int maxRange = prefNode->GetDouble(QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_RANGE_UPPER_BOUND_NAME, 0);
      double percentageOfRange = prefNode->GetDouble(QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_PERCENTAGE_NAME, 50);
      QString initialisationMethod = prefNode->Get(QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_METHOD_NAME, QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_PERCENTAGE);

      float minDataLimit(0);
      float maxDataLimit(0);
      float meanData(0);
      float stdDevData(0);

      bool minDataLimitFound = node->GetFloatProperty("image data min", minDataLimit);
      bool maxDataLimitFound = node->GetFloatProperty("image data max", maxDataLimit);
      bool meanDataFound = node->GetFloatProperty("image data mean", meanData);
      bool stdDevDataFound = node->GetFloatProperty("image data std dev", stdDevData);

      if (!minDataLimitFound || !maxDataLimitFound || !meanDataFound || !stdDevDataFound)
      {
        // Provide some defaults.
        minDataLimit = 0;
        maxDataLimit = 255;
        meanData = 15;
        stdDevData = 22;

        // Given that the above values are initial defaults, they must be stored on image.
        node->SetFloatProperty("image data min", minDataLimit);
        node->SetFloatProperty("image data max", maxDataLimit);
        node->SetFloatProperty("image data mean", meanData);
        node->SetFloatProperty("image data std dev", stdDevData);

        // Working data.
        double windowMin = 0;
        double windowMax = 255;
        mitk::LevelWindow levelWindow;

        // We don't have a policy for non-scalar images.
        // For example, how do you default Window/Level for RGB, HSV?
        // So, this stuff below, only valid for scalar images.
        if (image->GetPixelType().GetNumberOfComponents() == 1)
        {
          try
          {
            if (image->GetDimension() == 2)
            {
              AccessFixedDimensionByItk_n(image,
                  ITKGetStatistics, 2,
                  (minDataLimit, maxDataLimit, meanData, stdDevData)
                );
            }
            else if (image->GetDimension() == 3)
            {
              AccessFixedDimensionByItk_n(image,
                  ITKGetStatistics, 3,
                  (minDataLimit, maxDataLimit, meanData, stdDevData)
                );
            }
            else if (image->GetDimension() == 4)
            {
              AccessFixedDimensionByItk_n(image,
                  ITKGetStatistics, 4,
                  (minDataLimit, maxDataLimit, meanData, stdDevData)
                );
            }
            node->SetFloatProperty("image data min", minDataLimit);
            node->SetFloatProperty("image data max", maxDataLimit);
            node->SetFloatProperty("image data mean", meanData);
            node->SetFloatProperty("image data std dev", stdDevData);
            windowMin = minDataLimit;
            windowMax = maxDataLimit;
          }
          catch(const mitk::AccessByItkException& e)
          {
            MITK_ERROR << "Caught exception during QmitkCommonAppsApplicationPlugin::RegisterLevelWindowProperty, so image statistics will be wrong." << e.what();
          }

#if (_MSC_VER == 1700)
          // Visual Studio 2012 does not provide the C++11 std::isnan function.
          if (_isnan(stdDevData))
#else
          if (std::isnan(stdDevData))
#endif
          {
            MITK_WARN << "The image has NaN values. Overriding window/level initialisation mode from MIDAS convention to the mode based on percentage of data range.";
            initialisationMethod = QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_PERCENTAGE;
          }

          // This image hasn't had the data members that this view needs (minDataLimit, maxDataLimit etc) initialized yet.
          // i.e. we haven't seen it before. So we have a choice of how to initialise the Level/Window.
          if (initialisationMethod == QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_MIDAS)
          {
            double centre = (minDataLimit + 4.51*stdDevData)/2.0;
            double width = 4.5*stdDevData;
            windowMin = centre - width/2.0;
            windowMax = centre + width/2.0;

            if (windowMin < minDataLimit)
            {
              windowMin = minDataLimit;
            }
            if (windowMax > maxDataLimit)
            {
              windowMax = maxDataLimit;
            }
          }
          else if (initialisationMethod == QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_PERCENTAGE)
          {
            windowMin = minDataLimit;
            windowMax = minDataLimit + (maxDataLimit - minDataLimit)*percentageOfRange/100.0;
          }
          else if (initialisationMethod == QmitkNiftyViewApplicationPreferencePage::IMAGE_INITIALISATION_RANGE)
          {
            windowMin = minRange; // ignores data completely.
            windowMax = maxRange; // ignores data completely.
          }
          else
          {
            // Do nothing, which means the MITK framework will pick one.
          }
        }
        else
        {
          MITK_WARN << "QmitkCommonAppsApplicationPlugin::RegisterLevelWindowProperty: Using default Window/Level properties. " << std::endl;
        }

        levelWindow.SetRangeMinMax(minDataLimit, maxDataLimit);
        levelWindow.SetWindowBounds(windowMin, windowMax);
        node->SetLevelWindow(levelWindow);

      } // end if we haven't retrieved the data from the node.
    } // end if have pref node
  } // end if node is grey image
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::OnLookupTablePropertyChanged(const itk::Object *object, const itk::EventObject & event)
{
  const mitk::BaseProperty* prop = dynamic_cast<const mitk::BaseProperty*>(object);
  if (prop != NULL)
  {
    std::map<mitk::BaseProperty*, mitk::DataNode*>::const_iterator iter;
    iter = m_PropertyToNodeMap.find(const_cast<mitk::BaseProperty*>(prop));
    if (iter != m_PropertyToNodeMap.end())
    {
      mitk::DataNode *node = iter->second;
      if (node != NULL && mitk::IsNodeAGreyScaleImage(node))
      {
        float lowestOpacity = 1;
        bool gotLowest = node->GetFloatProperty("Image Rendering.Lowest Value Opacity", lowestOpacity);

        float highestOpacity = 1;
        bool gotHighest = node->GetFloatProperty("Image Rendering.Highest Value Opacity", highestOpacity);

        std::string defaultName = "grey";
        bool gotIndex = node->GetStringProperty("LookupTableName", defaultName);

        QString lutName = QString::fromStdString(defaultName);

        if (gotLowest && gotHighest && gotIndex)
        {
          // Get LUT from Micro Service.
          QmitkLookupTableProviderService *lutService = this->GetLookupTableProvider();
          niftk::NamedLookupTableProperty::Pointer mitkLUTProperty = lutService->CreateLookupTableProperty(lutName, lowestOpacity, highestOpacity);
          node->SetProperty("LookupTable", mitkLUTProperty);
        }
      }
    }
  }
}


//-----------------------------------------------------------------------------
QmitkLookupTableProviderService* QmitkCommonAppsApplicationPlugin::GetLookupTableProvider()
{
  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceReference<QmitkLookupTableProviderService> ref = context->GetServiceReference<QmitkLookupTableProviderService>();
  QmitkLookupTableProviderService* lutService = context->GetService<QmitkLookupTableProviderService>(ref);

  if (lutService == NULL)
  {
    mitkThrow() << "Failed to find QmitkLookupTableProviderService." << std::endl;
  }

  return lutService;
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterImageRenderingModeProperties(const QString& preferencesNodeName, mitk::DataNode *node)
{
  if (mitk::IsNodeAGreyScaleImage(node))
  {
    berry::IPreferences::Pointer prefNode = this->GetPreferencesNode(preferencesNodeName);
    if (prefNode.IsNotNull())
    {
      float lowestOpacity = prefNode->GetFloat(QmitkCommonAppsApplicationPreferencePage::LOWEST_VALUE_OPACITY, 1);
      float highestOpacity = prefNode->GetFloat(QmitkCommonAppsApplicationPreferencePage::HIGHEST_VALUE_OPACITY, 1);

      mitk::BaseProperty::Pointer lutProp = node->GetProperty("LookupTable");
      const niftk::NamedLookupTableProperty* prop = dynamic_cast<const niftk::NamedLookupTableProperty*>(lutProp.GetPointer());
      if(prop == NULL )
      {
        QString defaultName = "grey";

        // Get LUT from Micro Service.
        QmitkLookupTableProviderService *lutService = this->GetLookupTableProvider();
        niftk::NamedLookupTableProperty::Pointer mitkLUTProperty = lutService->CreateLookupTableProperty(defaultName, lowestOpacity, highestOpacity);

        node->ReplaceProperty("LookupTable", mitkLUTProperty);
        node->SetStringProperty("LookupTableName", defaultName.toStdString().c_str());
        node->SetProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR));
      }

      node->SetProperty("Image Rendering.Lowest Value Opacity", mitk::FloatProperty::New(lowestOpacity));
      node->SetProperty("Image Rendering.Highest Value Opacity", mitk::FloatProperty::New(highestOpacity));

      if (mitk::IsNodeAGreyScaleImage(node))
      {
        unsigned long int observerId;

        itk::MemberCommand<QmitkCommonAppsApplicationPlugin>::Pointer lowestIsOpaqueCommand = itk::MemberCommand<QmitkCommonAppsApplicationPlugin>::New();
        lowestIsOpaqueCommand->SetCallbackFunction(this, &QmitkCommonAppsApplicationPlugin::OnLookupTablePropertyChanged);
        mitk::BaseProperty::Pointer lowestIsOpaqueProperty = node->GetProperty("Image Rendering.Lowest Value Opacity");
        observerId = lowestIsOpaqueProperty->AddObserver(itk::ModifiedEvent(), lowestIsOpaqueCommand);
        m_PropertyToNodeMap.insert(std::pair<mitk::BaseProperty*, mitk::DataNode*>(lowestIsOpaqueProperty.GetPointer(), node));
        m_NodeToLowestOpacityObserverMap.insert(std::pair<mitk::DataNode*, unsigned long int>(node, observerId));

        itk::MemberCommand<QmitkCommonAppsApplicationPlugin>::Pointer highestIsOpaqueCommand = itk::MemberCommand<QmitkCommonAppsApplicationPlugin>::New();
        highestIsOpaqueCommand->SetCallbackFunction(this, &QmitkCommonAppsApplicationPlugin::OnLookupTablePropertyChanged);
        mitk::BaseProperty::Pointer highestIsOpaqueProperty = node->GetProperty("Image Rendering.Highest Value Opacity");
        observerId = highestIsOpaqueProperty->AddObserver(itk::ModifiedEvent(), highestIsOpaqueCommand);
        m_PropertyToNodeMap.insert(std::pair<mitk::BaseProperty*, mitk::DataNode*>(highestIsOpaqueProperty.GetPointer(), node));
        m_NodeToHighestOpacityObserverMap.insert(std::pair<mitk::DataNode*, unsigned long int>(node, observerId));
      }
    } // end if have pref node
  } // end if node is grey image
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterInterpolationProperty(
    const QString& preferencesNodeName, mitk::DataNode *node)
{
  if (mitk::IsNodeAGreyScaleImage(node))
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    berry::IPreferences::Pointer prefNode = this->GetPreferencesNode(preferencesNodeName);

    if (prefNode.IsNotNull() && image.IsNotNull())
    {

      int imageResliceInterpolation =  prefNode->GetInt(QmitkCommonAppsApplicationPreferencePage::IMAGE_RESLICE_INTERPOLATION, 2);
      int imageTextureInterpolation =  prefNode->GetInt(QmitkCommonAppsApplicationPreferencePage::IMAGE_TEXTURE_INTERPOLATION, 2);

      mitk::BaseProperty::Pointer mitkLUT = node->GetProperty("LookupTable");
      if (mitkLUT.IsNotNull())
      {
        niftk::LabeledLookupTableProperty::Pointer labelProperty 
          = dynamic_cast<niftk::LabeledLookupTableProperty*>(mitkLUT.GetPointer());

        if (labelProperty.IsNotNull() && labelProperty->GetIsScaled())
        {
          imageResliceInterpolation = prefNode->GetInt(QmitkCommonAppsApplicationPreferencePage::IMAGE_RESLICE_INTERPOLATION, 0);
          imageTextureInterpolation = prefNode->GetInt(QmitkCommonAppsApplicationPreferencePage::IMAGE_RESLICE_INTERPOLATION, 0);
        }
      }

      if (imageTextureInterpolation == 0)
      {
        node->SetProperty("texture interpolation", mitk::BoolProperty::New(false));
      }
      else
      {
        node->SetProperty("texture interpolation", mitk::BoolProperty::New(true));
      }

      mitk::VtkResliceInterpolationProperty::Pointer interpolationProperty = mitk::VtkResliceInterpolationProperty::New();

      if (imageResliceInterpolation == 0)
      {
        interpolationProperty->SetInterpolationToNearest();
      }
      else if (imageResliceInterpolation == 1)
      {
        interpolationProperty->SetInterpolationToLinear();
      }
      else if (imageResliceInterpolation == 2)
      {
        interpolationProperty->SetInterpolationToCubic();
      }
      node->SetProperty("reslice interpolation", interpolationProperty);

    } // end if have pref node
  } // end if node is grey image
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::RegisterBinaryImageProperties(const QString& preferencesNodeName, mitk::DataNode *node)
{
  if (mitk::IsNodeABinaryImage(node))
  {
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
    berry::IPreferences::Pointer prefNode = this->GetPreferencesNode(preferencesNodeName);

    if (prefNode.IsNotNull() && image.IsNotNull())
    {
      double defaultBinaryOpacity = prefNode->GetDouble(QmitkCommonAppsApplicationPreferencePage::BINARY_OPACITY_NAME, QmitkCommonAppsApplicationPreferencePage::BINARY_OPACITY_VALUE);
      node->SetOpacity(defaultBinaryOpacity);
      node->SetBoolProperty("outline binary", true);
    } // end if have pref node
  } // end if node is binary image
}


//-----------------------------------------------------------------------------
void QmitkCommonAppsApplicationPlugin::SetFileOpenTriggersReinit(bool openEditor)
{
  // Blank the departmental logo for now.
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();

  berry::IPreferences::Pointer generalPrefs = prefService->GetSystemPreferences()->Node("/General");
  generalPrefs->PutBool("OpenEditor", openEditor);
}


//-----------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  Q_EXPORT_PLUGIN2(uk_ac_ucl_cmic_commonapps, QmitkCommonAppsApplicationPlugin)
#endif