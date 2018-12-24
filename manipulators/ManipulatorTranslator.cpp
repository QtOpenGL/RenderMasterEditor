#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"
#include "common.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;

const float SelectDistance = 8.0f;
const float MaxDistance = 1000000.0f;
const vec3 AxesEndpoints[3] = {vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)};
const vec4 ColorSelection = vec4(1,1,0,1);
const vec4 ColorRed = vec4(1,0,0,1);
const vec4 ColorGreen = vec4(0,1,0,1);
const vec4 ColorBlue = vec4(0,0,1,1);
mat4 correctionXMat = mat4(1.0f);
mat4 correctionYMat = mat4(1.0f);
mat4 correctionZMat = mat4(1.0f);

float axisWorldSize(uint h, float dist)
{
	return (90.0f / h) * dist;
}

void ManipulatorTranslator::intersectMouseWithAxisPlane(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos, const vec3 &axisWorldSpace, AXIS type, vec3 &worldOut, float &distance)
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
	float distToCenter = WorldDistance(camViewProj, selectionWorldTransform);
	vec3 center = selectionWorldTransform.Column3(3);
	vec3 V = (center - cameraPosition).Normalized();

	vec3 VcrossAxis = V.Cross(axisWorldSpace).Normalized();
	vec3 N = axisWorldSpace.Cross(VcrossAxis).Normalized();
	Plane plane(N, center);

	Line3D R = MouseToRay(camModelMatrix, camFov, aspect, normalizedMousePos);

	vec3 intersectionWorld;
	if (LineIntersectPlane(intersectionWorld, plane, R))
	{
	   //qDebug() <<"intersection x:" << vec3ToString(i);

	   vec2 I = NdcToScreen(WorldToNdc(intersectionWorld, camViewProj), w, h);
	   vec2 A = NdcToScreen(WorldToNdc(center, camViewProj), w, h);
	   vec4 axisEndpointLocal = vec4(AxesEndpoints[(int)type] * axisWorldSize(h, distToCenter));
	   vec4 axisEndpointWorld = selectionWorldTransform * axisEndpointLocal;
	   vec2 Bndc = WorldToNdc(axisEndpointWorld.Vec3(), camViewProj);
	   vec2 B = NdcToScreen(Bndc, w, h);

	   distance = PointToSegmentDistance(A, B, I);
	   worldOut = intersectionWorld;
	   return;
	   //qDebug() << "mouse ndc:" << vec2ToString(normalizedMousePos * 2.0f - vec2(1,1)) << " p1Ndc:" << vec2ToString(p1Ndc) << " dist:" << dist;
	}

	distance = MaxDistance;
}

ManipulatorTranslator::ManipulatorTranslator(ICore *pCore) : ManipulatorBase(pCore)
{
	correctionYMat.el_2D[0][0] = 0.0f;
	correctionYMat.el_2D[1][1] = 0.0f;
	correctionYMat.el_2D[1][0] = 1.0f;
	correctionYMat.el_2D[0][1] = 1.0f;

	correctionZMat.el_2D[0][0] = 0.0f;
	correctionZMat.el_2D[2][2] = 0.0f;
	correctionZMat.el_2D[2][0] = 1.0f;
	correctionZMat.el_2D[0][2] = 1.0f;
}

ManipulatorTranslator::~ManipulatorTranslator()
{
}

bool ManipulatorTranslator::isMouseIntersects(const vec2& normalizedMousePos)
{
	return mouseHoverAxis != AXIS::NONE;
}

void ManipulatorTranslator::mouseButtonDown(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (mouseHoverAxis != AXIS::NONE)
	{
		isMoving = 1;
		mat4 selectionWorldTransform = editor->GetSelectionTransform();
		vec3 dirWorld = selectionWorldTransform.Column3((int)mouseHoverAxis).Normalized();
		vec3 center = selectionWorldTransform.Column3(3);
		lineAlongMoving = Line3D(dirWorld, center);
	}
}

void ManipulatorTranslator::mouseButtonUp()
{
	isMoving = 0;
}

