/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkMultiViewerWidgetTest.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include <mitkGlobalInteraction.h>
#include <mitkIOUtil.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestingMacros.h>

#include <QmitkMimeTypes.h>
#include <QmitkRegisterClasses.h>

#include <niftkCoreObjectFactory.h>
#include <niftkSingleViewerWidget.h>
#include <niftkMultiViewerWidget.h>
#include <niftkMultiViewerVisibilityManager.h>


namespace niftk
{

class MultiViewerWidgetTestClassPrivate
{
public:
  std::string FileName;
  mitk::DataStorage::Pointer DataStorage;
  mitk::RenderingManager::Pointer RenderingManager;

  mitk::DataNode::Pointer ImageNode;

  MultiViewerWidget* MultiViewer;
  MultiViewerVisibilityManager::Pointer VisibilityManager;

  bool InteractiveMode;
};


// --------------------------------------------------------------------------
MultiViewerWidgetTestClass::MultiViewerWidgetTestClass()
: QObject()
, d_ptr(new MultiViewerWidgetTestClassPrivate())
{
  Q_D(MultiViewerWidgetTestClass);

  d->MultiViewer = nullptr;
  d->InteractiveMode = false;
}


// --------------------------------------------------------------------------
MultiViewerWidgetTestClass::~MultiViewerWidgetTestClass()
{
}


// --------------------------------------------------------------------------
std::string MultiViewerWidgetTestClass::GetFileName() const
{
  Q_D(const MultiViewerWidgetTestClass);
  return d->FileName;
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::SetFileName(const std::string& fileName)
{
  Q_D(MultiViewerWidgetTestClass);
  d->FileName = fileName;
}


// --------------------------------------------------------------------------
bool MultiViewerWidgetTestClass::GetInteractiveMode() const
{
  Q_D(const MultiViewerWidgetTestClass);
  return d->InteractiveMode;
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::SetInteractiveMode(bool interactiveMode)
{
  Q_D(MultiViewerWidgetTestClass);
  d->InteractiveMode = interactiveMode;
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::initTestCase()
{
  Q_D(MultiViewerWidgetTestClass);

  QmitkRegisterClasses();

  d->DataStorage = mitk::StandaloneDataStorage::New();

  d->RenderingManager = mitk::RenderingManager::GetInstance();
  d->RenderingManager->SetDataStorage(d->DataStorage);

  std::vector<std::string> files;
  files.push_back(d->FileName);

  mitk::DataStorage::SetOfObjects::Pointer allImages = mitk::IOUtil::Load(files, *(d->DataStorage.GetPointer()));
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(allImages->size(), 1), ".. Test image loaded.");

  d->ImageNode = (*allImages)[0];

  d->VisibilityManager = MultiViewerVisibilityManager::New(d->DataStorage);
  d->VisibilityManager->SetInterpolationType(DNDDISPLAY_CUBIC_INTERPOLATION);
  d->VisibilityManager->SetDefaultWindowLayout(WINDOW_LAYOUT_CORONAL);
  d->VisibilityManager->SetDropType(DNDDISPLAY_DROP_SINGLE);
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::cleanupTestCase()
{
  Q_D(MultiViewerWidgetTestClass);
  d->VisibilityManager = 0;
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::init()
{
  Q_D(MultiViewerWidgetTestClass);

  // Create the MultiViewerWidget
  d->MultiViewer = new MultiViewerWidget(d->VisibilityManager, d->RenderingManager);

  // Setup GUI a bit more.
  d->MultiViewer->SetDropType(DNDDISPLAY_DROP_SINGLE);
  d->MultiViewer->SetShowOptionsVisible(true);
  d->MultiViewer->SetWindowLayoutControlsVisible(true);
  d->MultiViewer->SetViewerNumberControlsVisible(true);
  d->MultiViewer->SetShowDropTypeControls(false);
  d->MultiViewer->SetCursorDefaultVisibility(true);
  d->MultiViewer->SetDirectionAnnotationsVisible(true);
  d->MultiViewer->SetIntensityAnnotationVisible(true);
  d->MultiViewer->SetShowMagnificationSlider(true);
  d->MultiViewer->SetRememberSettingsPerWindowLayout(true);
  d->MultiViewer->SetSliceTracking(true);
  d->MultiViewer->SetTimeStepTracking(true);
  d->MultiViewer->SetMagnificationTracking(true);
  d->MultiViewer->SetDefaultWindowLayout(WINDOW_LAYOUT_CORONAL);

  d->MultiViewer->resize(1024, 768);
  d->MultiViewer->show();

  std::vector<mitk::DataNode*> nodes(1);
  nodes[0] = d->ImageNode;

  QmitkRenderWindow* axialWindow = d->MultiViewer->GetSelectedViewer()->GetAxialWindow();
  this->dropNodes(axialWindow, nodes);
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::cleanup()
{
  Q_D(MultiViewerWidgetTestClass);
  delete d->MultiViewer;
  d->MultiViewer = 0;
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::dropNodes(QWidget* window, const std::vector<mitk::DataNode*>& nodes)
{
  Q_D(MultiViewerWidgetTestClass);

  QMimeData* mimeData = new QMimeData;
  QMimeData* mimeData2 = new QMimeData;
  QString dataNodeAddresses("");
  QByteArray byteArray;
  byteArray.resize(sizeof(quintptr) * nodes.size());

  QDataStream ds(&byteArray, QIODevice::WriteOnly);
  QTextStream ts(&dataNodeAddresses);
  for (int i = 0; i < nodes.size(); ++i)
  {
    quintptr dataNodeAddress = reinterpret_cast<quintptr>(nodes[i]);
    ds << dataNodeAddress;
    ts << dataNodeAddress;
    if (i != nodes.size() - 1)
    {
      ts << ",";
    }
  }
  mimeData->setData("application/x-mitk-datanodes", QByteArray(dataNodeAddresses.toLatin1()));
  mimeData2->setData(QmitkMimeTypes::DataNodePtrs, byteArray);
//  QStringList types;
//  types << "application/x-mitk-datanodes";
  QDragEnterEvent dragEnterEvent(window->rect().center(), Qt::CopyAction | Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
  QDropEvent dropEvent(window->rect().center(), Qt::CopyAction | Qt::MoveAction, mimeData2, Qt::LeftButton, Qt::NoModifier);
  dropEvent.acceptProposedAction();
  if (!qApp->notify(window, &dragEnterEvent))
  {
    QTest::qWarn("Drag enter event not accepted by receiving widget.");
  }
  if (!qApp->notify(window, &dropEvent))
  {
    QTest::qWarn("Drop event not accepted by receiving widget.");
  }
}


// --------------------------------------------------------------------------
void MultiViewerWidgetTestClass::testViewer()
{
  Q_D(MultiViewerWidgetTestClass);

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QTest::qWaitForWindowShown(d->MultiViewer);
#else
  QVERIFY(QTest::qWaitForWindowExposed(d->MultiViewer));
#endif

  /// Remove the comment signs while you are doing interactive testing.
  if (d->InteractiveMode)
  {
    QEventLoop loop;
    loop.connect(d->MultiViewer, SIGNAL(destroyed()), SLOT(quit()));
    loop.exec();
  }

  QVERIFY(true);
}

}


// --------------------------------------------------------------------------
static void ShiftArgs(int& argc, char* argv[], int steps = 1)
{
  /// We exploit that there must be a NULL pointer after the arguments.
  /// (Guaranteed by the standard.)
  int i = 1;
  do
  {
    argv[i] = argv[i + steps];
    ++i;
  }
  while (argv[i - 1]);
  argc -= steps;
}


// --------------------------------------------------------------------------
int niftkMultiViewerWidgetTest(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Q_UNUSED(app);

  niftk::MultiViewerWidgetTestClass test;

  std::string interactiveModeOption("-i");
  for (int i = 1; i < argc; ++i)
  {
    if (std::string(argv[i]) == interactiveModeOption)
    {
      test.SetInteractiveMode(true);
      ::ShiftArgs(argc, argv);
      break;
    }
  }

  if (argc < 2)
  {
    MITK_INFO << "Missing argument. No image file given.";
    return 1;
  }

  test.SetFileName(argv[1]);

  /// We used the arguments to initialise the test. No arguments is passed
  /// to the Qt test, so that all the test functions are executed.
  argc = 1;
  argv[1] = NULL;
  return QTest::qExec(&test, argc, argv);
}
