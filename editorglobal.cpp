#include "editorglobal.h"


EditorGlobal::EditorGlobal()
{
}

void EditorGlobal::ChangeSelection(const std::vector<IResource *> &selectedGameObjects)
{
	if (selectedGameObjects.size() == 0)
	{
		_someObejctSelected = 0;
		_numberSelectedObjects = 0;
		_selectionGameObject = nullptr;
		selectionChanged(std::vector<IResource*>());
		return;
	}

	_someObejctSelected = 1;

	_numberSelectedObjects = selectedGameObjects.size();

	IGameObject *go;
	selectedGameObjects[0]->GetPointer((void**)&go);
	go->GetModelMatrix(&_selectionCenterWorldTransform);
	_selectionGameObject = selectedGameObjects[0];

	selectionChanged(selectedGameObjects);
}

void EditorGlobal::RaiseFocusOnSelevtedObjects()
{
	if (_someObejctSelected > 0)
	{
		vec3 worldCeneter;
		worldCeneter.x = _selectionCenterWorldTransform.el_2D[0][3];
		worldCeneter.y = _selectionCenterWorldTransform.el_2D[1][3];
		worldCeneter.z = _selectionCenterWorldTransform.el_2D[2][3];

		AABB aabb = {-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 5.0f};

		emit OnFocusAtSelectded(worldCeneter, aabb);
	}
}

