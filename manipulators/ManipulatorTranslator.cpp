#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;

ManipulatorTranslator::~ManipulatorTranslator()
{
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
	mat4 VP;
	float aspect = (float)screen.width() / screen.height();
	pCamera->GetViewProjectionMatrix(&VP, aspect);

	{
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
	{
		ShaderRequirement req;
		req.attributes = INPUT_ATTRUBUTE::POSITION | INPUT_ATTRUBUTE::COLOR;

		IShader *shader = nullptr;
		pRender->PreprocessStandardShader(&shader, &req);
		if (!shader)
			return;

		shader->AddRef();

		pCoreRender->SetShader(shader);

		mat4 selectionWorldTransform = editor->GetSelectionTransform();

		vec4 view4 = VP * vec4(selectionWorldTransform.el_2D[0][3], selectionWorldTransform.el_2D[1][3], selectionWorldTransform.el_2D[2][3], 1.0f);
		vec3 view(view4);
		float dist = view.Lenght();

		mat4 distanceScaleMat;
		uint h = screen.height();
		distanceScaleMat.el_2D[0][0] = (80.0f / h) * dist;
		distanceScaleMat.el_2D[1][1] = (80.0f / h) * dist;
		distanceScaleMat.el_2D[2][2] = (80.0f / h) * dist;

		mat4 MVP = VP * selectionWorldTransform * distanceScaleMat;
		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &vec4(1.0f, 1.0f, 1.0f, 1.0f));
		shader->FlushParameters();

		pCoreRender->SetDepthTest(0);

		pCoreRender->Draw(_pAxesMesh);
		pCoreRender->Draw(_pAxesArrowMesh);

		shader->Release();
	}
}


