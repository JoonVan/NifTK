/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#ifndef QmitkIGIDataSourceManager_h
#define QmitkIGIDataSourceManager_h

#include "niftkIGIGuiManagerExports.h"
#include "ui_QmitkIGIDataSourceManager.h"
#include <itkObject.h>
#include <QWidget>
#include <QList>
#include <QGridLayout>
#include <QTimer>
#include <QSet>
#include <QColor>
#include <QThread>
#include <QMap>
#include <QString>
#include <mitkDataStorage.h>
#include <mitkIGIDataSource.h>
#include <NiftyLinkSocketObject.h>
#include <QmitkIGIDataSource.h>
#include <igtlTimeStamp.h>
#include <map>

class QmitkIGIDataSourceManagerClearDownThread;
class QTimer;
class QGridLayout;
class QmitkIGIDataSourceGui;


/**
 * \class QmitkIGIDataSourceManager
 * \brief Class to manage a list of QmitkIGIDataSources (trackers, ultra-sound machines, video etc).
 *
 * This widget acts like a widget factory, setting up sources, instantiating
 * the appropriate GUI, and loading it into the grid layout owned by this widget.
 */
class NIFTKIGIGUIMANAGER_EXPORT QmitkIGIDataSourceManager : public QWidget, public Ui_QmitkIGIDataSourceManager, public itk::Object
{

  Q_OBJECT

public:

  friend class QmitkIGIDataSourceManagerClearDownThread;
  friend class QmitkIGIDataSourceManagerGuiUpdateThread;

  mitkClassMacro(QmitkIGIDataSourceManager, itk::Object);
  itkNewMacro(QmitkIGIDataSourceManager);

  static QString GetDefaultPath();
  static const QColor DEFAULT_ERROR_COLOUR;
  static const QColor DEFAULT_WARNING_COLOUR;
  static const QColor DEFAULT_OK_COLOUR;
  static const QColor DEFAULT_SUSPENDED_COLOUR;
  static const int    DEFAULT_FRAME_RATE;
  static const int    DEFAULT_CLEAR_RATE;
  static const int    DEFAULT_TIMING_TOLERANCE;
  static const bool   DEFAULT_SAVE_ON_RECEIPT;
  static const bool   DEFAULT_SAVE_IN_BACKGROUND;
  static const bool   DEFAULT_PICK_LATEST_DATA;
  static const char*  DEFAULT_RECORDINGDESTINATION_ENVIRONMENTVARIABLE;

  /**
   * \brief Creates the base class widgets, and connects signals and slots.
   */
  void setupUi(QWidget* parent);

  /**
   * \brief Set the Data Storage, and also sets it into any registered tools.
   * \param dataStorage An MITK DataStorage, which is set onto any registered tools.
   */
  void SetDataStorage(mitk::DataStorage* dataStorage);

  /**
   * \brief Get the Data Storage that this tool manager is currently connected to.
   */
  itkGetConstMacro(DataStorage, mitk::DataStorage*);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  void SetFramesPerSecond(const int& framesPerSecond);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  void SetClearDataRate(const int& numberOfSeconds);

  /**
   * \brief Called from the GUI to set the timing tolerance, in milliseconds, which is the time during which data is considered valid.
   */
  void SetTimingTolerance(const int& timingTolerance);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  void SetDirectoryPrefix(QString& directoryPrefix);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  void SetErrorColour(QColor &colour);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  void SetWarningColour(QColor &colour);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  void SetOKColour(QColor &colour);

  void SetSuspendedColour(QColor &colour);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  itkSetMacro(SaveOnReceipt, bool);

  /**
   * \brief Called from the GUI when the surgical guidance plugin preferences are modified.
   */
  itkSetMacro(SaveInBackground, bool);

  /**
   * \brief Sets all data sources to just pick the latest data from the queue.
   * This is useful if you are running multiple machines, and struggling to
   * synchronise clocks via NTP.
   */
  void SetPickLatestData(const bool& pickLatest);

  /**
   * Tries to parse the data source descriptor for directory-to-classname mappings.
   * @param filepath full qualified path to descriptor.cfg, e.g. "/home/jo/projectwork/2014-01-28-11-51-04-909/descriptor.cfg"
   * @returns a map with key = directory, value = classname
   * @throws std::exception if something goes wrong.
   * @warning This method does not check whether any class name is valid, i.e. whether that class has been compiled in!
   */
  static QMap<QString, QString> ParseDataSourceDescriptor(const QString& filepath);

signals:

  /**
   * \brief Emmitted as soon as the OnUpdateGui method has acquired a timestamp.
   */
  void UpdateGuiStart(igtlUint64 timeStamp);

  /**
   * \brief Emmitted when the OnUpdateGui method has asked each data source to update.
   */
  void UpdateGuiFinishedDataSources(igtlUint64 timeStamp);

  /**
   * \brief Emmitted when the OnUpdateGui method has updated a few widgets, and called the rendering manager.
   */
  void UpdateGuiFinishedFinishedRendering(igtlUint64 timeStamp);

  /**
   * \brief Emmitted when the OnUpdateGui method has finished, after the QCoreApplication::processEvents() has been called.
   */
  void UpdateGuiEnd(igtlUint64 timeStamp);

  void RecordingStarted(QString basedirectory);


public slots:
  /**
   * \brief Callback to start recording data.
   */
  void OnRecordStart();


protected:

  QmitkIGIDataSourceManager();
  virtual ~QmitkIGIDataSourceManager();

  QmitkIGIDataSourceManager(const QmitkIGIDataSourceManager&); // Purposefully not implemented.
  QmitkIGIDataSourceManager& operator=(const QmitkIGIDataSourceManager&); // Purposefully not implemented.

