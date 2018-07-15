#include "renderwidget.h"
#include "ui_dd3d11widget.h"
#include "EngineGlobal.h"
#include "Engine.h"
#include <QMouseEvent>

using namespace RENDER_MASTER;

extern EngineGlobal* eng;

RenderWidget::RenderWidget(QWidget *parent) :
    QWidget(parent, Qt::MSWindowsOwnDC),
    ui(new Ui::D3D11Widget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_NoBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_PaintUnclipped);
    setAttribute(Qt::WA_NativeWindow);
#ifdef _WIN32
    //setAttribute(Qt::WA_TransparentForMouseEvents);
#endif

    //setStyleSheet("background-color: black;");
    setFocusPolicy(Qt::StrongFocus);

    connect(eng, &EngineGlobal::EngineInited, this, &RenderWidget::onEngineInited, Qt::DirectConnection);
    connect(eng, &EngineGlobal::EngineBeforeClose, this, &RenderWidget::onEngineClosed, Qt::DirectConnection);
    connect(eng, &EngineGlobal::OnRender, this, &RenderWidget::onRender, Qt::DirectConnection);
    connect(eng, &EngineGlobal::OnUpdate, this, &RenderWidget::onUpdate, Qt::DirectConnection);

    h = (HWND)winId();
}

RenderWidget::~RenderWidget()
{
    delete ui;
}

void RenderWidget::resizeEvent(QResizeEvent* evt)
{
    Q_UNUSED( evt )
}

void RenderWidget::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED( evt )
/*
    if (pCore)
    {
        HWND h = (HWND)winId();
        //pCoreRender->MakeCurrent(&h);

        ICamera *pDefaultCamera{nullptr};
        pSceneManager->GetDefaultCamera(&pDefaultCamera);

        //pCoreRender->SetViewport(size().width(), size().height());

        pCore->RenderFrame(&h, pDefaultCamera);

        pCore->Log("D3D11Widget::paintEvent", RENDER_MASTER::LOG_TYPE::NORMAL);
    }
    */
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    mouse = 1;
    lastMousePos = event->pos();
    //pCore->Log("RenderWidget::mousePressEvent()", RENDER_MASTER::LOG_TYPE::NORMAL);
    QWidget::mousePressEvent(event);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    mouse = 0;
    //pCore->Log("RenderWidget::mouseReleaseEvent()", RENDER_MASTER::LOG_TYPE::NORMAL);
    QWidget::mouseReleaseEvent(event);
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (pCore && mouse)
    {
        dx = event->pos().x() - lastMousePos.x();
        dy = event->pos().y() - lastMousePos.y();
        lastMousePos = event->pos();

        //pCore->Log("RenderWidget::mouseMoveEvent(QMouseEvent *event)", RENDER_MASTER::LOG_TYPE::NORMAL);

        QWidget::mouseMoveEvent(event);
    }
}

void RenderWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) {key_w = 1; }
    if (event->key() == Qt::Key_S) {key_s = 1; }
    if (event->key() == Qt::Key_A) {key_a = 1; }
    if (event->key() == Qt::Key_D) {key_d = 1; }
    if (event->key() == Qt::Key_Q) {key_q = 1; }
    if (event->key() == Qt::Key_E) {key_e = 1; }
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_W) {key_w = 0; }
    if (event->key() == Qt::Key_S) {key_s = 0; }
    if (event->key() == Qt::Key_A) {key_a = 0; }
    if (event->key() == Qt::Key_D) {key_d = 0; }
    if (event->key() == Qt::Key_Q) {key_q = 0; }
    if (event->key() == Qt::Key_E) {key_e = 0; }
}


void RenderWidget::onRender()
{
    if (pCore)
    {
        pCore->RenderFrame(&h, pCamera);
        //pCore->Log("RenderWidget::onRender()", RENDER_MASTER::LOG_TYPE::NORMAL);
    }
}

void RenderWidget::onUpdate(float dt)
{
    if (pCore)
    {
        {
			const float moveSpeed = 20.0f;

			vec3 pos;
            pCamera->GetPosition(&pos);

			mat4 M;
            pCamera->GetModelMatrix(&M);

			vec3 orth_direction = vec3(M.Column(0)); // X local
			vec3 forward_direction = -vec3(M.Column(2)); // -Z local
			vec3 up_direction = vec3(0.0f, 0.0f, 1.0f); // Z world

			if (key_a)
                pos -= orth_direction * dt * moveSpeed;

            if (key_d)
                pos += orth_direction * dt * moveSpeed;

            if (key_w)
                pos += forward_direction * dt * moveSpeed;

            if (key_s)
                pos -= forward_direction * dt * moveSpeed;

            if (key_q)
                pos -= up_direction * dt * moveSpeed;

            if (key_e)
                pos += up_direction * dt * moveSpeed;

            pCamera->SetPosition(&pos);
        }

		if (mouse)
		{
//            const float rotSpeed = 7.0f;
//            vec3 rot;
//            pCamera->GetRotation(&rot);
//            rot.y += dx * dt * rotSpeed;
//            rot.x += dy * dt * rotSpeed;
//            pCamera->SetRotation(&rot);
		}

        //pCore->Log("RenderWidget::onUpdate()", RENDER_MASTER::LOG_TYPE::NORMAL);
    }
}

void RenderWidget::onEngineInited(ICore *pCoreIn)
{
    pCore = pCoreIn;

    pCore->GetSubSystem((ISubSystem **)&pCoreRender, RENDER_MASTER::SUBSYSTEM_TYPE::CORE_RENDER);
    pCore->GetSubSystem((ISubSystem **)&pSceneManager, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);

    pSceneManager->GetDefaultCamera(&pCamera);
}

void RenderWidget::onEngineClosed(ICore *pCoreIn)
{
    Q_UNUSED( pCoreIn )
    pCore = nullptr;
}
