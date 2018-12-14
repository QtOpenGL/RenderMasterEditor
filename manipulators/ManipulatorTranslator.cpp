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
		vec3 N;
		mat4 axisTransform = editor->GetSelectionTransform();
		N = axisTransform.Column3(2);

		vec3 origin = axisTransform.Column3(3);

		Plane plane(N, origin);

		_drawPlane(plane, pCamera, screen, pRender, pCoreRender);
	}
	{
		ShaderRequirement req;
		req.attributes = INPUT_ATTRUBUTE::POSITION;

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

		pCoreRender->SetDepthTest(0);

		auto draw_axis = [&](AXIS type) -> void
		{
			mat4 correctionMat(1.0f);

			if (type == AXIS::Y)
			{
				correctionMat.el_2D[0][0] = 0.0f;
				correctionMat.el_2D[1][1] = 0.0f;
				correctionMat.el_2D[1][0] = 1.0f;
				correctionMat.el_2D[0][1] = 1.0f;
			} else if (type == AXIS::Z)
			{
				correctionMat.el_2D[0][0] = 0.0f;
				correctionMat.el_2D[2][2] = 0.0f;
				correctionMat.el_2D[2][0] = 1.0f;
				correctionMat.el_2D[0][2] = 1.0f;
			}
			mat4 MVP = VP * selectionWorldTransform * distanceScaleMat * correctionMat;
			shader->SetMat4Parameter("MVP", &MVP);

			vec4 color = vec4(0,0,0,1.0f);
			color.xyzw[(int)type] = 1.0f;
			shader->SetVec4Parameter("main_color", &color);
			shader->FlushParameters();
			pCoreRender->Draw(_pAxesMesh);
			pCoreRender->Draw(_pAxesArrowMesh);
		};

		draw_axis(AXIS::X);
		draw_axis(AXIS::Y);
		draw_axis(AXIS::Z);

		shader->Release();
	}
}


