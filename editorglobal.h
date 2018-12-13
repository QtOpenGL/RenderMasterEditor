#ifndef EDITORGLOBAL_H
#define EDITORGLOBAL_H

#include <QObject>

#include "propertieswidget.h"
#include "scenetreewidget.h"
#include "manipulators/ManipulatorBase.h"

#include <vector>
#include <memory>


enum class MANIPULATOR
{
	SELECT,
	TRANSLATE,
	ROTATE,
	SCALE
};


class EditorGlobal : public QWidget, RENDER_MASTER::IPositionEventSubscriber, RENDER_MASTER::IRotationEventSubscriber
{
    Q_OBJECT

	IGameObject *_selectionGameObject = nullptr;
	mat4 _selectionCenterWorldTransform;
	int _someObejctSelected = 0;
	int _numberSelectedObjects = 0;

	std::unique_ptr<ManipulatorBase> _manipulator;
	MANIPULATOR _manipulatorType = MANIPULATOR::SELECT;

	API Call(OUT vec3 *pos) override;
	API Call(OUT quat *rot) override;

	void _unsubscribe(IGameObject *go);
	void _subscribe(IGameObject *go);

public:
    explicit EditorGlobal();

    PropertiesWidget *propertiesWidget{nullptr};
	SceneTreeWidget *sceneTreeWidget{nullptr};

	// Objects Selection
	//
	mat4 GetSelectionTransform() { return _selectionCenterWorldTransform; }
	IGameObject* GetSelectionObject() { return _selectionGameObject; }
	int IsSomeObjectSelected() { return _someObejctSelected; }
	int GetNumberSelectedObjects() { return _numberSelectedObjects; }
	void ChangeSelection(const std::vector<IGameObject*>& selectedGameObjects);

	// Focus
	//
	void RaiseFocusOnSelevtedObjects();

	// Manipulators
	//
	void ToggleManipulator(MANIPULATOR manipulator);
	ManipulatorBase *CurrentManipulator() { return _manipulator.get(); }

signals:

	void SceneTreeInited(SceneTreeWidget *sceneTreeWidget);

	// Objects Selection
	//
	void selectionChanged(const std::vector<IGameObject*>& selectedObjects);

	// Focus Event
	//
	void OnFocusAtSelectded(const vec3& worldCeneter, const RENDER_MASTER::AABB& aabb);

public slots:
};

#endif // EDITORGLOBAL_H
