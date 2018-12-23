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

    setMouseTracking(true);

	setAttribute(Qt::WA_NoBackground);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_PaintUnclipped);
	setAttribute(Qt::WA_NativeWindow);
	setFocusPolicy(Qt::StrongFocus);

	connect(eng, &EngineGlobal::EngineInited, this, &RenderWidget::onEngineInited, Qt::DirectConnection);
	connect(eng, &EngineGlobal::EngineBeforeClose, this, &RenderWidget::onEngineClosed, Qt::DirectConnection);
	connect(eng, &EngineGlobal::OnRender, this, &RenderWidget::onRender, Qt::DirectConnection);
	connect(eng, &EngineGlobal::OnUpdate, this, &RenderWidget::onUpdate, Qt::DirectConnection);

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
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
	{
		//qDebug() << "RenderWidget::mousePressEvent(QMouseEvent *event)";
		rightMousePressed = 1;
		oldMousePos = event->pos();
	}

	if (event->button() == Qt::LeftButton)
	{
		//qDebug() << "RenderWidget::mouseMoveEvent(QMouseEvent *event) (" << event->pos().x()<< event->pos().y() << ")";
		leftMousePressed = 1;
		leftMouseClick = 1;
		captureX = uint(event->pos().x());
		captureY = uint(event->pos().y());
	}
	QWidget::mousePressEvent(event);
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton)
		rightMousePressed = 0;

	if (event->button() == Qt::LeftButton)
	{
		leftMousePressed = 0;

		ManipulatorBase *manipulator = editor->CurrentManipulator();
		if (manipulator)
			manipulator->mouseButtonUp();
	}
	QWidget::mouseReleaseEvent(event);
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
	mousePos = event->pos();
	deltaMousePos = mousePos - oldMousePos;
	oldMousePos = mousePos;

	int w = size().width();
	int h = size().height();
	normalizedMousePos = vec2((float)mousePos.x() / w, (float)(h - mousePos.y()) / h);

	QWidget::mouseMoveEvent(event);
}

void RenderWidget::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_W) {keyW = 1; }
	if (event->key() == Qt::Key_S) {keyS = 1; }
	if (event->key() == Qt::Key_A) {keyA = 1; }
	if (event->key() == Qt::Key_D) {keyD = 1; }
	if (event->key() == Qt::Key_Q) {keyQ = 1; }
	if (event->key() == Qt::Key_E) {keyE = 1; }
	if (event->key() == Qt::Key_Alt) {keyAlt = 1; }
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_W) {keyW = 0; }
	if (event->key() == Qt::Key_S) {keyS = 0; }
	if (event->key() == Qt::Key_A) {keyA = 0; }
	if (event->key() == Qt::Key_D) {keyD = 0; }
	if (event->key() == Qt::Key_Q) {keyQ = 0; }
	if (event->key() == Qt::Key_E) {keyE = 0; }
	if (event->key() == Qt::Key_Alt) {keyAlt = 0; }
}

void RenderWidget::drawManipulator(ICamera *pCamera)
{
	if (!editor->IsSomeObjectSelected())
		return;

	ManipulatorBase *m = editor->CurrentManipulator();

	if (m)
		m->render(pCamera, rect(), pRender, pCoreRender);
}

