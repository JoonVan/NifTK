/*=============================================================================

  NifTK: A software platform for medical image computing.

  Copyright (c) University College London (UCL). All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

  See LICENSE.txt in the top level directory for details.

=============================================================================*/

#include "niftkSingleViewerWidgetTest.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTest>
#include <QTextStream>

#include <mitkGlobalInteraction.h>
#include <mitkIOUtil.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestingMacros.h>

#include <QmitkRenderingManagerFactory.h>
#include <QmitkApplicationCursor.h>

#include <mitkMIDASOrientationUtils.h>
#include <mitkNifTKCoreObjectFactory.h>
#include <niftkSingleViewerWidget.h>
#include <niftkMultiViewerWidget.h>
#include <niftkMultiViewerVisibilityManager.h>

#include <mitkItkSignalCollector.cxx>
#include <mitkQtSignalCollector.cxx>
#include <niftkSingleViewerWidgetState.h>


class niftkSingleViewerWidgetTestClassPrivate
{
public:
  std::string FileName;
  mitk::DataStorage::Pointer DataStorage;
  mitk::RenderingManager::Pointer RenderingManager;

  mitk::DataNode::Pointer ImageNode;

  niftkSingleViewerWidget* Viewer;
  niftkMultiViewerVisibilityManager* VisibilityManager;

  bool InteractiveMode;
};


// --------------------------------------------------------------------------
niftkSingleViewerWidgetTestClass::niftkSingleViewerWidgetTestClass()
: QObject()
, d_ptr(new niftkSingleViewerWidgetTestClassPrivate())
{
  Q_D(niftkSingleViewerWidgetTestClass);

  d->ImageNode = 0;
  d->Viewer = 0;
  d->VisibilityManager = 0;
  d->InteractiveMode = false;
}


// --------------------------------------------------------------------------
niftkSingleViewerWidgetTestClass::~niftkSingleViewerWidgetTestClass()
{
}


