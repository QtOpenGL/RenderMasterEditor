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

	AXIS moiseHoverAxis = AXIS::NONE;
	vec3 worldDelta;

public:
	ManipulatorTranslator(RENDER_MASTER::ICore *pCore) : ManipulatorBase(pCore) {}
	virtual ~ManipulatorTranslator();

	// IManipulator interface
public:
	bool isMouseIntersects(const vec2& normalizedMousePos) override;
	void update(RENDER_MASTER::ICamera *pCamera, const QRect& screen, const vec2& normalizedMousePos) override;
	void render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender) override;
};

#endif // MANIPULATORTRANSLATOR_H
