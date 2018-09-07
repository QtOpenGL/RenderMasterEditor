#include "renderwidget.h"
#include "ui_dd3d11widget.h"
#include "EngineGlobal.h"
#include "Engine.h"
#include <QMouseEvent>
#include <QDebug>

using namespace RENDER_MASTER;

extern EngineGlobal* eng;
extern EditorGlobal* editor;

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

	connect(editor, &EditorGlobal::ManipulatorPressed, this, &RenderWidget::onManipulatorPressed, Qt::DirectConnection);

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

void RenderWidget::_draw_axes(const mat4& VP)
{
	if (MANIPULATOR::TRANSLATE != _currentManipulator)
		return;

	INPUT_ATTRUBUTE a;
	_pAxesMesh->GetAttributes(&a);

	ICoreShader *shader{nullptr};
	ShaderRequirement req;
	req.attributes = a;
	req.alphaTest = false;
	pRender->GetShader(&shader, &req);
	if (!shader) return;


	pCoreRender->SetShader(shader);

	float z = VP.el_2D[2][3];

	uint w = rect().height();

	mat4 M;
	M.el_2D[0][0] = 80.0f * z / w;
	M.el_2D[1][1] = 80.0f * z / w;
	M.el_2D[2][2] = 80.0f * z / w;

	params.MVP = VP * M;
	params.main_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	pCoreRender->SetUniform(paramsBuffer, &params.main_color);
	pCoreRender->SetUniformBufferToShader(paramsBuffer, 0);

	pCoreRender->SetDepthState(false);

	pCoreRender->SetMesh(_pAxesMesh);
	pCoreRender->Draw(_pAxesMesh);

	pCoreRender->SetMesh(_pAxesArrowMesh);
	pCoreRender->Draw(_pAxesArrowMesh);

	pCoreRender->SetDepthState(true);
}

void RenderWidget::RenderWidget::_draw_grid(const mat4 &VP)
{
	INPUT_ATTRUBUTE a;
	_pGridMesh->GetAttributes(&a);

	ICoreShader *shader{nullptr};
	ShaderRequirement req;
	req.attributes = a;
	req.alphaTest = false;
	pRender->GetShader(&shader, &req);
	if (!shader) return;
	pCoreRender->SetShader(shader);

	params.MVP = VP;
	params.main_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);

	pCoreRender->SetUniform(paramsBuffer, &params.main_color);
	pCoreRender->SetUniformBufferToShader(paramsBuffer, 0);


	pCoreRender->SetDepthState(true);

	pCoreRender->SetMesh(_pGridMesh);
	pCoreRender->Draw(_pGridMesh);
}

void RenderWidget::onRender()
{
    if (pCore)
    {
        pCore->RenderFrame(&h, pCamera);

		eng->getCoreRender()->PushStates();
		{
			float aspect = (float)rect().width() / rect().height();

			mat4 VP;
			pCamera->GetViewProjectionMatrix(&VP, aspect);

			_draw_grid(VP);
			_draw_axes(VP);
		}
		eng->getCoreRender()->PopStates();

		pCoreRender->SwapBuffers();
    }
}

void RenderWidget::onUpdate(float dt)
{
    if (pCore)
    {
        {
			const float moveSpeed = 40.0f;

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
			const float rotSpeed = 13.0f;
			quat rot;
			pCamera->GetRotation(&rot);
			quat dxRot = quat(-dy * dt * rotSpeed, 0.0f, 0.0f);
			quat dyRot = quat(0.0f, 0.0f,-dx * dt * rotSpeed);
			rot = dyRot * rot * dxRot;
			pCamera->SetRotation(&rot);

			dx = 0.0f;
			dy = 0.0f;
		}

        //pCore->Log("RenderWidget::onUpdate()", RENDER_MASTER::LOG_TYPE::NORMAL);
    }
}

void RenderWidget::onManipulatorPressed(MANIPULATOR m) { _currentManipulator = m; }

void RenderWidget::onEngineInited(ICore *pCoreIn)
{
	pCore = pCoreIn;

	pCore->GetSubSystem((ISubSystem **)&pCoreRender, RENDER_MASTER::SUBSYSTEM_TYPE::CORE_RENDER);
	pCore->GetSubSystem((ISubSystem **)&pRender, RENDER_MASTER::SUBSYSTEM_TYPE::RENDER);
	pCore->GetSubSystem((ISubSystem **)&pSceneManager, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);
	pCore->GetSubSystem((ISubSystem **)&pResMan, RENDER_MASTER::SUBSYSTEM_TYPE::RESOURCE_MANAGER);

    pSceneManager->GetDefaultCamera(&pCamera);

	pResMan->GetDefaultResource((IResource**)&_pAxesMesh, DEFAULT_RES_TYPE::AXES);
	pResMan->GetDefaultResource((IResource**)&_pAxesArrowMesh, DEFAULT_RES_TYPE::AXES_ARROWS);
	pResMan->GetDefaultResource((IResource**)&_pGridMesh, DEFAULT_RES_TYPE::GRID);

	pCoreRender->CreateUniformBuffer(&paramsBuffer, sizeof(EveryFrameParameters));
}

void RenderWidget::onEngineClosed(ICore *pCoreIn)
{
    Q_UNUSED( pCoreIn )

	pResMan->DecrementRef(_pAxesMesh);
	pResMan->DecrementRef(_pAxesArrowMesh);
	pResMan->DecrementRef(_pGridMesh);

    pCore = nullptr;
}
