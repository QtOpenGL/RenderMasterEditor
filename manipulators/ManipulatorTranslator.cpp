#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"
#include "common.h"

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

void ManipulatorTranslator::render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *pRender, RENDER_MASTER::ICoreRender *pCoreRender, const vec2& mousePos)
{
	mat4 VP;
	float aspect = (float)screen.width() / screen.height();
	pCamera->GetViewProjectionMatrix(&VP, aspect);

    mat4 camModelMatrix;
    pCamera->GetModelMatrix(&camModelMatrix);
    vec3 cameraPosition = camModelMatrix.Column3(3);


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

		auto draw_axis = [&](AXIS type, const vec4& color) -> void
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

			shader->SetVec4Parameter("main_color", &color);

			shader->FlushParameters();

			pCoreRender->Draw(_pAxesMesh);
			pCoreRender->Draw(_pAxesArrowMesh);
		};

		static const vec4 colorWhite = vec4(1,1,1,1);
		static const vec4 colorRed = vec4(1,0,0,1);
		static const vec4 colorGreen = vec4(0,1,0,1);
		static const vec4 colorBlue = vec4(0,0,1,1);

        //draw_axis(AXIS::X, colorRed);
        //draw_axis(AXIS::Y, colorGreen);
        //draw_axis(AXIS::Z, colorBlue);

        // debug
        {

            auto draw_vector_debug = [&](const vec3& v, const vec4& color) -> void
            {
                vec3 vN = v.Normalized();
                mat4 basis(0.0f);
                basis.SetColumn3(0, vN);

                qDebug() << "draw_vector_debug N:" << vec3ToString(vN);

                mat4 MVP = VP * selectionWorldTransform * basis * distanceScaleMat;
                shader->SetMat4Parameter("MVP", &MVP);

                shader->SetVec4Parameter("main_color", &color);

                shader->FlushParameters();

                pCoreRender->Draw(_pAxesMesh);
                pCoreRender->Draw(_pAxesArrowMesh);
            };

            float camFov;
            pCamera->GetFovAngle(&camFov);

            mat4 axisTransform = editor->GetSelectionTransform();

            vec3 axis = axisTransform.Column3(0).Normalized();
            vec3 V1 = (axisTransform.Column3(3) - cameraPosition).Normalized();
            vec3 tmp = V1.Cross(axis).Normalized();
            vec3 N = axis.Cross(tmp).Normalized();

            draw_vector_debug(tmp, colorRed);
            draw_vector_debug(N, colorGreen);
            draw_vector_debug(V1, colorGreen);

            vec3 origin = axisTransform.Column3(3);
            Plane plane(N, origin);



            //Line3D r = Line3D((axisTransform.Column3(3) - cameraPosition + vec3(1.0f,0,0)), cameraPosition);
            Line3D r = MouseToRay(camModelMatrix, camFov, aspect, mousePos);

            vec3 i;
            if (LineIntersectPlane(i, plane, r))
            {
                qDebug() <<"intersection x:" << vec3ToString(i);
            }


            //_drawPlane(plane, pCamera, screen, pRender, pCoreRender);

        }

		shader->Release();
	}
}


