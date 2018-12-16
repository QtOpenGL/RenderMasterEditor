#ifndef MANIPULATORTRANSLATOR_H
#define MANIPULATORTRANSLATOR_H
#include <QObject>
#include "ManipulatorBase.h"


enum class AXIS
{
	NONE = -1,
	X,
	Y,
	Z
};

class ManipulatorTranslator : public ManipulatorBase
{
	Q_OBJECT

	AXIS mouseHoverAxis = AXIS::NONE;
	vec3 delta;
	int isMoving = 0;

	// not moving
	vec3 worldDelta;

	// moving
	AXIS axisMoving = AXIS::NONE;
	Line3D lineAlongMoving;
	vec2 lastNormalizedMousePos;

	void intersectMouseWithAxisPlane(RENDER_MASTER::ICamera *pCamera, const QRect &screen, const vec2 &normalizedMousePos, const vec3& axisWorldSpace, AXIS type, vec3& worldDeltaOut, vec3& worldOut, float& distance);

public:
	ManipulatorTranslator(RENDER_MASTER::ICore *pCore) : ManipulatorBase(pCore) {}
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
