#include "renderwidget.h"
#include "ui_dd3d11widget.h"
#include "EngineGlobal.h"
#include "Engine.h"
#include "common.h"
#include <QMouseEvent>
#include <QDebug>
#include <math.h>
#include <algorithm>

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
	connect(editor, &EditorGlobal::OnFocusAtSelectded, this, &RenderWidget::onFocusAtSelected, Qt::DirectConnection);

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
	if (event->button() == Qt::RightButton)
	{
		//qDebug() << "RenderWidget::mousePressEvent(QMouseEvent *event)";
		rightMouse = 1;
		lastMousePos = event->pos();
	}
	QWidget::mousePressEvent(event);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		//qDebug() << "RenderWidget::mouseReleaseEvent(QMouseEvent *event)";
		rightMouse = 0;
	}
	QWidget::mouseReleaseEvent(event);
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (pCore && rightMouse /*&& event->button() == Qt::RightButton*/)
	{
		dx = event->pos().x() - lastMousePos.x();
		dy = event->pos().y() - lastMousePos.y();
		lastMousePos = event->pos();
		//qDebug() << "RenderWidget::mouseMoveEvent(QMouseEvent *event)";
	}
	QWidget::mouseMoveEvent(event);
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

void RenderWidget::_draw_axes(const mat4& VP, ICamera *pCamera)
{
	if (MANIPULATOR::TRANSLATE != _currentManipulator)
		return;

	if (!editor->IsSomeObjectSelected())
		return;

	ShaderRequirement req;
	req.attributes = INPUT_ATTRUBUTE::POSITION | INPUT_ATTRUBUTE::COLOR;

	IShader *shader{nullptr};
	pRender->PreprocessStandardShader(&shader, &req);
	if (!shader)
		return;

	shader->AddRef();

	ICoreShader *coreShader = nullptr;
	shader->GetCoreShader(&coreShader);
	pCoreRender->SetShader(coreShader);

	ICoreConstantBuffer *coreCB;
	parameters->GetCoreBuffer(&coreCB);

	ICoreMesh *coreAxesMesh;
	_pAxesMesh->GetCoreMesh(&coreAxesMesh);

	ICoreMesh *coreAxesArrows;
	_pAxesArrowMesh->GetCoreMesh(&coreAxesArrows);

	uint h = rect().height();

	mat4 modelWorldTransform = editor->GetSelectionCeneter();

	mat4 axisWorldTransform;

	axisWorldTransform.el_2D[0][3] = modelWorldTransform.el_2D[0][3];
	axisWorldTransform.el_2D[1][3] = modelWorldTransform.el_2D[1][3];
	axisWorldTransform.el_2D[2][3] = modelWorldTransform.el_2D[2][3];

	vec4 view4 = VP * vec4(axisWorldTransform.el_2D[0][3], axisWorldTransform.el_2D[1][3], axisWorldTransform.el_2D[2][3], 1.0f);
	vec3 view(view4);
	float dist = view.Lenght();

	axisWorldTransform.el_2D[0][0] = (80.0f / h) * dist;
	axisWorldTransform.el_2D[1][1] = (80.0f / h) * dist;
	axisWorldTransform.el_2D[2][2] = (80.0f / h) * dist;

	params.MVP = VP * axisWorldTransform;
	params.main_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	pCoreRender->SetUniformBufferData(coreCB, &params.main_color);
	pCoreRender->SetUniformBuffer(coreCB, 0);

	pCoreRender->SetDepthState(false);

	pCoreRender->SetMesh(coreAxesMesh);
	pCoreRender->Draw(coreAxesMesh);

	pCoreRender->SetMesh(coreAxesArrows);
	pCoreRender->Draw(coreAxesArrows);

	shader->Release();
}

void RenderWidget::RenderWidget::_draw_grid(const mat4 &VP, ICamera *pCamera)
{
	ShaderRequirement req;
	req.attributes = INPUT_ATTRUBUTE::POSITION;

	IShader *shader{nullptr};
	pRender->PreprocessStandardShader(&shader, &req);
	if (!shader)
		return;

	shader->AddRef();

	ICoreShader *coreShader = nullptr;
	shader->GetCoreShader(&coreShader);

	ICoreConstantBuffer *coreCB;
	parameters->GetCoreBuffer(&coreCB);

	ICoreMesh *coreGrid;
	_pGridMesh->GetCoreMesh(&coreGrid);

	params.MVP = VP;
	params.main_color = vec4(0.1f, 0.1f, 0.1f, 1.0f);

	pCoreRender->SetShader(coreShader);

	pCoreRender->SetUniformBufferData(coreCB, &params.main_color);
	pCoreRender->SetUniformBuffer(coreCB, 0);

	pCoreRender->SetDepthState(true);

	pCoreRender->SetMesh(coreGrid);
	pCoreRender->Draw(coreGrid);

	shader->Release();
}