void RenderWidget::RenderWidget::drawGrid(const mat4 &VP)
{
	ShaderRequirement req;
	req.attributes = INPUT_ATTRUBUTE::POSITION;

	IShader *shader{nullptr};
	pRender->PreprocessStandardShader(&shader, &req);
	if (!shader)
		return;

	shader->AddRef();

	pCoreRender->SetShader(shader);

	shader->SetMat4Parameter("MVP", &VP);
	shader->SetVec4Parameter("main_color", &vec4(0.1f, 0.1f, 0.1f, 1.0f));
	shader->FlushParameters();

	pCoreRender->SetDepthTest(1);

	pCoreRender->SetMesh(pGridMesh);
	pCoreRender->Draw(pGridMesh);

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

	mat4 VP;
	float aspect = (float)rect().width() / rect().height();
	pCamera->GetViewProjectionMatrix(&VP, aspect);

	// default engine frame
	{
		pCore->RenderFrame(&h, pCamera);
	}

	eng->getCoreRender()->PushStates();
	{
		// manipulator, grid
		{
			drawGrid(VP);
			drawManipulator(pCamera);

			pCoreRender->SwapBuffers();
		}

		ManipulatorBase *manipulator = editor->CurrentManipulator();

		// picking
		if (!keyAlt && // not in orbit mode
			((!manipulator && leftMouseClick) ||
			(leftMouseClick && manipulator && !manipulator->isMouseIntersects(normalizedMousePos))))
		{
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

			eng->getCoreRender()->SetDepthTest(1);

			render->RenderPassIDPass(pCamera, idTex, depthIdTex);

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
	}
	eng->getCoreRender()->PopStates();

	leftMouseClick = 0;
}

void RenderWidget::onUpdate(float dt)
{
	if (pCore)
		pCore->Update();

	ICamera *pCamera = nullptr;
	pSceneManager->GetDefaultCamera(&pCamera);
	if (pCamera)
	{

	// update manipulator
	ManipulatorBase *m = editor->CurrentManipulator();
	if (m)
	{
		if (leftMouseClick)
			m->mouseButtonDown(pCamera, rect(), normalizedMousePos);
		else
			m->update(pCamera, rect(), normalizedMousePos);
	}

	vec3 pos;
	pCamera->GetPosition(&pos);

	quat rot;
	pCamera->GetRotation(&rot);

	if (isFocusing) // we are still focus
	{
		pos = lerp(pos, focusCameraPosition, dt * 10.0f);
		pCamera->SetPosition(&pos);

		if ((pos - focusCameraPosition).Lenght() < 0.01f)
			isFocusing = 0;
	} else if (keyAlt) // orbit mode
	{
		if (rightMousePressed || leftMousePressed)
		{
			vec3 dPos = pos - focusCenter;
			Spherical sPos = ToSpherical(dPos);

			if (rightMousePressed) // zoom
			{
				sPos.r += (-deltaMousePos.x() - deltaMousePos.y()) * sPos.r * dt * zoomSpeed;
				sPos.r = clamp(sPos.r, 0.01f, std::numeric_limits<float>::max());
				pos = ToCartesian(sPos) + focusCenter;
				pCamera->SetPosition(&pos);
			}

			if (leftMousePressed) // orbit
			{
				sPos.phi -= deltaMousePos.x() * dt * orbitHorSpeed;
				sPos.theta -= deltaMousePos.y() * dt * orbitVertSpeed;
				sPos.theta = clamp(sPos.theta, 0.01f, 3.14f);
				pos = ToCartesian(sPos) + focusCenter;
				pCamera->SetPosition(&pos);

				mat4 new_rot_mat;
				lookAtCamera(new_rot_mat, pos, focusCenter);
				new_rot_mat.Transpose();
				quat new_quat = quat(new_rot_mat);
				pCamera->SetRotation(&new_quat);
			}
		}

	} else if (rightMousePressed)
	{
		mat4 M;
		pCamera->GetModelMatrix(&M);

		vec3 orth_direction = vec3(M.Column(0)); // X local
		vec3 forward_direction = -vec3(M.Column(2)); // -Z local
		vec3 up_direction = vec3(0.0f, 0.0f, 1.0f); // Z world

		if (keyA)
			pos -= orth_direction * dt * moveSpeed;

		if (keyD)
			pos += orth_direction * dt * moveSpeed;

		if (keyW)
			pos += forward_direction * dt * moveSpeed;

		if (keyS)
			pos -= forward_direction * dt * moveSpeed;

		if (keyQ)
			pos -= up_direction * dt * moveSpeed;

		if (keyE)
			pos += up_direction * dt * moveSpeed;

		pCamera->SetPosition(&pos);

		if (rightMousePressed)
		{
			quat dxRot = quat(-deltaMousePos.y() * dt * rotateSpeed, 0.0f, 0.0f);
			quat dyRot = quat(0.0f, 0.0f,-deltaMousePos.x() * dt * rotateSpeed);
			rot = dyRot * rot * dxRot;
			pCamera->SetRotation(&rot);
		}

		focusCenter = pos + forward_direction * focusDistance;
	}
	}

	deltaMousePos = {};
}

void RenderWidget::onFocusAtSelected(const vec3& worldCenter, const RENDER_MASTER::AABB& aabb)
{
	if (!pCore)
		return; // engine not loaded

	ICamera *pCamera = nullptr;
	pSceneManager->GetDefaultCamera(&pCamera);

	if (!pCamera)
		return; // no camera

	isFocusing = 1;

	mat4 ModelMat;
	pCamera->GetModelMatrix(&ModelMat);
	vec3 view = -ModelMat.Column3(2).Normalized();

	float fovDegs;
	pCamera->GetFovAngle(&fovDegs);
	float fovRads = fovDegs * DEGTORAD;

    float objectHalfSize = std::max(std::max(abs(aabb.maxX - aabb.minX), abs(aabb.maxY - aabb.minY)), abs(aabb.maxZ - aabb.minZ));

	float distance = objectHalfSize / tan(fovRads * 0.5f);

	focusCenter = worldCenter;
	focusCameraPosition = worldCenter - view * distance;

	vec3 cameraPos;
	pCamera->GetPosition(&cameraPos);
	vec3 dPos = cameraPos - focusCenter;
	Spherical sPos = ToSpherical(dPos);
	focusDistance = sPos.r;

	//qDebug() << "RenderWidget::onFocusAtSelected(): target = " << vec3ToString(focusingTargetPosition);
}

void RenderWidget::onEngineInited(ICore *pCoreIn)
{
	pCore = pCoreIn;

	pCore->GetSubSystem((ISubSystem **)&pCoreRender, RENDER_MASTER::SUBSYSTEM_TYPE::CORE_RENDER);
	pCore->GetSubSystem((ISubSystem **)&pRender, RENDER_MASTER::SUBSYSTEM_TYPE::RENDER);
	pCore->GetSubSystem((ISubSystem **)&pSceneManager, RENDER_MASTER::SUBSYSTEM_TYPE::SCENE_MANAGER);
	pCore->GetSubSystem((ISubSystem **)&pResMan, RENDER_MASTER::SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	pResMan->LoadMesh(&pGridMesh, "std#grid");
	pGridMesh->AddRef();
}

void RenderWidget::onEngineClosed(ICore *pCoreIn)
{
	Q_UNUSED( pCoreIn )

	pGridMesh->Release();

	pCore = nullptr;
}
