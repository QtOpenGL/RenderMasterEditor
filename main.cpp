#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

#include "mainwindow.h"
#include "EngineGlobal.h"
#include "editorglobal.h"

#include "Engine.h"

using namespace RENDER_MASTER;

EngineGlobal *eng;
EditorGlobal *editor;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // splash
/*
    QPixmap pixmap("render_master2.png");
    QSplashScreen splash(pixmap);

    splash.show();

    for(float o = 0.01f; o < 1.0f; o+=0.01f)
    {
        splash.setWindowOpacity(o);
        a.thread()->msleep(2);
    }

    a.thread()->msleep(2000);
*/
    QFile file(":/styles/dark.qss");
    file.open(QFile::ReadOnly);
    QString str = QLatin1String(file.readAll());
    a.setStyleSheet(str);

    ICore *pCore;
    GetCore(&pCore);

    eng = new EngineGlobal();
    eng->SetCore(pCore);

    editor = new EditorGlobal();

    //splash.hide();

    MainWindow *w = new MainWindow;
    w->show();

    QWidget *render_view = w->findChild<QWidget*>("D3D11Widget");
    HWND h = (HWND)render_view->winId();

	auto glFlag = INIT_FLAGS::DIRECTX11;
	w->setWindowTitle(w->windowTitle() + (glFlag == INIT_FLAGS::OPENGL45 ? "  (GL)" : " (DX11)"));
	auto rm_inited = pCore->Init(INIT_FLAGS::EXTERN_WINDOW | glFlag, "resources", &h);

	ResourcePtr<IModel> model1;
	ResourcePtr<IModel> model2;

	if (SUCCEEDED(rm_inited))
	{
		eng->Init(pCore);

		IResourceManager *pResManager;
		pCore->GetSubSystem((ISubSystem**)&pResManager, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

		model1 = pResManager->loadModel("box.fbx");
		model2 = pResManager->loadModel("box.fbx");
		model2->SetPosition(&vec3(15.0f, 0.0f, 0.0f));
	}else
		qWarning() << "Failed to initialize RenderMaster";

    int ret = a.exec();

	model1.reset();
	model2.reset();

	if (SUCCEEDED(rm_inited))
    {
		eng->BeforeClose();
		pCore->ReleaseEngine();
	}

	FreeCore(pCore);

    delete eng;
    delete editor;

    return ret;
}
