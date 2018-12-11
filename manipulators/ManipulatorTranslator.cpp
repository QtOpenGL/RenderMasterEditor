#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;


void ManipulatorTranslator::_free()
{
	if (_pAxesMesh)
	{
		_pAxesMesh->Release();
		_pAxesMesh = nullptr;
	}

	if (_pAxesArrowMesh)
	{
		_pAxesArrowMesh->Release();
		_pAxesArrowMesh = nullptr;
	}
}

ManipulatorTranslator::ManipulatorTranslator(RENDER_MASTER::ICore *pCore)
{
	IResourceManager *pResMan;
	pCore->GetSubSystem((ISubSystem **)&pResMan, RENDER_MASTER::SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	pResMan->LoadMesh(&_pAxesMesh, "std#axes");
	_pAxesMesh->AddRef();

	pResMan->LoadMesh(&_pAxesArrowMesh, "std#axes_arrows");
	_pAxesArrowMesh->AddRef();

	connect(eng, &EngineGlobal::EngineBeforeClose, this, &ManipulatorTranslator::onEngineClosed, Qt::DirectConnection);
}

ManipulatorTranslator::~ManipulatorTranslator()
{
	_free();
}

bool ManipulatorTranslator::isIntersects(const QPointF &mousePos)
{
	return false;
}

void ManipulatorTranslator::beginDrag(const QPointF &mousePos)
{

}

void ManipulatorTranslator::drag(const QPointF &mousePos)
{

}

void ManipulatorTranslator::endDrag()
{

}

void ManipulatorTranslator::render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *pRender, RENDER_MASTER::ICoreRender *pCoreRender)
{
	ShaderRequirement req;
	req.attributes = INPUT_ATTRUBUTE::POSITION | INPUT_ATTRUBUTE::COLOR;

	mat4 VP;
	float aspect = (float)screen.width() / screen.height();
	pCamera->GetViewProjectionMatrix(&VP, aspect);

	IShader *shader = nullptr;
	pRender->PreprocessStandardShader(&shader, &req);
	if (!shader)
		return;

	shader->AddRef();

	pCoreRender->SetShader(shader);

	uint h = screen.height();

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

	pCoreRender->SetDepthTest(0);

	pCoreRender->Draw(_pAxesMesh);
	pCoreRender->Draw(_pAxesArrowMesh);

	shader->Release();

}

void ManipulatorTranslator::onEngineClosed(ICore *pCore)
{
	_free();
}
