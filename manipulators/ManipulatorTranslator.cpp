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
const vec4 ColorMagneta = vec4(0,1,1,1);

mat4 correctionXMat = mat4(1.0f);
mat4 correctionYMat = mat4(1.0f);
mat4 correctionZMat = mat4(1.0f);

constexpr float axesPlaneScale = 0.33f;
mat4 correctionXYMat = mat4(axesPlaneScale);
mat4 correctionYZMat = mat4(axesPlaneScale);
mat4 correctionZXMat = mat4(axesPlaneScale);

vec4 triangle_[2][3];


float axisWorldSize(uint h, float dist)
{
	return (90.0f / h) * dist;
}

void ManipulatorTranslator::intersectMouseWithAxisPlane(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos, const vec3 &axisWorldSpace, AXIS_EL type, vec3 &worldOut, float &distance)
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
	float distToCenter = DistanceTo(camViewProj, selectionWorldTransform);
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
	   vec2 Bndc = WorldToNdc((vec3&)axisEndpointWorld, camViewProj);
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

	correctionZXMat.el_2D[2][2] = 0.0f;
	correctionZXMat.el_2D[1][1] = 0.0f;
	correctionZXMat.el_2D[1][2] = axesPlaneScale;
	correctionZXMat.el_2D[2][1] = axesPlaneScale;

	correctionYZMat.el_2D[0][0] = 0.0f;
	correctionYZMat.el_2D[2][2] = 0.0f;
	correctionYZMat.el_2D[2][0] = axesPlaneScale;
	correctionYZMat.el_2D[0][2] = axesPlaneScale;

	triangle_[0][0] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	triangle_[0][1] = vec4(1.0f, 0.0f, 0.0f, 1.0f);
	triangle_[0][2] = vec4(1.0f, 1.0f, 0.0f, 1.0f);

	triangle_[1][0] = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	triangle_[1][1] = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	triangle_[1][2] = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}

ManipulatorTranslator::~ManipulatorTranslator()
{
}

bool ManipulatorTranslator::isMouseIntersects(const vec2& normalizedMousePos)
{
	return underMouse != AXIS_EL::NONE;
}

void ManipulatorTranslator::mouseButtonDown(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (AXIS_EL::NONE < underMouse && underMouse <= AXIS_EL::Z)
	{
		isMoving = 1;
		mat4 selectionWorldTransform = editor->GetSelectionTransform();
		vec3 dirWorld = selectionWorldTransform.Column3((int)underMouse).Normalized();
		vec3 center = selectionWorldTransform.Column3(3);
		lineAlongMoving = Line3D(dirWorld, center);
	}
}

void ManipulatorTranslator::mouseButtonUp()
{
	isMoving = 0;
}

