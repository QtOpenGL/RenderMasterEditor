#ifndef EDITORGLOBAL_H
#define EDITORGLOBAL_H

#include <QObject>
#include "propertieswidget.h"
#include "scenetreewidget.h"

#include <vector>


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
	int IsSomeObjectSelected() { return _someObejctSelected; }
	int GetNumberSelectedObjects() { return _numberSelectedObjects; }
	void ChangeSelection(const std::vector<IResource*>& selectedGameObjects);

signals:

	void SceneTreeInited(SceneTreeWidget *sceneTreeWidget);

	// Toolbar
	//
	void ManipulatorPressed(MANIPULATOR manipulator);

	// Objects Selection
	//
	void selectionChanged(const std::vector<IResource*>& selectedObjects);

public slots:
};

#endif // EDITORGLOBAL_H
