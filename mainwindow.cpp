#include <QThread>
#include <QWidget>
#include <QCalendarWidget>
#include <QFile>
#include <QShortcut>
#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "treemodel.h"
#include "dialog.h"
#include "consolewidget.h"
#include "scenetreewidget.h"
#include "icontitlewidget.h"
#include "renderwidget.h"
#include "projectview.h"
#include "propertieswidget.h"
#include "editorglobal.h"
#include "EngineGlobal.h"
#include "about.h"

#include "advanceddockingsystem/include/SectionWidget.h"
#include "advanceddockingsystem/include/DropOverlay.h"

extern EditorGlobal *editor;
extern EngineGlobal *eng;

using RENDER_MASTER::ICore;
using RENDER_MASTER::ISceneManager;
using RENDER_MASTER::ISubSystem;


static int CONTENT_COUNT = 0;

static ADS_NS::SectionContent::RefPtr createConsole(ADS_NS::ContainerWidget* container)
{
    /*
    QWidget* w = new QWidget();
    QBoxLayout* bl = new QBoxLayout(QBoxLayout::TopToBottom);
    w->setLayout(bl);

    QLabel* l = new QLabel();
    l->setWordWrap(true);
    l->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    l->setText(QString("Lorem Ipsum ist ein einfacher Demo-Text für die Print- und Schriftindustrie. Lorem Ipsum ist in der Industrie bereits der Standard Demo-Text seit 1500, als ein unbekannter Schriftsteller eine Hand voll Wörter nahm und diese durcheinander warf um ein Musterbuch zu erstellen. Es hat nicht nur 5 Jahrhunderte überlebt, sondern auch in Spruch in die elektronische Schriftbearbeitung geschafft (bemerke, nahezu unverändert). Bekannt wurde es 1960, mit dem erscheinen von Letrase, welches Passagen von Lorem Ipsum enhielt, so wie Desktop Software wie Aldus PageMaker - ebenfalls mit Lorem Ipsum."));
    bl->addWidget(l);
*/

    ConsoleWidget *w = new ConsoleWidget();

    const int index = ++CONTENT_COUNT;
	ADS_NS::SectionContent::RefPtr sc = ADS_NS::SectionContent::newSectionContent(QString("Console-%1").arg(index), container, new IconTitleWidget(QIcon(), QString("Console").arg(index)), w);
    //sc->setTitle("Ein Label " + QString::number(index));
    return sc;
}

static ADS_NS::SectionContent::RefPtr createD3D11Widget(ADS_NS::ContainerWidget* container)
{
    RenderWidget* w = new RenderWidget();

    const int index = ++CONTENT_COUNT;
	return ADS_NS::SectionContent::newSectionContent(QString("Viewport-%2").arg(index), container, new IconTitleWidget(QIcon(), QString("Viewport").arg(index)), w);
}

static ADS_NS::SectionContent::RefPtr createSceneTree(ADS_NS::ContainerWidget* container)
{
    SceneTreeWidget* w = new SceneTreeWidget();

	editor->sceneTreeWidget = w;
	editor->SceneTreeInited(w);

    const int index = ++CONTENT_COUNT;
	return ADS_NS::SectionContent::newSectionContent(QString("Scene"), container, new IconTitleWidget(QIcon(), QString("Scene").arg(index)), w);
}

static ADS_NS::SectionContent::RefPtr createProjectView(ADS_NS::ContainerWidget* container)
{
    ProjectView* w = new ProjectView();

    const int index = ++CONTENT_COUNT;
	return ADS_NS::SectionContent::newSectionContent(QString("Project"), container, new IconTitleWidget(QIcon(), QString("Project").arg(index)), w);
}

static ADS_NS::SectionContent::RefPtr createPropertiesView(ADS_NS::ContainerWidget* container)
{
    PropertiesWidget* w = new PropertiesWidget();
    editor->propertiesWidget = w;

    const int index = ++CONTENT_COUNT;
	return ADS_NS::SectionContent::newSectionContent(QString("Properties"), container, new IconTitleWidget(QIcon(), QString("Properties").arg(index)), w);
}

static QByteArray loadDataHelper(const QString& fname)
{
    QFile f(fname + QString(".dat"));
    if (f.open(QFile::ReadOnly))
    {
        QByteArray ba = f.readAll();
        f.close();
        return ba;
    }
    return QByteArray();
}

static void storeDataHelper(const QString& fname, const QByteArray& ba)
{
    QFile f(fname + QString(".dat"));
    if (f.open(QFile::WriteOnly))
    {
        f.write(ba);
        f.close();
    }
}

void apply_style(QString name)
{
	QFile File(name);
	File.open(QFile::ReadOnly);
	QString StyleSheet = QLatin1String(File.readAll());
	qApp->setStyleSheet(StyleSheet);
}

void apply_dark_style()
{
	apply_style("dark.qss");
}