void ManipulatorTranslator::update(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (!editor->IsSomeObjectSelected())
	{
		isMoving = 0;
		return;
	}

	mat4 selectionWorldTransform = editor->GetSelectionTransform();

	if (isMoving)
	{
		if (!lastNormalizedMousePos.Aproximately(normalizedMousePos))
		{
			lastNormalizedMousePos = normalizedMousePos;

			vec3 axesDirWorld = lineAlongMoving.direction;

			vec3 intersectionWorld;
			float intersectionDistance;
			intersectMouseWithAxisPlane(pCamera, screen, normalizedMousePos, axesDirWorld, mouseHoverAxis, intersectionWorld, intersectionDistance);

			if (intersectionDistance < MaxDistance)
			{
				vec3 projectedToLinePoint = lineAlongMoving.projectPoint(intersectionWorld);

				vec3 pos = projectedToLinePoint - delta;
				IGameObject *g = editor->GetSelectionObject();
				g->SetPosition(&pos);
			}
		}

	} else
	{
		vec3 center = selectionWorldTransform.Column3(3);
		vec3 axes[3] = { selectionWorldTransform.Column3(0).Normalized(), selectionWorldTransform.Column3(1).Normalized(), selectionWorldTransform.Column3(2).Normalized() };
		mouseHoverAxis = AXIS::NONE;
		float minDist = MaxDistance;

		for (int i = 0; i < 3; i++)
		{
			vec3 intersectionWorld;
			vec3 intersectionDeltaToCenter;
			float intersectionDistance;
			intersectMouseWithAxisPlane(pCamera, screen, normalizedMousePos, axes[i], (AXIS)i, intersectionWorld, intersectionDistance);

			if (intersectionDistance < SelectDistance && intersectionDistance < minDist)
			{
				minDist = intersectionDistance;
				mouseHoverAxis = (AXIS)i;

				Line3D axis3d = Line3D(axes[i], center);
				vec3 projectedToLinePoint = axis3d.projectPoint(intersectionWorld);
				delta = projectedToLinePoint - center;
			}
		}
	}


	// project camera position to "axis space"
	vec3 cameraPos;
	pCamera->GetPosition(&cameraPos);
	mat4 invSelectionWorldTransform = selectionWorldTransform.Inverse();
	vec4 camPos_axesSpace = invSelectionWorldTransform * vec4(cameraPos);
	xyPlaneMat.el_2D[0][0] = camPos_axesSpace.x > 0 ? 1.0f : -1.0f;
	xyPlaneMat.el_2D[1][1] = camPos_axesSpace.y > 0 ? 1.0f : -1.0f;
	yzPlaneMat.el_2D[1][1] = camPos_axesSpace.y > 0 ? 1.0f : -1.0f;
	yzPlaneMat.el_2D[2][2] = camPos_axesSpace.z > 0 ? 1.0f : -1.0f;
	zxPlaneMat.el_2D[0][0] = camPos_axesSpace.x > 0 ? 1.0f : -1.0f;
	zxPlaneMat.el_2D[2][2] = camPos_axesSpace.z > 0 ? 1.0f : -1.0f;

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
	float distToCenter = WorldDistance(camViewProj, selectionWorldTransform);

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
		distanceScaleMat.el_2D[0][0] = axisWorldSize(h, distToCenter);
		distanceScaleMat.el_2D[1][1] = axisWorldSize(h, distToCenter);
		distanceScaleMat.el_2D[2][2] = axisWorldSize(h, distToCenter);

		auto draw_axis = [&](AXIS type, const vec4& color, const mat4& correctionMat) -> void
		{
			mat4 MVP = camViewProj * selectionWorldTransform * distanceScaleMat * correctionMat;
			shader->SetMat4Parameter("MVP", &MVP);
			shader->SetVec4Parameter("main_color", &color);
			shader->FlushParameters();

			pCoreRender->Draw(_pAxesMesh);
			pCoreRender->Draw(_pAxesArrowMesh);
		};

		auto draw_axis_plane = [&](AXIS_PLANE type, const vec4& color) -> void
		{
			constexpr float u = 0.33f;
			mat4 correctionMat(u);
			if (type == AXIS_PLANE::ZX)
			{
				correctionMat.el_2D[2][2] = 0.0f;
				correctionMat.el_2D[1][1] = 0.0f;
				correctionMat.el_2D[1][2] = u;
				correctionMat.el_2D[2][1] = u;
				correctionMat = zxPlaneMat * correctionMat;
			} else if (type == AXIS_PLANE::YZ)
			{
				correctionMat.el_2D[0][0] = 0.0f;
				correctionMat.el_2D[2][2] = 0.0f;
				correctionMat.el_2D[2][0] = u;
				correctionMat.el_2D[0][2] = u;
				correctionMat = yzPlaneMat * correctionMat;
			} else
				correctionMat = xyPlaneMat * correctionMat;

			mat4 MVP = camViewProj * selectionWorldTransform * distanceScaleMat * correctionMat;
			shader->SetMat4Parameter("MVP", &MVP);
			shader->SetVec4Parameter("main_color", &color);
			shader->FlushParameters();

			pCoreRender->Draw(_pQuadLines);
		};


		draw_axis_plane(AXIS_PLANE::XY, ColorSelection);
		draw_axis_plane(AXIS_PLANE::YZ, ColorSelection);
		draw_axis_plane(AXIS_PLANE::ZX, ColorSelection);

		draw_axis(AXIS::X, (int)mouseHoverAxis == 0 ? ColorSelection : ColorRed,	correctionXMat);
		draw_axis(AXIS::Y, (int)mouseHoverAxis == 1 ? ColorSelection : ColorGreen,	correctionYMat);
		draw_axis(AXIS::Z, (int)mouseHoverAxis == 2 ? ColorSelection : ColorBlue,	correctionZMat);

		shader->Release();
	}
}


