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

		//qDebug() << "RenderWidget::mouseMoveEvent(QMouseEvent *event) (" << event->pos().x()<< event->pos().y() << ")";
		//needCaptureId = 1;
		//captureX = uint(lastMousePos.x());
		//captureY = uint(lastMousePos.y());
	}

	if (event->button() == Qt::LeftButton)
	{
		//qDebug() << "RenderWidget::mouseMoveEvent(QMouseEvent *event) (" << event->pos().x()<< event->pos().y() << ")";
		needCaptureId = 1;
		captureX = uint(event->pos().x());
		captureY = uint(event->pos().y());
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

	pCoreRender->SetShader(shader);

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

	mat4 MVP = VP * axisWorldTransform;
	shader->SetMat4Parameter("MVP", &MVP);
	shader->SetVec4Parameter("main_color", &vec4(1.0f, 1.0f, 1.0f, 1.0f));
	shader->FlushParameters();

	pCoreRender->SetDepthState(false);

	pCoreRender->SetMesh(_pAxesMesh);
	pCoreRender->Draw(_pAxesMesh);

	pCoreRender->SetMesh(_pAxesArrowMesh);
	pCoreRender->Draw(_pAxesArrowMesh);

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

	pCoreRender->SetShader(shader);

	shader->SetVec4Parameter("main_color", &vec4(0.1f, 0.1f, 0.1f, 1.0f));
	shader->FlushParameters();

	pCoreRender->SetDepthState(true);

	pCoreRender->SetMesh(_pGridMesh);
	pCoreRender->Draw(_pGridMesh);

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

	pCoreRender->SwapBuffers();

	if (needCaptureId)
	{
		needCaptureId = 0;

		IRender *render;
		pCore->GetSubSystem((ISubSystem**)&render, SUBSYSTEM_TYPE::RENDER);

		int w = size().width();
		int h = size().height();

		ITexture *idTex;
		render->GetRenderTexture2D(&idTex, w, h, TEXTURE_FORMAT::R32UI);
		idTex->AddRef();

		ITexture *depthIdTex;
		render->GetRenderTexture2D(&depthIdTex, w, h, TEXTURE_FORMAT::D24S8);
		depthIdTex->AddRef();

		render->RenderPassIDPass(pCamera, idTex, depthIdTex);

		//
		//i->GetMousePos(&captureX, &captureY);
		ICoreTexture *coreTex;
		idTex->GetCoreTexture(&coreTex);
		uint data = 0;
		uint read = 0;
		eng->getCoreRender()->ReadPixel2D(coreTex, &data, &read, captureX, captureY);
		if (read > 0)
		{
			qDebug() << "Captured Id = " <<  data;

			ISceneManager *sm;
			pCore->GetSubSystem((ISubSystem**)&sm, SUBSYSTEM_TYPE::SCENE_MANAGER);

			IGameObject *go;
			sm->FindChildById(&go, data);

			if (go)
			{
				std::vector<IGameObject*> gos = {go};
				editor->ChangeSelection(gos);
			} else
			{
				std::vector<IGameObject*> gos;
				editor->ChangeSelection(gos);
			}

		}

		depthIdTex->Release();
		idTex->Release();
		render->ReleaseRenderTexture2D(idTex);
		render->ReleaseRenderTexture2D(depthIdTex);
	}

	eng->getCoreRender()->PopStates();
}

void RenderWidget::onUpdate(float dt)
{
	if (!pCore)
		return; // engine not loaded

	pCore->Update();

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

	_pAxesArrowMesh->Release();
	_pAxesMesh->Release();
	_pGridMesh->Release();

	pCore = nullptr;
}
