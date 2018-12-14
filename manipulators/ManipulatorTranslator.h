#ifndef MANIPULATORTRANSLATOR_H
#define MANIPULATORTRANSLATOR_H
#include <QObject>
#include "ManipulatorBase.h"


enum class AXIS
{
	X,
	Y,
	Z
};

class ManipulatorTranslator : public ManipulatorBase
{
	Q_OBJECT

public:
	ManipulatorTranslator(RENDER_MASTER::ICore *pCore) : ManipulatorBase(pCore) {}
	virtual ~ManipulatorTranslator();

	// IManipulator interface
public:
	bool isIntersects(const QPointF &mousePos);
	void beginDrag(const QPointF &mousePos);
	void drag(const QPointF &mousePos);
	void endDrag();
	void render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender);
};

#endif // MANIPULATORTRANSLATOR_H
