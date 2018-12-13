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
}

void ManipulatorBase::onEngineClosed(RENDER_MASTER::ICore *pCore)
{
	_free();
}
