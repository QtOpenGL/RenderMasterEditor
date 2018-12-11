#ifndef IMANUPULATOR_H
#define IMANUPULATOR_H
#include <QPointF>
#include <QRect>
#include <Engine.h>

class IManipulator
{
public:
	IManipulator(){}
	virtual ~IManipulator(){}

	virtual bool isIntersects(const QPointF& mousePos) = 0;
	virtual void beginDrag(const QPointF& mousePos) = 0;
	virtual void drag(const QPointF& mousePos) = 0;
	virtual void endDrag() = 0;
	virtual void render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender) = 0;
};

#endif // IMANUPULATOR_H