void RenderWidget::onRender()
{
	if (!pCore)
		return; // engine not loaded

	ICamera *pCamera = nullptr;
	pSceneManager->GetDefaultCamera(&pCamera);

	if (!pCamera)
		return; // no camera

	pCore->RenderFrame(&h, pCamera);

	eng->getCoreRender()->PushStates();
	{
		float aspect = (float)rect().width() / rect().height();

		mat4 VP;
		pCamera->GetViewProjectionMatrix(&VP, aspect);

		_draw_grid(VP, pCamera);
		_draw_axes(VP, pCamera);
	}
	eng->getCoreRender()->PopStates();

	pCoreRender->SwapBuffers();
}

void RenderWidget::onUpdate(float dt)
{
	if (!pCore)
		return; // engine not loaded

	ICamera *pCamera = nullptr;
	pSceneManager->GetDefaultCamera(&pCamera);

	if (!pCamera)
		return; // no camera

	vec3 pos;
	pCamera->GetPosition(&pos);

	if (isFocusing) // we are still focus
	{
		pos = lerp(pos, focusingTargetPosition, dt * 10.0f);
		pCamera->SetPosition(&pos);

		if ((pos - focusingTargetPosition).Lenght() < 0.01f)
			isFocusing = 0;
	} else
	{
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

		if (rightMouse)
		{
			quat rot;
			pCamera->GetRotation(&rot);
			quat dxRot = quat(-dy * dt * rotateSpeed, 0.0f, 0.0f);
			quat dyRot = quat(0.0f, 0.0f,-dx * dt * rotateSpeed);
			rot = dyRot * rot * dxRot;
			pCamera->SetRotation(&rot);

			dx = 0.0f;
			dy = 0.0f;
		}
	}
}

void RenderWidget::onManipulatorPressed(MANIPULATOR m) { _currentManipulator = m; }

void RenderWidget::onFocusAtSelected(const vec3& worldCenter, const RENDER_MASTER::AABB& aabb)
{
	if (!pCore)
		return; // engine not loaded

	ICamera *pCamera = nullptr;
	pSceneManager->GetDefaultCamera(&pCamera);

	if (!pCamera)
		return; // no camera

	isFocusing = 1;

	// calculate target position
	//

	//
	mat4 ModelMat;
	pCamera->GetModelMatrix(&ModelMat);
	vec3 view = -ModelMat.Column3(2).Normalized();

	//
	float fovDegs;
	pCamera->GetFovAngle(&fovDegs);
	float fovRads = fovDegs * DEGTORAD;

	//
	float objectHalfSize = max(max(abs(aabb.maxX - aabb.minX), abs(aabb.maxY - aabb.minY)), abs(aabb.maxZ - aabb.minZ));

	//
	float distance = objectHalfSize / tan(fovRads * 0.5f);

	focusingTargetPosition = worldCenter - view * distance;

	qDebug() << "RenderWidget::onFocusAtSelected(): target = " << vec3ToString(focusingTargetPosition);

}

void RenderWidget::onEngineInited(ICore *pCoreIn)
{
	pCore = pCoreIn;

	pCore->GetSubSystem((ISubSystem **)&pCoreRender, RENDER_MASTER::SUBSYSTEM_TYPE::CORE_RENDER);
	pCore->GetSubSystem((ISubSystem **)&pRender, RENDER_MASTER::SUBSYSTEM_TYPE::RENDER);
	pCore->GetSubSystem((ISubSystem **)&pSceneManager, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);
	pCore->GetSubSystem((ISubSystem **)&pResMan, RENDER_MASTER::SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	pResMan->CreateConstantBuffer(&parameters, sizeof(EveryFrameParameters));
	parameters->AddRef();

	pResMan->LoadMesh(&_pAxesMesh, "std#axes");
	_pAxesMesh->AddRef();

	pResMan->LoadMesh(&_pAxesArrowMesh, "std#axes_arrows");
	_pAxesArrowMesh->AddRef();

	pResMan->LoadMesh(&_pGridMesh, "std#grid");
	_pGridMesh->AddRef();
}

void RenderWidget::onEngineClosed(ICore *pCoreIn)
{
	Q_UNUSED( pCoreIn )

	parameters->Release();
	_pAxesArrowMesh->Release();
	_pAxesMesh->Release();
	_pGridMesh->Release();

	pCore = nullptr;
}