// --------------------------------------------------------------------------
std::string niftkSingleViewerWidgetTestClass::GetFileName() const
{
  Q_D(const niftkSingleViewerWidgetTestClass);
  return d->FileName;
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::SetFileName(const std::string& fileName)
{
  Q_D(niftkSingleViewerWidgetTestClass);
  d->FileName = fileName;
}


// --------------------------------------------------------------------------
bool niftkSingleViewerWidgetTestClass::GetInteractiveMode() const
{
  Q_D(const niftkSingleViewerWidgetTestClass);
  return d->InteractiveMode;
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::SetInteractiveMode(bool interactiveMode)
{
  Q_D(niftkSingleViewerWidgetTestClass);
  d->InteractiveMode = interactiveMode;
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::initTestCase()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  // Need to load images, specifically using MIDAS/DRC object factory.
  ::RegisterNifTKCoreObjectFactory();

  mitk::GlobalInteraction* globalInteraction =  mitk::GlobalInteraction::GetInstance();
  globalInteraction->Initialize("global");
  globalInteraction->GetStateMachineFactory()->LoadBehaviorString(mitk::DnDDisplayStateMachine::STATE_MACHINE_XML);

  /// Create and register RenderingManagerFactory for this platform.
  static QmitkRenderingManagerFactory qmitkRenderingManagerFactory;
  Q_UNUSED(qmitkRenderingManagerFactory);

  /// Create one instance
  static QmitkApplicationCursor globalQmitkApplicationCursor;
  Q_UNUSED(globalQmitkApplicationCursor);

  d->DataStorage = mitk::StandaloneDataStorage::New();

  d->RenderingManager = mitk::RenderingManager::GetInstance();
  d->RenderingManager->SetDataStorage(d->DataStorage);

  std::vector<std::string> files;
  files.push_back(d->FileName);

  mitk::IOUtil::LoadFiles(files, *(d->DataStorage.GetPointer()));
  mitk::DataStorage::SetOfObjects::ConstPointer allImages = d->DataStorage->GetAll();
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(allImages->size(), 1), ".. Test image loaded.");

  d->ImageNode = (*allImages)[0];

  d->VisibilityManager = new niftkMultiViewerVisibilityManager(d->DataStorage);
  d->VisibilityManager->SetInterpolationType(DNDDISPLAY_CUBIC_INTERPOLATION);
  d->VisibilityManager->SetDefaultWindowLayout(WINDOW_LAYOUT_CORONAL);
  d->VisibilityManager->SetDropType(DNDDISPLAY_DROP_SINGLE);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::cleanupTestCase()
{
  Q_D(niftkSingleViewerWidgetTestClass);
  delete d->VisibilityManager;
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::init()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  d->Viewer = new niftkSingleViewerWidget(tr("QmitkRenderWindow"),
    -5, 20, 0, d->RenderingManager, d->DataStorage);
  d->Viewer->setObjectName(tr("niftkSingleViewerWidget"));

//  QColor backgroundColour("black");
  QColor backgroundColour("#fffaf0");
  d->Viewer->SetDirectionAnnotationsVisible(true);
  d->Viewer->SetBackgroundColor(backgroundColour);
  d->Viewer->SetShow3DWindowIn2x2WindowLayout(true);
  d->Viewer->SetRememberSettingsPerWindowLayout(true);
  d->Viewer->SetDisplayInteractionsEnabled(true);
  d->Viewer->SetCursorPositionBinding(true);
  d->Viewer->SetScaleFactorBinding(true);
  d->Viewer->SetDefaultSingleWindowLayout(WINDOW_LAYOUT_CORONAL);
  d->Viewer->SetDefaultMultiWindowLayout(WINDOW_LAYOUT_ORTHO);

  d->VisibilityManager->connect(d->Viewer, SIGNAL(NodesDropped(niftkSingleViewerWidget*, QmitkRenderWindow*, std::vector<mitk::DataNode*>)), SLOT(OnNodesDropped(niftkSingleViewerWidget*, QmitkRenderWindow*, std::vector<mitk::DataNode*>)), Qt::DirectConnection);

  d->VisibilityManager->RegisterViewer(d->Viewer);
  d->VisibilityManager->SetAllNodeVisibilityForViewer(0, false);

  d->Viewer->resize(1024, 768);
  d->Viewer->show();

  QTest::qWaitForWindowShown(d->Viewer);

  std::vector<mitk::DataNode*> nodes(1);
  nodes[0] = d->ImageNode;

  QmitkRenderWindow* axialWindow = d->Viewer->GetAxialWindow();
  this->dropNodes(axialWindow, nodes);

  d->Viewer->SetCursorVisible(true);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::cleanup()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  if (d->InteractiveMode)
  {
    QEventLoop loop;
    loop.connect(d->Viewer, SIGNAL(destroyed()), SLOT(quit()));
    loop.exec();
  }

  d->VisibilityManager->DeRegisterAllViewers();

  delete d->Viewer;
  d->Viewer = 0;
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::dropNodes(QWidget* window, const std::vector<mitk::DataNode*>& nodes)
{
  Q_D(niftkSingleViewerWidgetTestClass);

  QMimeData* mimeData = new QMimeData;
  QString dataNodeAddresses("");
  for (int i = 0; i < nodes.size(); ++i)
  {
    long dataNodeAddress = reinterpret_cast<long>(nodes[i]);
    QTextStream(&dataNodeAddresses) << dataNodeAddress;

    if (i != nodes.size() - 1)
    {
      QTextStream(&dataNodeAddresses) << ",";
    }
  }
  mimeData->setData("application/x-mitk-datanodes", QByteArray(dataNodeAddresses.toAscii()));
  QStringList types;
  types << "application/x-mitk-datanodes";
  QDropEvent dropEvent(window->rect().center(), Qt::CopyAction | Qt::MoveAction, mimeData, Qt::LeftButton, Qt::NoModifier);
  dropEvent.acceptProposedAction();
  QApplication::instance()->sendEvent(window, &dropEvent);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testViewer()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  /// Tests if the viewer has been successfully created.
  QVERIFY(d->Viewer);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testGetOrientation()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  ViewerStateTester::Pointer stateTester = ViewerStateTester::New(d->Viewer);
  ViewerState::Pointer state = ViewerState::New(d->Viewer);
  stateTester->SetExpectedState(state);

  MIDASOrientation orientation = d->Viewer->GetOrientation();

  /// The default window layout was set to coronal in the init() function.
  QCOMPARE(orientation, MIDAS_ORIENTATION_CORONAL);
  QCOMPARE(stateTester->GetItkSignals().size(), 0ul);
  QCOMPARE(stateTester->GetQtSignals().size(), 0ul);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testGetSelectedPosition()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  /// Make sure that the state does not change and no signal is sent out.
  ViewerStateTester::Pointer stateTester = ViewerStateTester::New(d->Viewer);
  ViewerState::Pointer state = ViewerState::New(d->Viewer);
  stateTester->SetExpectedState(state);

  mitk::Point3D selectedPosition = d->Viewer->GetSelectedPosition();

  QCOMPARE(stateTester->GetItkSignals().size(), 0ul);
  QCOMPARE(stateTester->GetQtSignals().size(), 0ul);

  mitk::Image* image = dynamic_cast<mitk::Image*>(d->ImageNode->GetData());
  mitk::Geometry3D::Pointer geometry = image->GetGeometry();
  mitk::Point3D centre = geometry->GetCenter();

  mitk::Vector3D extentsInWorldCoordinateOrder;
  mitk::GetExtentsInVxInWorldCoordinateOrder(image, extentsInWorldCoordinateOrder);

  mitk::Vector3D spacingInWorldCoordinateOrder;
  mitk::GetSpacingInWorldCoordinateOrder(image, spacingInWorldCoordinateOrder);

  for (int i = 0; i < 3; ++i)
  {
    double distanceFromCentre = std::abs(selectedPosition[i] - centre[i]);
    if (static_cast<int>(extentsInWorldCoordinateOrder[i]) % 2 == 0)
    {
      /// If the number of slices is an even number then the selected position
      /// must be a half voxel far from the centre, either way.
      /// Tolerance is 0.001 millimetre because of float precision.
      QVERIFY(std::abs(distanceFromCentre - spacingInWorldCoordinateOrder[i] / 2.0) < 0.001);
    }
    else
    {
      /// If the number of slices is an odd number then the selected position
      /// must be exactly at the centre position.
      /// Tolerance is 0.001 millimetre because of float precision.
      QVERIFY(distanceFromCentre < 0.001);
    }
  }
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testSetSelectedPosition()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  mitk::Image* image = dynamic_cast<mitk::Image*>(d->ImageNode->GetData());

  mitk::Vector3D spacingInWorldCoordinateOrder;
  mitk::GetSpacingInWorldCoordinateOrder(image, spacingInWorldCoordinateOrder);

  ViewerStateTester::Pointer stateTester = ViewerStateTester::New(d->Viewer);

  QmitkRenderWindow* axialWindow = d->Viewer->GetAxialWindow();
  QmitkRenderWindow* sagittalWindow = d->Viewer->GetSagittalWindow();
  QmitkRenderWindow* coronalWindow = d->Viewer->GetCoronalWindow();

  /// Register to listen to SliceNavigators, slice changed events.
  mitk::SliceNavigationController::GeometrySliceEvent geometrySliceEvent(NULL, 0);
  mitk::SliceNavigationController* axialSnc = axialWindow->GetSliceNavigationController();
  mitk::SliceNavigationController* sagittalSnc = sagittalWindow->GetSliceNavigationController();
  mitk::SliceNavigationController* coronalSnc = coronalWindow->GetSliceNavigationController();

  stateTester->Connect(axialSnc, geometrySliceEvent);
  stateTester->Connect(sagittalSnc, geometrySliceEvent);
  stateTester->Connect(coronalSnc, geometrySliceEvent);

  mitk::FocusEvent focusEvent;
  stateTester->Connect(axialWindow->GetRenderer(), focusEvent);
  stateTester->Connect(sagittalWindow->GetRenderer(), focusEvent);
  stateTester->Connect(coronalWindow->GetRenderer(), focusEvent);

  mitk::Point3D initialPosition = d->Viewer->GetSelectedPosition();
  mitk::Point3D newPosition = initialPosition;
  newPosition[0] += 2 * spacingInWorldCoordinateOrder[0];

  d->Viewer->SetSelectedPosition(newPosition);

  QCOMPARE(d->Viewer->GetSelectedPosition(), newPosition);
  QCOMPARE(stateTester->GetItkSignals(axialSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals(sagittalSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals(coronalSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals().size(), 1ul);

  stateTester->Clear();

  newPosition[1] += 2 * spacingInWorldCoordinateOrder[1];
  d->Viewer->SetSelectedPosition(newPosition);

  QCOMPARE(d->Viewer->GetSelectedPosition(), newPosition);
  QCOMPARE(stateTester->GetItkSignals(axialSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals(sagittalSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals(coronalSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals().size(), 1ul);

  stateTester->Clear();

  newPosition[2] += 2 * spacingInWorldCoordinateOrder[2];
  d->Viewer->SetSelectedPosition(newPosition);

  QCOMPARE(d->Viewer->GetSelectedPosition(), newPosition);
  QCOMPARE(stateTester->GetItkSignals(axialSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals(sagittalSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals(coronalSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals().size(), 1ul);

  stateTester->Clear();

  newPosition[0] -= 3 * spacingInWorldCoordinateOrder[0];
  newPosition[1] -= 3 * spacingInWorldCoordinateOrder[1];
  d->Viewer->SetSelectedPosition(newPosition);

  QCOMPARE(d->Viewer->GetSelectedPosition(), newPosition);
  QCOMPARE(stateTester->GetItkSignals(axialSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals(sagittalSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals(coronalSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals().size(), 2ul);

  stateTester->Clear();

  newPosition[0] -= 4 * spacingInWorldCoordinateOrder[0];
  newPosition[2] -= 4 * spacingInWorldCoordinateOrder[2];
  d->Viewer->SetSelectedPosition(newPosition);

  QCOMPARE(d->Viewer->GetSelectedPosition(), newPosition);
  QCOMPARE(stateTester->GetItkSignals(axialSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals(sagittalSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals(coronalSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals().size(), 2ul);

  stateTester->Clear();

  newPosition[1] += 5 * spacingInWorldCoordinateOrder[1];
  newPosition[2] += 5 * spacingInWorldCoordinateOrder[2];
  d->Viewer->SetSelectedPosition(newPosition);

  QCOMPARE(d->Viewer->GetSelectedPosition(), newPosition);
  QCOMPARE(stateTester->GetItkSignals(axialSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals(sagittalSnc).size(), 0ul);
  QCOMPARE(stateTester->GetItkSignals(coronalSnc).size(), 1ul);
  QCOMPARE(stateTester->GetItkSignals().size(), 2ul);

  stateTester->Clear();
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testSelectPositionByInteraction()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  QmitkRenderWindow* axialWindow = d->Viewer->GetAxialWindow();
  mitk::SliceNavigationController* axialSnc = axialWindow->GetSliceNavigationController();
  QmitkRenderWindow* sagittalWindow = d->Viewer->GetSagittalWindow();
  mitk::SliceNavigationController* sagittalSnc = sagittalWindow->GetSliceNavigationController();
  QmitkRenderWindow* coronalWindow = d->Viewer->GetCoronalWindow();
  mitk::SliceNavigationController* coronalSnc = coronalWindow->GetSliceNavigationController();

  /// Register to listen to SliceNavigators, slice changed events.
  ViewerStateTester::Pointer viewerStateTester = ViewerStateTester::New(d->Viewer);

  mitk::SliceNavigationController::GeometrySliceEvent geometrySliceEvent(NULL, 0);
  viewerStateTester->Connect(axialSnc, geometrySliceEvent);
  viewerStateTester->Connect(sagittalSnc, geometrySliceEvent);
  viewerStateTester->Connect(coronalSnc, geometrySliceEvent);

  QPoint centre = coronalWindow->rect().center();
  QTest::mouseClick(coronalWindow, Qt::LeftButton, Qt::NoModifier, centre);

  viewerStateTester->Clear();

  mitk::Point3D lastPosition = d->Viewer->GetSelectedPosition();

  QPoint newPoint = centre;
  newPoint.rx() += 30;
  QTest::mouseClick(coronalWindow, Qt::LeftButton, Qt::NoModifier, newPoint);

  mitk::Point3D newPosition = d->Viewer->GetSelectedPosition();
  QVERIFY(newPosition[0] != lastPosition[0]);
  QCOMPARE(newPosition[1], lastPosition[1]);
  QCOMPARE(newPosition[2], lastPosition[2]);
  QCOMPARE(viewerStateTester->GetItkSignals(axialSnc).size(), 0ul);
  QCOMPARE(viewerStateTester->GetItkSignals(sagittalSnc).size(), 1ul);
  QCOMPARE(viewerStateTester->GetItkSignals(coronalSnc).size(), 0ul);

  viewerStateTester->Clear();

  lastPosition = newPosition;

  newPoint.ry() += 20;
  QTest::mouseClick(coronalWindow, Qt::LeftButton, Qt::NoModifier, newPoint);

  newPosition = d->Viewer->GetSelectedPosition();
  QCOMPARE(newPosition[0], lastPosition[0]);
  QCOMPARE(newPosition[1], lastPosition[1]);
  QVERIFY(newPosition[2] != lastPosition[1]);
  QCOMPARE(viewerStateTester->GetItkSignals(axialSnc).size(), 1ul);
  QCOMPARE(viewerStateTester->GetItkSignals(sagittalSnc).size(), 0ul);
  QCOMPARE(viewerStateTester->GetItkSignals(coronalSnc).size(), 0ul);

  viewerStateTester->Clear();

  lastPosition = d->Viewer->GetSelectedPosition();

  newPoint.rx() -= 40;
  newPoint.ry() += 50;
  QTest::mouseClick(coronalWindow, Qt::LeftButton, Qt::NoModifier, newPoint);

  newPosition = d->Viewer->GetSelectedPosition();
  QVERIFY(newPosition[0] != lastPosition[0]);
  QCOMPARE(newPosition[1], lastPosition[1]);
  QVERIFY(newPosition[2] != lastPosition[2]);
  QCOMPARE(viewerStateTester->GetItkSignals(axialSnc).size(), 1ul);
  QCOMPARE(viewerStateTester->GetItkSignals(sagittalSnc).size(), 1ul);
  QCOMPARE(viewerStateTester->GetItkSignals(coronalSnc).size(), 0ul);

  viewerStateTester->Clear();
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testGetWindowLayout()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  /// The default window layout was set to coronal in the init() function.
  QCOMPARE(d->Viewer->GetWindowLayout(), WINDOW_LAYOUT_CORONAL);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testGetSelectedRenderWindow()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  QmitkRenderWindow* coronalWindow = d->Viewer->GetCoronalWindow();

  /// The default window layout was set to coronal in the init() function.
  QCOMPARE(d->Viewer->GetSelectedRenderWindow(), coronalWindow);
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testFocusedRenderer()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  QmitkRenderWindow* coronalWindow = d->Viewer->GetCoronalWindow();

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  mitk::BaseRenderer* focusedRenderer = focusManager->GetFocused();

  /// The default window layout was set to coronal in the init() function.
  /// TODO The focus should be on the selected window.
//  QCOMPARE(focusedRenderer, coronalWindow->GetRenderer());
}


// --------------------------------------------------------------------------
void niftkSingleViewerWidgetTestClass::testSetWindowLayout()
{
  Q_D(niftkSingleViewerWidgetTestClass);

  mitk::SliceNavigationController* axialSnc = d->Viewer->GetAxialWindow()->GetSliceNavigationController();
  mitk::SliceNavigationController* sagittalSnc = d->Viewer->GetSagittalWindow()->GetSliceNavigationController();
  mitk::SliceNavigationController* coronalSnc = d->Viewer->GetCoronalWindow()->GetSliceNavigationController();

  mitk::FocusManager* focusManager = mitk::GlobalInteraction::GetInstance()->GetFocusManager();
  /// TODO The focus should be on the coronal window already.
  focusManager->SetFocused(d->Viewer->GetCoronalWindow()->GetRenderer());

  ViewerStateTester::Pointer stateTester = ViewerStateTester::New(d->Viewer);

  mitk::SliceNavigationController::GeometrySliceEvent geometrySliceEvent(NULL, 0);
  stateTester->Connect(axialSnc, geometrySliceEvent);
  stateTester->Connect(sagittalSnc, geometrySliceEvent);
  stateTester->Connect(coronalSnc, geometrySliceEvent);

  mitk::FocusEvent focusEvent;
  stateTester->Connect(focusManager, focusEvent);

  QVERIFY(d->Viewer->GetCoronalWindow()->hasFocus());
  QVERIFY(focusManager->GetFocused() == d->Viewer->GetCoronalWindow()->GetRenderer());

  /// Disabled because the cursor state of the sagittal window will be different,
  /// since it will be initialised just now.
//  niftkSingleViewerWidgetState::Pointer expectedState = niftkSingleViewerWidgetState::New(d->Viewer);
//  expectedState->SetOrientation(MIDAS_ORIENTATION_SAGITTAL);
//  expectedState->SetSelectedRenderWindow(d->Viewer->GetSagittalWindow());
//  expectedState->SetWindowLayout(WINDOW_LAYOUT_SAGITTAL);
//  stateTester->SetExpectedState(expectedState);

  /// The default layout was set to coronal in the init() function.
  d->Viewer->SetWindowLayout(WINDOW_LAYOUT_SAGITTAL);

  QVERIFY(d->Viewer->GetSagittalWindow()->hasFocus());
  QVERIFY(focusManager->GetFocused() == d->Viewer->GetSagittalWindow()->GetRenderer());

  QCOMPARE(stateTester->GetItkSignals(focusManager, focusEvent).size(), 1ul);
  QVERIFY(stateTester->GetItkSignals(axialSnc, geometrySliceEvent).size() <= 1);
  QVERIFY(stateTester->GetItkSignals(sagittalSnc, geometrySliceEvent).size() <= 1);
  QVERIFY(stateTester->GetItkSignals(sagittalSnc, geometrySliceEvent).size() <= 1);

//  viewerStateTester->Clear();

//  QRect rect = sagittalWindow->rect();
//  QPoint centre = rect.center();
//  QPoint bottomLeftCorner = rect.bottomLeft();
//  QPoint aPosition((bottomLeftCorner.x() + centre.x()) / 2, (bottomLeftCorner.y() + centre.y()) / 2);
//  QTest::mouseClick(sagittalWindow, Qt::LeftButton, Qt::NoModifier, aPosition);

//  QVERIFY(sncAndFocusSignals.size() <= 3);
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
int niftkSingleViewerWidgetTest(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Q_UNUSED(app);

  niftkSingleViewerWidgetTestClass test;

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
  ::ShiftArgs(argc, argv);

  /// We used the arguments to initialise the test. No arguments is passed
  /// to the Qt test, so that all the test functions are executed.
//  argc = 1;
//  argv[1] = NULL;
  return QTest::qExec(&test, argc, argv);
}