void apply_light_style()
{
	apply_style("light.qss");
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	QShortcut *shortcut_dark = new QShortcut(QKeySequence(Qt::Key_F5), this);
	QObject::connect(shortcut_dark, &QShortcut::activated, apply_dark_style);

	QShortcut *shortcut_light = new QShortcut(QKeySequence(Qt::Key_F6), this);
	QObject::connect(shortcut_light, &QShortcut::activated, apply_light_style);

	QShortcut *shortcut_focus = new QShortcut(QKeySequence(Qt::Key_F), this);
	QObject::connect(shortcut_focus, &QShortcut::activated, [&]() { editor->RaiseFocusOnSelevtedObjects(); });

	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this, SLOT(clone_node()));
	new QShortcut(QKeySequence(Qt::Key_F7), this, SLOT(shaders_reload()));

    // ADS - Create main container (ContainerWidget).
    _container = new ADS_NS::ContainerWidget();
    setCentralWidget(_container);

    // ADS - Adding some contents.
    if (true)
	{
        ADS_NS::ContainerWidget* cw = _container;
        ADS_NS::SectionWidget* sw = NULL;

        sw = _container->addSectionContent(createConsole(cw), sw, ADS_NS::CenterDropArea);

        ADS_NS::SectionContent::RefPtr sc = createD3D11Widget(cw);
        sc->setFlags(ADS_NS::SectionContent::AllFlags ^ ADS_NS::SectionContent::Closeable);
        sw = _container->addSectionContent(sc, sw, ADS_NS::TopDropArea);

		sw = _container->addSectionContent(createSceneTree(cw), nullptr, ADS_NS::LeftDropArea);
        sw = _container->addSectionContent(createProjectView(cw), nullptr, ADS_NS::BottomDropArea);
        sw = _container->addSectionContent(createPropertiesView(cw), nullptr, ADS_NS::RightDropArea);
    }

    resize(800, 600);
    restoreGeometry(loadDataHelper("MainWindow"));

    // ADS - Restore geometries and states of contents.
    _container->restoreState(loadDataHelper("ContainerWidget"));

	editor->ToggleManipulator(MANIPULATOR::SELECT);
	switch_button(ui->actionactionCursorSelect);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
}

void MainWindow::on_actionNew_scene_triggered()
{
	dg = new Dialog(this);
    dg->show();
    dg->exec();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionBuild_Standalone_triggered()
{
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    Q_UNUSED(e);
    storeDataHelper("MainWindow", saveGeometry());
	storeDataHelper("ContainerWidget", _container->saveState());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	qDebug() << "jjj";
}



//void MainWindow::contextMenuEvent(QContextMenuEvent* e)
//{
//    Q_UNUSED(e);
//    QMenu* m = _container->createContextMenu();
//    m->exec(QCursor::pos());
//    delete m;
//}

void MainWindow::on_actionAbout_Render_Master_triggered()
{
	auto *about = new About();
	about->show();
	about->exec();
}

void MainWindow::switch_button(QAction *action)
{
	bool flag = ui->actionactionManipulatorTransform == action;
	ui->actionactionManipulatorTransform->setChecked(flag);

	flag = ui->actionManipulatorScale == action;
	ui->actionManipulatorScale->setChecked(flag);

	flag = ui->actionactionManipulatorRotate == action;
	ui->actionactionManipulatorRotate->setChecked(flag);

	flag = ui->actionactionCursorSelect == action;
	ui->actionactionCursorSelect->setChecked(flag);
}

void MainWindow::on_actionactionCursorSelect_triggered(bool checked)
{
	Q_UNUSED( checked )
	editor->ToggleManipulator(MANIPULATOR::SELECT);
	switch_button(ui->actionactionCursorSelect);
}

void MainWindow::on_actionactionManipulatorRotate_triggered(bool checked)
{
	Q_UNUSED( checked )
	editor->ToggleManipulator(MANIPULATOR::ROTATE);
	switch_button(ui->actionactionManipulatorRotate);
}

void MainWindow::on_actionManipulatorScale_triggered(bool checked)
{
	Q_UNUSED( checked )
	editor->ToggleManipulator(MANIPULATOR::SCALE);
	switch_button(ui->actionManipulatorScale);
}

void MainWindow::on_actionactionManipulatorTransform_triggered(bool checked)
{
	Q_UNUSED( checked )
	editor->ToggleManipulator(MANIPULATOR::TRANSLATE);
	switch_button(ui->actionactionManipulatorTransform);
}

void MainWindow::on_actionSave_scene_triggered()
{
	if (!eng) return;

	ICore *core;
	eng->GetCore(core);

	ISceneManager *sm;
	core->GetSubSystem((ISubSystem**)&sm, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);

	sm->SaveScene("Scene.yaml");
}

void MainWindow::on_actionLoad_Scene_triggered()
{
	if (!eng) return;

	ICore *core;
	eng->GetCore(core);

	auto path = eng->GetProjectDir();

	//QString fileName = QFileDialog::getOpenFileName(this, tr("Open Scene"), path, tr("Scene file (*.yaml)"));

	qDebug() << "Loading Scene: " << "Scene.yaml";

	ISceneManager *sm;
	core->GetSubSystem((ISubSystem**)&sm, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);

	sm->LoadScene("Scene.yaml");
}

void MainWindow::on_actionClose_Scene_triggered()
{
	if (!eng) return;

	ICore *core;
	eng->GetCore(core);

	ISceneManager *sm;
	core->GetSubSystem((ISubSystem**)&sm, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);

	sm->CloseScene();
}

void MainWindow::clone_node()
{
	if (editor->IsSomeObjectSelected())
	{
		if (!eng) return;

		ICore *core;
		eng->GetCore(core);

		IResourceManager *resMan;
		core->GetSubSystem((ISubSystem**)&resMan, RENDER_MASTER::SUBSYSTEM_TYPE::RESOURCE_MANAGER);

		// not impl
		//IGameObject *res = editor->GetSelectionObject();
		//IGameObject *clonedRes;
		//resMan->CloneGameObject(res, &clonedRes);
	}
}

void MainWindow::shaders_reload()
{
	if (!eng) return;

	ICore *core;
	eng->GetCore(core);

	IRender *render;
	core->GetSubSystem((ISubSystem**)&render, RENDER_MASTER::SUBSYSTEM_TYPE::RENDER);

	render->ShadersReload();
}

