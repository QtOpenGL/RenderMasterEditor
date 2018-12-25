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

	// state
	int isMoving = 0;
	AXIS_EL underMouse = AXIS_EL::NONE;
	vec2 oldNormalizedMousePos;

	// moving state
	AXIS_EL movingAxis = AXIS_EL::NONE;
	Line3D movesAlongLine;
	vec3 worldDelta;

	// axis plane
	mat4 xyMirroringMat = mat4(1.0f);
	mat4 yzMirroringMat = mat4(1.0f);
	mat4 zxMirroringMat = mat4(1.0f);

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