  bool eventFilter(QObject *obj,  QEvent *event );

private slots:

  /**
   * \brief Updates the whole rendered scene, based on the available messages.
   *
   * More specifically, this method is called on a timer, and determines the
   * effective refresh rate of the data storage, and hence of the screen,
   * and also the widgets of the QmitkDataSourceManager itself. This method
   * assumes that all the data sources are instantiated, and the right number
   * of rows exists in the table.
   */
  void OnUpdateGui();

  /**
   * \brief Works out the table row, then updates the fields in the GUI.
   *
   * In comparison with OnUpdateGui, this method is just for updating the table
   * of available sources. Importantly, there is a use case where we need to dynamically
   * add rows. When a tool (typically a networked tool) provides information that
   * there should be additional related sources, we have to dynamically create them.
   *
   * \see OnUpdateGui
   */
  void OnUpdateSourceView(const int& sourceIdentifier);

  /**
   * \brief Tells each data source to clean data, see mitk::IGIDataSource::CleanData().
   */
  void OnCleanData();

  /**
   * \brief Adds a data source to the table, using values from UI for sourcetype and portnumbeR
   */
  void OnAddSource();

  /**
   * \brief Adds a data source to the table.
   * \return the added tool's identifier
   */
  int AddSource(const mitk::IGIDataSource::SourceTypeEnum& sourcetype, int portnumber, NiftyLinkSocketObject* socket=NULL);

  /**
   * \brief Removes a data source from the table, and completely destroys it.
   */
  void OnRemoveSource();

  /**
   * \brief Callback to indicate that a cell has been double clicked, to launch that sources' GUI.
   */
  void OnCellDoubleClicked(int row, int column);

  /**
   * \brief Callback when combo box for data source type is changed, we enable/disable widgets accordingly.
   */
  void OnCurrentIndexChanged(int indexNumber);

  /**
   * \brief Callback to stop recording/playback data.
   */
  void OnStop();


  void OnPlayStart();

  void OnFreezeTableHeaderClicked(int section);

  void OnTimestampEditFinished();

  void AdvancePlaybackTime();

  void OnComputeStats();

private:

  mitk::DataStorage                        *m_DataStorage;
  QGridLayout                              *m_GridLayoutClientControls;
  QSet<int>                                 m_PortsInUse;
  std::vector<QmitkIGIDataSource::Pointer>  m_Sources;
  unsigned int                              m_NextSourceIdentifier;

  QColor                                    m_ErrorColour;
  QColor                                    m_WarningColour;
  QColor                                    m_OKColour;
  QColor                                    m_SuspendedColour;
  int                                       m_FrameRate;
  int                                       m_ClearDataRate;
  igtlUint64                                m_TimingTolerance;
  QString                                   m_DirectoryPrefix;
  bool                                      m_SaveOnReceipt;
  bool                                      m_SaveInBackground;
  bool                                      m_PickLatestData;
  QTimer                                   *m_GuiUpdateTimer;
  QTimer                                   *m_ClearDownTimer;

  // These all pertain to stats. These are being "investigated".
  // i.e. we don't know the functional requirements.
  // So lets "give it a go", and see what seems useful.
  QTimer                                   *m_StatsTimer;
  igtlUint64                                m_RequestedFrameRate;
  igtlUint64                                m_NumberOfTimesRenderingLoopCalled;
  igtlUint64                                m_NumberOfTimesRenderingIsActuallyCalled;
  igtl::TimeStamp::Pointer                  m_StatsTimerStart;
  igtl::TimeStamp::Pointer                  m_StatsTimerEnd;
  std::vector<double>                       m_ListRenderingTimes;
  std::vector<double>                       m_ListDataFetchTimes;
  std::vector<double>                       m_ListLagTimes;
  std::map<int, std::vector<double> >       m_MapLagTiming;

  // either real wallclock time or slider-determined playback time
  igtlUint64                                m_CurrentTime;

  // slider position is relative to this base value.
  // slider can only represent int values, but we need all 64 bit.
  igtlUint64                                m_PlaybackSliderBase;
  igtlUint64                                m_PlaybackSliderFactor;

  // This class now remembers the current GUI, and asks it to update
  // at the assigned frame rate.
  QmitkIGIDataSourceGui                    *m_CurrentSourceGUI;

  // used to decide whether to clean up signals in the destructor;
  bool                                      m_setupUiHasBeenCalled;

  /**
   * \brief Checks the m_SourceSelectComboBox to see if the currentIndex pertains to a port specific type.
   */
  bool IsPortSpecificType();

  /**
   * m_Sources is ordered, and MUST correspond to the order in the display QTableWidget,
   * so this returns the source number or -1 if not found.
   */
  int GetSourceNumberFromIdentifier(int sourceIdentifier);

  /**
   * m_Sources is ordered, and MUST correspond to the order in the display QTableWidget,
   * so this returns the identifier or -1 if not found.
   */
  int GetIdentifierFromSourceNumber(int sourceNumber);

  /**
   * \brief
   */
  void UpdateSourceView(const int& sourceIdentifier, bool instantiateRelatedSources);

  /**
   * \brief Called by UpdateSourceView to actually instantiate the extra rows needed dynamically.
   */
  void InstantiateRelatedSources(const int& rowNumber);

  /**
   * \brief Adds a message to the QmitkIGIDataSourceManager console.
   */
  void PrintStatusMessage(const QString& message) const;

  /**
   * \brief Deletes the current GUI widget.
   */
  void DeleteCurrentGuiWidget();

  /**
   * \brief Gets a suitable directory name from a prefix determined by preferences, and a date-time stamp.
   */
  QString GetDirectoryName();

}; // end class

#endif

