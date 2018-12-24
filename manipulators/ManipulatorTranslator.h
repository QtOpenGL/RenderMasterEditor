#ifndef MANIPULATORTRANSLATOR_H
#define MANIPULATORTRANSLATOR_H
#include <QObject>
#include "ManipulatorBase.h"


enum class AXIS_EL
{
	NONE = -1,
	X,
	Y,
	Z,
	XY,
	YZ,
	ZX
};

class ManipulatorTranslator : public ManipulatorBase
{
	Q_OBJECT

	AXIS_EL underMouse = AXIS_EL::NONE;
	vec3 delta;
	int isMoving = 0;
	vec3 worldDelta;

	mat4 xyPlaneMat = mat4(1.0f);
	mat4 yzPlaneMat = mat4(1.0f);
	mat4 zxPlaneMat = mat4(1.0f);

	// moving
	AXIS_EL axisMoving = AXIS_EL::NONE;
	Line3D lineAlongMoving;
	vec2 lastNormalizedMousePos;

	void intersectMouseWithAxisPlane(RENDER_MASTER::ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos, const vec3& axisWorldSpace, AXIS_EL type, vec3& worldOut, float& distance);

public:
	ManipulatorTranslator(RENDER_MASTER::ICore *pCore);
	virtual ~ManipulatorTranslator();

	// IManipulator interface
public:
	bool isMouseIntersects(const vec2& normalizedMousePos) override;
	void mouseButtonDown(RENDER_MASTER::ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos) override;
	void mouseButtonUp() override;
	void update(RENDER_MASTER::ICamera *pCamera, const QRect& screen, const vec2& normalizedMousePos) override;
	void render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender) override;
};

#endif // MANIPULATORTRANSLATOR_H
