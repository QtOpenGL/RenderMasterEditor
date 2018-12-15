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

float axisSize(uint h, float dist)
{
    return (80.0f / h) * dist;
}

void ManipulatorTranslator::render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *pRender, RENDER_MASTER::ICoreRender *pCoreRender, const vec2& mousePos)
{
    uint w = screen.width();
    uint h = screen.height();

    mat4 camVP;
    float aspect = (float)w / h;
    pCamera->GetViewProjectionMatrix(&camVP, aspect);

    mat4 camModelMatrix;
    pCamera->GetModelMatrix(&camModelMatrix);
    vec3 cameraPosition = camModelMatrix.Column3(3);

    mat4 camProjMat;
    pCamera->GetProjectionMatrix(&camProjMat, aspect);

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

        vec4 view4 = camVP * vec4(selectionWorldTransform.el_2D[0][3], selectionWorldTransform.el_2D[1][3], selectionWorldTransform.el_2D[2][3], 1.0f);
		vec3 view(view4);
		float dist = view.Lenght();

		mat4 distanceScaleMat;
        distanceScaleMat.el_2D[0][0] = axisSize(h, dist);
        distanceScaleMat.el_2D[1][1] = axisSize(h, dist);
        distanceScaleMat.el_2D[2][2] = axisSize(h, dist);

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
            mat4 MVP = camVP * selectionWorldTransform * distanceScaleMat * correctionMat;
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

        draw_axis(AXIS::X, colorRed);
        draw_axis(AXIS::Y, colorGreen);
        draw_axis(AXIS::Z, colorBlue);

        // debug
        {

            auto draw_vector_debug = [&](const vec3& v, const vec4& color) -> void
            {
                vec3 vN = v.Normalized();
                mat4 basis(0.0f);
                basis.SetColumn3(0, vN);

                //qDebug() << "draw_vector_debug N:" << vec3ToString(vN);

                mat4 MVP = camVP * selectionWorldTransform * basis * distanceScaleMat;
                shader->SetMat4Parameter("MVP", &MVP);

                shader->SetVec4Parameter("main_color", &color);

                shader->FlushParameters();

                pCoreRender->Draw(_pAxesMesh);
                pCoreRender->Draw(_pAxesArrowMesh);
            };

            float camFov;
            pCamera->GetFovAngle(&camFov);

            mat4 axisTransform = editor->GetSelectionTransform();
            vec3 axisOrigin = axisTransform.Column3(3);

            vec3 axis = axisTransform.Column3(0).Normalized();
            vec3 V1 = (axisOrigin - cameraPosition).Normalized();
            vec3 tmp = V1.Cross(axis).Normalized();
            vec3 N = axis.Cross(tmp).Normalized();

            //draw_vector_debug(tmp, colorRed);
            //draw_vector_debug(N, colorGreen);
           // draw_vector_debug(V1, colorGreen);

            vec3 origin = axisTransform.Column3(3);
            Plane plane(N, origin);

            Line3D r = MouseToRay(camModelMatrix, camFov, aspect, mousePos);

            vec3 i;
            if (LineIntersectPlane(i, plane, r))
            {
               //qDebug() <<"intersection x:" << vec3ToString(i);

               vec2 iScreen = NdcToScreen(WorldToNdc(i, camVP), w, h);
               qDebug() <<"Intersection in screen:" << vec2ToString(iScreen);

               vec2 p0Ndc = NdcToScreen(WorldToNdc(axisOrigin, camVP), w, h);
               vec2 p1Ndc = NdcToScreen(WorldToNdc((axisOrigin + vec3(1,0,0) * axisSize(h, dist)), camVP), w, h);

               //float distToX = SegmentPointDistance(p0Ndc, p1Ndc, iNdc);

               //qDebug() << "distance:" <<
            }


            //_drawPlane(plane, pCamera, screen, pRender, pCoreRender);

        }

		shader->Release();
	}
}


