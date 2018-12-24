#include "ManipulatorBase.h"
#include "editorglobal.h"
#include "EngineGlobal.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;

ManipulatorBase::ManipulatorBase(RENDER_MASTER::ICore *pCore)
{
	IResourceManager *pResMan;
	pCore->GetSubSystem((ISubSystem **)&pResMan, RENDER_MASTER::SUBSYSTEM_TYPE::RESOURCE_MANAGER);

	pResMan->LoadMesh(&_pAxesMesh, "std#axes");
	_pAxesMesh->AddRef();

	pResMan->LoadMesh(&_pAxesArrowMesh, "std#axes_arrows");
	_pAxesArrowMesh->AddRef();

	pResMan->LoadMesh(&_pGrid, "std#grid");
	_pGrid->AddRef();

	pResMan->LoadMesh(&_pQuadLines, "std#quad_lines");
	_pQuadLines->AddRef();

	pResMan->LoadMesh(&_pQuad, "std#plane");
	_pQuad->AddRef();

	connect(eng, &EngineGlobal::EngineBeforeClose, this, &ManipulatorBase::onEngineClosed, Qt::DirectConnection);
}

void ManipulatorBase::_free()
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

	if (_pGrid)
	{
		_pGrid->Release();
		_pGrid = nullptr;
	}

	if (_pQuadLines)
	{
		_pQuadLines->Release();
		_pQuadLines = nullptr;
	}

	if (_pQuad)
	{
		_pQuad->Release();
		_pQuad = nullptr;
	}
}

void ManipulatorBase::_drawPlane(const Plane &plane, ICamera *pCamera, const QRect &screen, RENDER_MASTER::IRender *pRender, ICoreRender *pCoreRender)
{
	mat4 VP;
	float aspect = (float)screen.width() / screen.height();
	pCamera->GetViewProjectionMatrix(&VP, aspect);

	ShaderRequirement req;
	req.attributes = INPUT_ATTRUBUTE::POSITION;

	IShader *shader{nullptr};
	pRender->PreprocessStandardShader(&shader, &req);
	if (!shader)
		return;

	shader->AddRef();

	pCoreRender->SetShader(shader);

	mat4 scaleMat;
	scaleMat.el_2D[0][0] = 0.2f;
	scaleMat.el_2D[1][1] = 0.2f;
	scaleMat.el_2D[2][2] = 0.2f;

	mat4 selectionWorldTransform = editor->GetSelectionTransform();
	mat4 MVP = VP * selectionWorldTransform * scaleMat;

	shader->SetMat4Parameter("MVP", &MVP);
	shader->FlushParameters();

	pCoreRender->SetDepthTest(1);

	pCoreRender->SetMesh(_pGrid);
	pCoreRender->Draw(_pGrid);

	shader->Release();
}

void ManipulatorBase::onEngineClosed(RENDER_MASTER::ICore *pCore)
{
	_free();
}
