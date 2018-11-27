#ifndef EDITORGLOBAL_H
#define EDITORGLOBAL_H

#include <QObject>
#include "propertieswidget.h"
#include "scenetreewidget.h"

#include <vector>


enum class MANIPULATOR
{
	SELECT,
	TRANSLATE,
	ROTATE,
	SCALE
};


class EditorGlobal : public QObject
{
    Q_OBJECT

	IGameObject *_selectionGameObject = nullptr;
	mat4 _selectionCenterWorldTransform;
	int _someObejctSelected = 0;
	int _numberSelectedObjects = 0;

public:
    explicit EditorGlobal();

    PropertiesWidget *propertiesWidget{nullptr};
	SceneTreeWidget *sceneTreeWidget{nullptr};

	// Objects Selection
	//
	mat4 GetSelectionCeneter() { return _selectionCenterWorldTransform; }
	IGameObject* GetSelectionObject() { return _selectionGameObject; }
	int IsSomeObjectSelected() { return _someObejctSelected; }
	int GetNumberSelectedObjects() { return _numberSelectedObjects; }
	void ChangeSelection(const std::vector<IGameObject*>& selectedGameObjects);

	// Focus
	//
	void RaiseFocusOnSelevtedObjects();

signals:

	void SceneTreeInited(SceneTreeWidget *sceneTreeWidget);

	// Toolbar
	//
	void ManipulatorPressed(MANIPULATOR manipulator);

	// Objects Selection
	//
	void selectionChanged(const std::vector<IGameObject*>& selectedObjects);

	// Focus Event
	//
	void OnFocusAtSelectded(const vec3& worldCeneter, const RENDER_MASTER::AABB& aabb);

public slots:
};

#endif // EDITORGLOBAL_H
