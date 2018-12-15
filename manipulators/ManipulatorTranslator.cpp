#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"
#include "common.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;

const float SelectDistance = 8.0f;

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
	return (90.0f / h) * dist;
}

void ManipulatorTranslator::render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *pRender, RENDER_MASTER::ICoreRender *pCoreRender, const vec2& mousePos)
{
	// screen
    uint w = screen.width();
    uint h = screen.height();


	// camera
    mat4 camVP;
    float aspect = (float)w / h;
    pCamera->GetViewProjectionMatrix(&camVP, aspect);

    mat4 camModelMatrix;
    pCamera->GetModelMatrix(&camModelMatrix);
    vec3 cameraPosition = camModelMatrix.Column3(3);

    mat4 camProjMat;
    pCamera->GetProjectionMatrix(&camProjMat, aspect);

	float camFov;
	pCamera->GetFovAngle(&camFov);


	// selection
	mat4 selectionWorldTransform = editor->GetSelectionTransform();

	vec4 view4 = camVP * vec4(selectionWorldTransform.el_2D[0][3], selectionWorldTransform.el_2D[1][3], selectionWorldTransform.el_2D[2][3], 1.0f);
	vec3 view(view4);
	float dist = view.Lenght();

	mat4 axisTransform = editor->GetSelectionTransform();
	vec3 axisOrigin = axisTransform.Column3(3);

	vec3 V = (axisOrigin - cameraPosition).Normalized();

	int mouseNearX = 0;
	int mouseNearY = 0;
	int mouseNearZ = 0;

	float distanceToX = 1000000.0;
	float distanceToY = 1000000.0;
	float distanceToZ = 1000000.0;

	//vec3 axisX = axisTransform.Column3(0).Normalized();

	auto distance_to_axis = [&](const vec3& axis, const vec3& endPoint) -> float
	{
		vec3 tmp = V.Cross(axis).Normalized();
		vec3 planeN = axis.Cross(tmp).Normalized();

		vec3 origin = axisTransform.Column3(3);
		Plane plane(planeN, origin);

		Line3D r = MouseToRay(camModelMatrix, camFov, aspect, mousePos);

		vec3 i;
		if (LineIntersectPlane(i, plane, r))
		{
		   //qDebug() <<"intersection x:" << vec3ToString(i);

		   vec2 iScreen = NdcToScreen(WorldToNdc(i, camVP), w, h);
		   //qDebug() <<"Intersection in screen:" << vec2ToString(iScreen);

		   vec2 p0Screen = NdcToScreen(WorldToNdc(axisOrigin, camVP), w, h);
		   vec4 axisEndpointLocal = vec4(endPoint * axisSize(h, dist));
		   vec4 axisEndpointWorld = axisTransform * axisEndpointLocal;
		   vec2 p1Ndc = WorldToNdc(axisEndpointWorld.Vec3(), camVP);
		   vec2 p1Screen = NdcToScreen(p1Ndc, w, h);

		   float dist = SegmentPointDistance(p0Screen, p1Screen, iScreen);

		   qDebug() << "mouse ndc:" << vec2ToString(mousePos * 2.0f - vec2(1,1)) << " p1Ndc:" << vec2ToString(p1Ndc) << " dist:" << dist;

		   return dist;
		}

		return 1000000.0f;
	};

	float minDisatnce = 100000.0f;
	int axesIdx = -1;
	vec3 axes[3] = { axisTransform.Column3(0).Normalized(), axisTransform.Column3(1).Normalized(), axisTransform.Column3(2).Normalized() };
	vec3 axesEndpoints[3] = { vec3(1,0,0), vec3(0,1,0), vec3(0,0,1) };

	for (int i = 0; i < 1; i++)
	{
		float dist = distance_to_axis(axes[i], axesEndpoints[i]);
		if (dist < SelectDistance && dist < minDisatnce)
		{
			minDisatnce = dist;
			axesIdx = i;
		}
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

		static const vec4 colorSelection = vec4(1,0,1,1);
		static const vec4 colorRed = vec4(1,0,0,1);
		static const vec4 colorGreen = vec4(0,1,0,1);
		static const vec4 colorBlue = vec4(0,0,1,1);

		draw_axis(AXIS::X, axesIdx == 0 ? colorSelection : colorRed);
		draw_axis(AXIS::Y, axesIdx == 1 ? colorSelection : colorGreen);
		draw_axis(AXIS::Z, axesIdx == 2 ? colorSelection : colorBlue);

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



            //_drawPlane(plane, pCamera, screen, pRender, pCoreRender);

        }

		shader->Release();
	}
}


