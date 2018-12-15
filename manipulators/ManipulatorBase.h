#ifndef IMANUPULATOR_H
#define IMANUPULATOR_H
#include "ManipulatorUtils.h"
#include <QPointF>
#include <QRect>
#include <QObject>
#include <Engine.h>


class ManipulatorBase : public QObject
{
	Q_OBJECT

protected:
	RENDER_MASTER::IMesh *_pAxesMesh = nullptr;
	RENDER_MASTER::IMesh *_pAxesArrowMesh = nullptr;
	RENDER_MASTER::IMesh *_pGrid = nullptr;

	void _free();
	void _drawPlane(const Plane& plane, RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *pRender, RENDER_MASTER::ICoreRender *pCoreRender);

public:
	ManipulatorBase(RENDER_MASTER::ICore *pCore);
	virtual ~ManipulatorBase(){ _free(); }

	virtual bool isIntersects(const vec2& normalizedMousePos) = 0;
	virtual void update(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender, const vec2& normalizedMousePos) = 0;
	virtual void render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender) = 0;

private slots:
	void onEngineClosed(RENDER_MASTER::ICore *pCore);
};

#endif // IMANUPULATOR_H
