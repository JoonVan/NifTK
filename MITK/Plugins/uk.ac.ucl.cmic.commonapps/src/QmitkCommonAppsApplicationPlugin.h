/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkCommonAppsApplicationPlugin_h
#define QmitkCommonAppsApplicationPlugin_h

#include <uk_ac_ucl_cmic_commonapps_Export.h>
#include <ctkServiceTracker.h>
#include <berryAbstractUICTKPlugin.h>
#include <berryIPreferences.h>
#include <mitkIDataStorageService.h>
#include <QmitkLookupTableProviderService.h>

#include <itkImage.h>

#include <QObject>
#include <QString>

namespace mitk {
  class DataNode;
  class DataStorage;
}

/**
 * \class QmitkCommonAppsApplicationPlugin
 * \brief Abstract class that implements QT and CTK specific functionality to launch the application as a plugin.
 * \ingroup uk_ac_ucl_cmic_commonapps_internal
 */
class CMIC_QT_COMMONAPPS QmitkCommonAppsApplicationPlugin : public berry::AbstractUICTKPlugin
{
  Q_OBJECT
  Q_INTERFACES(ctkPluginActivator)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  Q_PLUGIN_METADATA(IID "uk_ac_ucl_cmic_commonapps")
#endif

public:

  QmitkCommonAppsApplicationPlugin();
  virtual ~QmitkCommonAppsApplicationPlugin();

  static QmitkCommonAppsApplicationPlugin* GetDefault();
  ctkPluginContext* GetPluginContext() const;

  virtual void start(ctkPluginContext* context) override;
  virtual void stop(ctkPluginContext* context) override;

protected:

  /**
   * \brief Called when the user toggles the opacity control properties.
   */
  virtual void OnLookupTablePropertyChanged(const itk::Object *caller, const itk::EventObject &event);

  /// \brief Deliberately not virtual method that enables derived classes to set the plugin context, and should be called from within the plugin start method.
  void SetPluginContext(ctkPluginContext*);

  /// \brief Deliberately not virtual method that connects this class to DataStorage so that we can receive NodeAdded events etc.
  void RegisterDataStorageListener();

  /// \brief Deliberately not virtual method that ddisconnects this class from DataStorage so that we can receive NodeAdded events etc.
  void UnregisterDataStorageListener();

  /// \brief Deliberately not virtual method thats called by derived classes within the start method to set up the help system.
  void RegisterHelpSystem();

  /// \brief Deliberately not virtual method thats called by derived classes, to register an initial LevelWindow property to each image.
  void RegisterLevelWindowProperty(const QString& preferencesNodeName, mitk::DataNode *constNode);

  /// \brief Deliberately not virtual method thats called by derived classes, to register an initial "Image Rendering.Mode" property to each image.
  void RegisterImageRenderingModeProperties(const QString& preferencesNodeName, mitk::DataNode *constNode);

  /// \brief Deliberately not virtual method thats called by derived classes, to register an initial value for Texture Interpolation, and Reslice Interpolation.
  void RegisterInterpolationProperty(const QString& preferencesNodeName, mitk::DataNode *constNode);

  /// \brief Deliberately not virtual method that registers initial property values of "outline binary"=true and "opacity"=1 for binary images.
  void RegisterBinaryImageProperties(const QString& preferencesNodeName, mitk::DataNode *constNode);

  /// \brief Deliberately not virtual method thats called by derived classes, to set the departmental logo to blank.
  void BlankDepartmentalLogo();

  /// \brief Called each time a data node is added, and derived classes can override it.
  virtual void NodeAdded(const mitk::DataNode *node);

  /// \brief Called each time a data node is removed, and derived classes can override it.
  virtual void NodeRemoved(const mitk::DataNode *node);

  /// \brief Derived classes should provide a URL for which help page to use as the 'home' page.
  virtual QString GetHelpHomePageURL() const { return QString(); }

  // \brief Sets a preference whether to reinitialise the rendering manager after opening a file.
  // It is suggested to set this to 'false' with the DnD display.
  void SetFileOpenTriggersReinit(bool openEditor);

private:

  /// \brief Parses a node property value that was specified on the command line.
  QVariant ParsePropertyValue(const QString& propertyValue);

  /// \brief Sets node properties from a map of QVariant values per renderer name per property name.
  void SetNodeProperty(mitk::DataNode* node, const QString& propertyName, const QVariant& propertyValue, const QString& rendererName = QString());

  /// \brief Private method that checks whether or not we are already updating and if not, calls NodeAdded()
  void NodeAddedProxy(const mitk::DataNode *node);

  /// \brief Private method that checks whether or not we are already removing and if not, calls NodeRemoved()
  void NodeRemovedProxy(const mitk::DataNode *node);

  /// \brief Returns the lookup table provider service.
  QmitkLookupTableProviderService* GetLookupTableProvider();

  /// \brief Private method that retrieves the DataStorage from the m_DataStorageServiceTracker
  const mitk::DataStorage* GetDataStorage();

  /// \brief Retrieves the preferences node name, or Null if unsuccessful.
  berry::IPreferences::Pointer GetPreferencesNode(const QString& preferencesNodeName);

  /// \brief Private utility method to calculate min, max, mean and stdDev of an ITK image.
  template<typename TPixel, unsigned int VImageDimension>
  void
  ITKGetStatistics(
      const itk::Image<TPixel, VImageDimension> *itkImage,
      float &min,
      float &max,
      float &mean,
      float &stdDev);


  void LoadDataFromDisk(const QStringList& args, bool globalReinit);
  void startNewInstance(const QStringList& args, const QStringList &files);

private Q_SLOTS:

  void handleIPCMessage(const QByteArray &msg);

private:

  ctkPluginContext* m_Context;
  ctkServiceTracker<mitk::IDataStorageService*>* m_DataStorageServiceTracker;
  bool m_InDataStorageChanged;
  static QmitkCommonAppsApplicationPlugin* s_Inst;

  std::map<mitk::BaseProperty*, mitk::DataNode*> m_PropertyToNodeMap;
  std::map<mitk::DataNode*, unsigned long int>   m_NodeToLowestOpacityObserverMap;
  std::map<mitk::DataNode*, unsigned long int>   m_NodeToHighestOpacityObserverMap;
};

#endif /* QMITKCOMMONAPPSAPPLICATIONPLUGIN_H_ */
