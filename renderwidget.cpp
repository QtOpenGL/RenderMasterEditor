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
		lastMousePos = event->pos();
	}

	if (event->button() == Qt::LeftButton)
	{
		qDebug() << "RenderWidget::mouseMoveEvent(QMouseEvent *event) (" << event->pos().x()<< event->pos().y() << ")";
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
		rightMousePressed = 0;
	}
	QWidget::mouseReleaseEvent(event);
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
	mousePosX = event->pos().x();
	mousePosY = event->pos().y();

	int w = size().width();
	int h = size().height();
	normalizedMousePos = vec2((float)mousePosX / w, (float)(h - mousePosY) / h);

	if (pCore && rightMousePressed)
	{
		dx = event->pos().x() - lastMousePos.x();
		dy = event->pos().y() - lastMousePos.y();
		lastMousePos = event->pos();

	}

	needCaptureId = 0;

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

		// picking. render id's
		if ((!manipulator && needCaptureId) ||
			(needCaptureId && manipulator && !manipulator->isMouseIntersects(normalizedMousePos)))
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

	// update manipulator
	ManipulatorBase *m = editor->CurrentManipulator();
	if (m)
		m->update(pCamera, rect(), pRender, pCoreRender, normalizedMousePos);

	// update camera
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

		if (rightMousePressed)
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

	pResMan->LoadMesh(&_pGridMesh, "std#grid");
	_pGridMesh->AddRef();
}

void RenderWidget::onEngineClosed(ICore *pCoreIn)
{
	Q_UNUSED( pCoreIn )

	_pGridMesh->Release();

	pCore = nullptr;
}
