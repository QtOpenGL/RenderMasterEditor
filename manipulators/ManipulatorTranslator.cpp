#include "ManipulatorTranslator.h"
#include "editorglobal.h"
#include "EngineGlobal.h"
#include "common.h"

using namespace RENDER_MASTER;

extern EditorGlobal* editor;
extern EngineGlobal* eng;

const float SelectionThresholdInPixels = 8.0f;
const float MaxDistanceInPixels = 1000000.0f;
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

void ManipulatorTranslator::intersectMouseWithAxis(ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos, const vec3 &axisWorldSpace, AXIS_EL type, vec3 &worldOut, float &distance)
{
	uint w = screen.width();
	uint h = screen.height();

	mat4 camViewProj;
	float aspect = (float)w / h;
	pCamera->GetViewProjectionMatrix(&camViewProj, aspect);

	mat4 camModelMatrix;
	pCamera->GetModelMatrix(&camModelMatrix);
	vec3 cameraPosition = camModelMatrix.Column3(3);

	float camFov;
	pCamera->GetFovAngle(&camFov);

	mat4 selectionWorldTransform = editor->GetSelectionTransform();
	vec3 center = selectionWorldTransform.Column3(3);
	vec3 V = (center - cameraPosition).Normalized();
	vec3 VcrossAxis = V.Cross(axisWorldSpace).Normalized();
	vec3 N = axisWorldSpace.Cross(VcrossAxis).Normalized();


	Plane plane(N, center);

	Line3D ray = MouseToRay(camModelMatrix, camFov, aspect, normalizedMousePos);

	if (LineIntersectPlane(worldOut, plane, ray))
	{
	   vec2 A = NdcToScreen(WorldToNdc(center, camViewProj), w, h);

	   float distToCenter = DistanceTo(camViewProj, selectionWorldTransform);
	   vec4 axisEndpointLocal = vec4(AxesEndpoints[(int)type] * axisWorldSize(h, distToCenter));
	   vec4 axisEndpointWorld = selectionWorldTransform * axisEndpointLocal;
	   vec2 B = NdcToScreen(WorldToNdc((vec3&)axisEndpointWorld, camViewProj), w, h);

	   vec2 I = NdcToScreen(WorldToNdc(worldOut, camViewProj), w, h);

	   distance = PointToSegmentDistance(A, B, I);
	   return;
	}

	distance = MaxDistanceInPixels;
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
		vec3 worldDirection = selectionWorldTransform.Column3((int)underMouse).Normalized();
		vec3 center = selectionWorldTransform.Column3(3);
		movesAlongLine = Line3D(worldDirection, center);
	}
	else if (AXIS_EL::XY <= underMouse && underMouse <= AXIS_EL::ZX)
	{
		isMoving = 2;
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
	mat4 invSelectionWorldTransform = selectionWorldTransform.Inverse();
	vec3 center = selectionWorldTransform.Column3(3);

	float aspect = (float)screen.width() / screen.height();


	if (isMoving == 1) // we move manipulator arrow by mouse
	{
		if (!oldNormalizedMousePos.Aproximately(normalizedMousePos))
		{
			oldNormalizedMousePos = normalizedMousePos;

			vec3 intersectionWorld;
			float intersectionDistance;

			intersectMouseWithAxis(pCamera, screen, normalizedMousePos, movesAlongLine.direction, underMouse, intersectionWorld, intersectionDistance);

			if (intersectionDistance < MaxDistanceInPixels)
			{
				vec3 pos = movesAlongLine.projectPoint(intersectionWorld) - worldDelta;
				editor->GetSelectionObject()->SetPosition(&pos);
			}
		}

	} else if (isMoving == 2) // we move manipulator plane by mouse
	{

		float camFov;
		pCamera->GetFovAngle(&camFov);

		mat4 camModelMatrix;
		pCamera->GetModelMatrix(&camModelMatrix);

		Line3D ray = MouseToRay(camModelMatrix, camFov, aspect, normalizedMousePos);

		vec3 worldIntersection;
		if (LineIntersectPlane(worldIntersection, movesAlongPlane, ray))
		{
			vec3 pos = worldIntersection - worldDelta;
			editor->GetSelectionObject()->SetPosition(&pos);
		}

	} else
	{
		underMouse = AXIS_EL::NONE;


		// Check if mouse intersects axes

		vec3 axesWorldDirection[3] = { selectionWorldTransform.Column3(0).Normalized(),
										selectionWorldTransform.Column3(1).Normalized(),
										selectionWorldTransform.Column3(2).Normalized() };
		float minDist = MaxDistanceInPixels;

		for (int i = 0; i < 3; i++)
		{
			vec3 intersectionWorld;
			float intersectionDistance;

			intersectMouseWithAxis(pCamera, screen, normalizedMousePos, axesWorldDirection[i], (AXIS_EL)i, intersectionWorld, intersectionDistance);

			if (intersectionDistance < SelectionThresholdInPixels && intersectionDistance < minDist)
			{
				minDist = intersectionDistance;

				underMouse = static_cast<AXIS_EL>(i);

				Line3D axisLine = Line3D(axesWorldDirection[i], center);
				vec3 projectedToAxisLinePoint = axisLine.projectPoint(intersectionWorld);
				worldDelta = projectedToAxisLinePoint - center;
			}
		}


		// Check if mouse intersects axis plane

		float camFov;
		pCamera->GetFovAngle(&camFov);

		mat4 camModelMatrix;
		pCamera->GetModelMatrix(&camModelMatrix);

		mat4 camViewProj;
		pCamera->GetViewProjectionMatrix(&camViewProj, aspect);

		float distToCenter = DistanceTo(camViewProj, selectionWorldTransform);
		mat4 distanceScaleMat = mat4(axisWorldSize(screen.height(), distToCenter));
		mat4 tmpMat = camViewProj * selectionWorldTransform * distanceScaleMat;

		mat4 axisToWorld[3];
		axisToWorld[0] = tmpMat * xyMirroringMat * correctionXYMat;
		axisToWorld[1] = tmpMat * yzMirroringMat * correctionYZMat;
		axisToWorld[2] = tmpMat * zxMirroringMat * correctionZXMat;

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

		if (AXIS_EL::XY <= underMouse && underMouse <= AXIS_EL::ZX)
		{
			vec3 N = axesWorldDirection[0].Cross(axesWorldDirection[1]);
			if (underMouse == AXIS_EL::YZ)
				N = axesWorldDirection[1].Cross(axesWorldDirection[2]);
			else if (underMouse == AXIS_EL::ZX)
				N = axesWorldDirection[0].Cross(axesWorldDirection[2]);

			movesAlongPlane = Plane(N, center);


			Line3D ray = MouseToRay(camModelMatrix, camFov, aspect, normalizedMousePos);

			vec3 worldIntersection;
			if (LineIntersectPlane(worldIntersection, movesAlongPlane, ray))
			{
				worldDelta = worldIntersection - center;

				//qDebug() << vec3ToString(worldDelta);
			}
		}
	}


	// Project camera position to "axis space"
	if (isMoving != 2)
	{
		vec3 cameraPos;
		pCamera->GetPosition(&cameraPos);
		vec4 camPos_axesSpace = invSelectionWorldTransform * vec4(cameraPos);

		xyMirroringMat.el_2D[0][0] = camPos_axesSpace.x > 0 ? 1.0f : -1.0f;
		xyMirroringMat.el_2D[1][1] = camPos_axesSpace.y > 0 ? 1.0f : -1.0f;
		yzMirroringMat.el_2D[1][1] = camPos_axesSpace.y > 0 ? 1.0f : -1.0f;
		yzMirroringMat.el_2D[2][2] = camPos_axesSpace.z > 0 ? 1.0f : -1.0f;
		zxMirroringMat.el_2D[0][0] = camPos_axesSpace.x > 0 ? 1.0f : -1.0f;
		zxMirroringMat.el_2D[2][2] = camPos_axesSpace.z > 0 ? 1.0f : -1.0f;
	}
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

	auto draw_axis_quad = [&](const vec4& color, const mat4& correctionMat) -> void
	{
		mat4 MVP = scaledTransform * correctionMat;

		shader->SetMat4Parameter("MVP", &MVP);
		shader->SetVec4Parameter("main_color", &color);
		shader->FlushParameters();

		pCoreRender->Draw(_pQuadLines);
	};

	draw_axis_quad(underMouse == AXIS_EL::XY ? ColorSelection : ColorMagneta, xyMirroringMat * correctionXYMat);
	draw_axis_quad(underMouse == AXIS_EL::YZ ? ColorSelection : ColorMagneta, yzMirroringMat * correctionYZMat);
	draw_axis_quad(underMouse == AXIS_EL::ZX ? ColorSelection : ColorMagneta, zxMirroringMat * correctionZXMat);

	draw_axis(underMouse == AXIS_EL::X ? ColorSelection : ColorRed,		correctionXMat);
	draw_axis(underMouse == AXIS_EL::Y ? ColorSelection : ColorGreen,	correctionYMat);
	draw_axis(underMouse == AXIS_EL::Z ? ColorSelection : ColorBlue,	correctionZMat);


	// draw transparent selection plane
	if (underMouse == AXIS_EL::XY || underMouse == AXIS_EL::YZ || underMouse == AXIS_EL::ZX)
	{
		mat4 mirr = xyMirroringMat * correctionXYMat;
		if (underMouse == AXIS_EL::YZ)
			mirr = yzMirroringMat * correctionYZMat;
		else if (underMouse == AXIS_EL::ZX)
			mirr = zxMirroringMat * correctionZXMat;

		pCoreRender->SetBlendState(BLEND_FACTOR::SRC_ALPHA, BLEND_FACTOR::ONE_MINUS_SRC_ALPHA);

		mat4 shrinkPlane = mat4(0.5f);
		shrinkPlane.el_2D[0][3] = 0.5f;
		shrinkPlane.el_2D[1][3] = 0.5f;

		mat4 MVP = scaledTransform * mirr * shrinkPlane;
		shader->SetMat4Parameter("MVP", &MVP);

		shader->SetVec4Parameter("main_color", &vec4(1, 1, 0, 0.21f));

		shader->FlushParameters();

		pCoreRender->Draw(_pPlane);
	}

	shader->Release();
}