float sig(const vec2& p1, const vec2& p2, const vec2& p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle(const vec2& pt, const vec2& v1, const vec2& v2, const vec2& v3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sig(pt, v1, v2);
	d2 = sig(pt, v2, v3);
	d3 = sig(pt, v3, v1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

void ManipulatorTranslator::update(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos)
{
	if (!editor->IsSomeObjectSelected())
	{
		isMoving = 0;
		return;
	}

	mat4 selectionWorldTransform = editor->GetSelectionTransform();

	// we move manipulator by mouse
	if (isMoving)
	{
		if (!lastNormalizedMousePos.Aproximately(normalizedMousePos))
		{
			lastNormalizedMousePos = normalizedMousePos;
			vec3 axesDirWorld = lineAlongMoving.direction;
			vec3 intersectionWorld;
			float intersectionDistance;

			intersectMouseWithAxisPlane(pCamera, screen, normalizedMousePos, axesDirWorld, underMouse, intersectionWorld, intersectionDistance);

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
		underMouse = AXIS_EL::NONE;
		vec3 center = selectionWorldTransform.Column3(3);

		// check if mouse select axes

		vec3 axes[3] = { selectionWorldTransform.Column3(0).Normalized(), selectionWorldTransform.Column3(1).Normalized(), selectionWorldTransform.Column3(2).Normalized() };
		float minDist = MaxDistance;

		for (int i = 0; i < 3; i++)
		{
			vec3 intersectionWorld;
			vec3 intersectionDeltaToCenter;
			float intersectionDistance;

			intersectMouseWithAxisPlane(pCamera, screen, normalizedMousePos, axes[i], (AXIS_EL)i, intersectionWorld, intersectionDistance);

			if (intersectionDistance < SelectDistance && intersectionDistance < minDist)
			{
				minDist = intersectionDistance;
				underMouse = (AXIS_EL)i;
				Line3D axisLine = Line3D(axes[i], center);

				vec3 projectedToAxisLinePoint = axisLine.projectPoint(intersectionWorld);

				delta = projectedToAxisLinePoint - center;
			}
		}

		// check if mouse select axis plane

		mat4 camViewProj;
		float aspect = (float)screen.width() / screen.height();
		pCamera->GetViewProjectionMatrix(&camViewProj, aspect);

		float distToCenter = DistanceTo(camViewProj, selectionWorldTransform);
		mat4 distanceScaleMat = mat4(axisWorldSize(screen.height(), distToCenter));
		mat4 tmpMat = camViewProj * selectionWorldTransform * distanceScaleMat;

		mat4 axisToWorld[3];
		axisToWorld[0] = tmpMat * xyPlaneMat * correctionXYMat;
		axisToWorld[1] = tmpMat * yzPlaneMat * correctionYZMat;
		axisToWorld[2] = tmpMat * zxPlaneMat * correctionZXMat;

		vec2 ndcMouse = normalizedMousePos * 2.0f - vec2(1.0f, 1.0f);

		for (int k = 0; k < 3; k++) // each plane
		{
			for (int j = 0; j < 2; j++) // 2 triangle
			{
				vec4 ndc[3];
				for (int i = 0; i < 3; i++) // 3 point
				{
					ndc[i] = axisToWorld[k] * triangle_[j][i];
					ndc[i] /= ndc[i].w;
				}

				if (PointInTriangle(ndcMouse, ndc[0], ndc[1], ndc[2]))
				{
					underMouse = static_cast<AXIS_EL>(int(AXIS_EL::XY) + k);
				}
			}
		}

		//qDebug() << vec2ToString(ndcMouse)<< vec2ToString(ndc[0])<< vec2ToString(ndc[1])<< vec2ToString(ndc[2]);
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
    uint w = screen.width();
    uint h = screen.height();

	mat4 camViewProj;
	float aspect = (float)w / h;
	pCamera->GetViewProjectionMatrix(&camViewProj, aspect);

	ShaderRequirement req;
	req.attributes = INPUT_ATTRUBUTE::POSITION;

	IShader *shader = nullptr;
	pRender->PreprocessStandardShader(&shader, &req);
	if (!shader)
		return;

	shader->AddRef();

	pCoreRender->SetShader(shader);
	pCoreRender->SetDepthTest(0);

	mat4 selectionWorldTransform = editor->GetSelectionTransform();
	float distToCenter = DistanceTo(camViewProj, selectionWorldTransform);
	mat4 distanceScaleMat = mat4(axisWorldSize(h, distToCenter));
	mat4 scaledTransform = camViewProj * selectionWorldTransform * distanceScaleMat;

	auto draw_axis = [&](const vec4& color, const mat4& correctionMat) -> void
	{
		mat4 MVP = scaledTransform * correctionMat;

		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &color);
		shader->FlushParameters();

		pCoreRender->Draw(_pAxesMesh);
		pCoreRender->Draw(_pAxesArrowMesh);
	};

	auto draw_axis_plane = [&](const vec4& color, const mat4& correctionMat) -> void
	{
		mat4 MVP = scaledTransform * correctionMat;

		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &color);
		shader->FlushParameters();

		pCoreRender->Draw(_pQuadLines);
	};


	draw_axis_plane(underMouse == AXIS_EL::XY ? ColorSelection : ColorMagneta, xyPlaneMat * correctionXYMat);
	draw_axis_plane(underMouse == AXIS_EL::YZ ? ColorSelection : ColorMagneta, yzPlaneMat * correctionYZMat);
	draw_axis_plane(underMouse == AXIS_EL::ZX ? ColorSelection : ColorMagneta, zxPlaneMat * correctionZXMat);

	draw_axis(underMouse == AXIS_EL::X ? ColorSelection : ColorRed,		correctionXMat);
	draw_axis(underMouse == AXIS_EL::Y ? ColorSelection : ColorGreen,	correctionYMat);
	draw_axis(underMouse == AXIS_EL::Z ? ColorSelection : ColorBlue,	correctionZMat);

	shader->Release();
}


