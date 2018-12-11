#ifndef MANIPULATORTRANSLATOR_H
#define MANIPULATORTRANSLATOR_H
#include <QObject>
#include "IManipulator.h"


class ManipulatorTranslator : public QObject, public IManipulator
{
	Q_OBJECT

	RENDER_MASTER::IMesh *_pAxesMesh = nullptr;
	RENDER_MASTER::IMesh *_pAxesArrowMesh = nullptr;

	void _free();

public:
	ManipulatorTranslator(RENDER_MASTER::ICore *pCore);
	virtual ~ManipulatorTranslator();

	// IManipulator interface
public:
	bool isIntersects(const QPointF &mousePos);
	void beginDrag(const QPointF &mousePos);
	void drag(const QPointF &mousePos);
	void endDrag();
	void render(RENDER_MASTER::ICamera *pCamera, const QRect& screen, RENDER_MASTER::IRender *render, RENDER_MASTER::ICoreRender *coreRender);

private slots:
	void onEngineClosed(RENDER_MASTER::ICore *pCore);

};

#endif // MANIPULATORTRANSLATOR_H
