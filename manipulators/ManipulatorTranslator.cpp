#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"
#include "common.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;

const float SelectDistance = 8.0f;
const float MaxDistance = 1000000.0f;
const vec3 AxesEndpointDelta[3] = {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)};
const vec4 ColorSelection = vec4(1,1,0,1);
const vec4 ColorRed = vec4(1,0,0,1);
const vec4 ColorGreen = vec4(0,1,0,1);
const vec4 ColorBlue = vec4(0,0,1,1);

ManipulatorTranslator::~ManipulatorTranslator()
{
}

bool ManipulatorTranslator::isMouseIntersects(const vec2& normalizedMousePos)
{
	return moiseHoverAxis != AXIS::NONE;
}

float axisWorldSize(uint h, float dist)
{
	return (85.0f / h) * dist;
}

void ManipulatorTranslator::update(ICamera *pCamera, const QRect &screen, IRender *render, ICoreRender *coreRender, const vec2 &normalizedMousePos)
{
	// screen
	uint w = screen.width();
	uint h = screen.height();


	// camera
	mat4 camViewProj;
	float aspect = (float)w / h;
	pCamera->GetViewProjectionMatrix(&camViewProj, aspect);

	mat4 camModelMatrix;
	pCamera->GetModelMatrix(&camModelMatrix);
	vec3 cameraPosition = camModelMatrix.Column3(3);

	mat4 camProjMat;
	pCamera->GetProjectionMatrix(&camProjMat, aspect);

	float camFov;
	pCamera->GetFovAngle(&camFov);


	// selection
	mat4 selectionWorldTransform = editor->GetSelectionTransform();

	vec4 view4 = camViewProj * vec4(selectionWorldTransform.el_2D[0][3], selectionWorldTransform.el_2D[1][3], selectionWorldTransform.el_2D[2][3], 1.0f);
	vec3 view(view4);
	float distToSelction = view.Lenght();

	mat4 axisTransform = editor->GetSelectionTransform();
	vec3 axisOrigin = axisTransform.Column3(3);

	vec3 V = (axisOrigin - cameraPosition).Normalized();


	auto distance_to_axis = [&](const vec3& axis, AXIS type) -> float
	{
		vec3 VcrossAxis = V.Cross(axis).Normalized();
		vec3 N = axis.Cross(VcrossAxis).Normalized();
		vec3 O = axisTransform.Column3(3);
		Plane plane(N, O);

		Line3D R = MouseToRay(camModelMatrix, camFov, aspect, normalizedMousePos);

		vec3 I;
		if (LineIntersectPlane(I, plane, R))
		{
		   //qDebug() <<"intersection x:" << vec3ToString(i);

		   vec2 i = NdcToScreen(WorldToNdc(I, camViewProj), w, h);
		   vec2 A = NdcToScreen(WorldToNdc(axisOrigin, camViewProj), w, h);
		   vec4 axisEndpointLocal = vec4(AxesEndpointDelta[(int)type] * axisWorldSize(h, distToSelction));
		   vec4 axisEndpointWorld = axisTransform * axisEndpointLocal;
		   vec2 Bndc = WorldToNdc(axisEndpointWorld.Vec3(), camViewProj);
		   vec2 B = NdcToScreen(Bndc, w, h);

		   float dist = PointToSegmentDistance(A, B, i);

		   //qDebug() << "mouse ndc:" << vec2ToString(normalizedMousePos * 2.0f - vec2(1,1)) << " p1Ndc:" << vec2ToString(p1Ndc) << " dist:" << dist;

		   return dist;
		}

		return MaxDistance;
	};


	vec3 axes[3] = { axisTransform.Column3(0).Normalized(), axisTransform.Column3(1).Normalized(), axisTransform.Column3(2).Normalized() };
	moiseHoverAxis = AXIS::NONE;
	float minDist = MaxDistance;

	for (int i = 0; i < 3; i++)
	{
		float dist = distance_to_axis(axes[i], (AXIS)i);

		if (dist < SelectDistance && dist < minDist)
		{
			minDist = dist;
			moiseHoverAxis = (AXIS)i;
		}
	}
}

void ManipulatorTranslator::render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *pRender, RENDER_MASTER::ICoreRender *pCoreRender)
{
	// screen
    uint w = screen.width();
    uint h = screen.height();

	// camera
	mat4 camViewProj;
	float aspect = (float)w / h;
	pCamera->GetViewProjectionMatrix(&camViewProj, aspect);

	// selection
	mat4 selectionWorldTransform = editor->GetSelectionTransform();

	vec4 view4 = camViewProj * vec4(selectionWorldTransform.el_2D[0][3], selectionWorldTransform.el_2D[1][3], selectionWorldTransform.el_2D[2][3], 1.0f);
	vec3 view(view4);
	float dist = view.Lenght();

	{
		ShaderRequirement req;
		req.attributes = INPUT_ATTRUBUTE::POSITION;

		IShader *shader = nullptr;
		pRender->PreprocessStandardShader(&shader, &req);
		if (!shader)
			return;

		shader->AddRef();

		pCoreRender->SetShader(shader);
		pCoreRender->SetDepthTest(0);

		mat4 distanceScaleMat;
		distanceScaleMat.el_2D[0][0] = axisWorldSize(h, dist);
		distanceScaleMat.el_2D[1][1] = axisWorldSize(h, dist);
		distanceScaleMat.el_2D[2][2] = axisWorldSize(h, dist);

		auto draw_axis = [&](AXIS type, const vec4& color) -> void
		{
			mat4 rotationMat(1.0f);

			if (type == AXIS::Y)
			{
				rotationMat.el_2D[0][0] = 0.0f;
				rotationMat.el_2D[1][1] = 0.0f;
				rotationMat.el_2D[1][0] = 1.0f;
				rotationMat.el_2D[0][1] = 1.0f;
			} else if (type == AXIS::Z)
			{
				rotationMat.el_2D[0][0] = 0.0f;
				rotationMat.el_2D[2][2] = 0.0f;
				rotationMat.el_2D[2][0] = 1.0f;
				rotationMat.el_2D[0][2] = 1.0f;
			}
			mat4 MVP = camViewProj * selectionWorldTransform * distanceScaleMat * rotationMat;
			shader->SetMat4Parameter("MVP", &MVP);
			shader->SetVec4Parameter("main_color", &color);
			shader->FlushParameters();

			pCoreRender->Draw(_pAxesMesh);
			pCoreRender->Draw(_pAxesArrowMesh);
		};

		draw_axis(AXIS::X, (int)moiseHoverAxis == 0 ? ColorSelection : ColorRed);
		draw_axis(AXIS::Y, (int)moiseHoverAxis == 1 ? ColorSelection : ColorGreen);
		draw_axis(AXIS::Z, (int)moiseHoverAxis == 2 ? ColorSelection : ColorBlue);

		shader->Release();
	}
}


