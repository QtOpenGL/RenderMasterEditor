#ifndef EDITORGLOBAL_H
#define EDITORGLOBAL_H

#include <QObject>
#include "propertieswidget.h"
#include "scenetreewidget.h"


enum class MANIPULATOR
{
	NONE,
	TRANSLATE,
	ROTATE,
	SCALE
};


class EditorGlobal : public QObject
{
    Q_OBJECT

public:
    explicit EditorGlobal();

    PropertiesWidget *propertiesWidget{nullptr};
	SceneTreeWidget *sceneTreeWidget{nullptr};

signals:
    //void EngineInited(RENDER_MASTER::ICore *pCore);
    //void EngineBeforeClose(RENDER_MASTER::ICore *pCore);
    //void OnUpdate(float dt);
    //void OnRender();
	void SceneTreeInited(SceneTreeWidget *sceneTreeWidget);


	// TOOLBAR

	void ManipulatorPressed(MANIPULATOR manipulator);

public slots:
     //void _OnTimer();
};

#endif // EDITORGLOBAL_H
